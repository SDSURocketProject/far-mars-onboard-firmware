/*
 * far_mars_onboard_firmware.h
 *
 * Created: 12/26/2018 1:52:11 PM
 *  Author: David Knight
 */

#ifndef FAR_MARS_ONBOARD_FIRMWARE_H_
#define FAR_MARS_ONBOARD_FIRMWARE_H_

//! Returns the length of an array
#define LENGTH(x) (sizeof(x) / sizeof((x)[0]))

//! All of the return values for functions in the Far Mars Onboard Firmware (FMOF)
enum FMOF_returns {
    FMOF_SUCCESS,                   //! Generic success
    FMOF_INVALID_ARG,               //! Generic invalid argument, see function return values for more detail
    FMOF_NOT_INIT,                  //! Something needed by the function is not yet initialized, see function return values for more detail
    FMOF_LOGGER_LOW_LOGGING_LEVEL,  //! A message was send to be logged but it's logging level was less than LOGGING_LEVEL
    FMOF_LOGGER_QUEUE_FULL,         //! The logger queue is completely full of messages
    FMOF_LOGGER_QUEUE_NOT_INIT,     //! The logger queue is not yet initialized
    FMOF_LOGGER_INVALID_MSGID,      //! An invalid message ID was passed to the message logger
    FMOF_PRESSURE_SENSOR_IN_USE,    //! The pressure sensor is being used by another task
    FMOF_PRESSURE_QUEUE_FULL,       //! The pressure sensor is unavailable
    FMOF_PRESSURE_START_CONVERSION, //! Read conversion was called before start conversion
    FMOF_FAILURE                    //! Generic failure
};

/**
 * @mainpage Lady Elizabeth OBC firmware Documentation.
 *
 * This manual describes the operation of the firmware running on the Lady
 * Elizabeth rocket.
 *
 * @section mainpageIntroduction Introduction
 *
 * The primary tasks for the onboard computer are:
 * -# Read data from the following sensors:
 *     - Methane, Liquid Oxygen, Helium, Chamber, and Helium Regulator pressure transducers
 *     - Upper air frame type K thermocouple
 *     - Methane and Liquid Oxygen vent valve hall effect sensors
 *     - BNO055 Internal Measurement Unit
 * -# Send the above sensor data to the off board data acquisition computer prior to launch
 * -# Log the above sensor data to an SD card inserted into the onboard computer.
 *
 * This firmware is created in Atmel Studio 7.0.
 * 
 * This documentation assumes basic familiarity with the FreeRTOS kernel. For
 * learning basic concepts of the FreeRTOS kernel refer to the pdf book,
 * "Mastering the FreeRTOS Real Time Kernel", at the
 * <a href="https://www.freertos.org/Documentation/RTOS_book.html">FreeRTOS website</a>.
 * For the sake of understanding the basics of this firmware you should read
 * the sections Preface, Task Management, and Queue Management.
 *
 * The current version of the onboard computer schematic can be found at <a href="https://github.com/SDSURocketProject/far-mars-avionics-pcb/blob/master/documentation/schematic.pdf">far-mars-avionics-pcb</a>.
 * 
 * @section mainpageAcronyms Acronyms
 * | Acronym | Meaning                                                          |
 * | :------ | :--------------------------------------------------------------- |
 * | ADC     | Analog to Digital Converter                                      |
 * | ASF     | Atmel Software Framework                                         |
 * | CH4     | Methane                                                          |
 * | DAQ     | Data Acquisition                                                 |
 * | FMOF    | Far Mars Onboard Firmware                                        |
 * | HE      | Helium                                                           |
 * | I2C     | Inter-Integrated Circuit (pronounced I-squared-C)                |
 * | IMU     | Internal Measurement Unit                                        |
 * | LED     | Light Emitting Diode                                             |
 * | LiPo    | Lithium Polymer Battery                                          |
 * | LOX     | Liquid Oxygen                                                    |
 * | OBC     | Onboard Computer                                                 |
 * | PSIG    | Pressure per Square Inch Guage                                   |
 * | PSIA    | Pressure per Square Inch Absolute                                |
 * | PT      | Pressure Transducer                                              |
 * | RTC     | Real Time Clock                                                  |
 * | RTOS    | Real Time Operating System                                       |
 * | RS485   | <a href="https://en.wikipedia.org/wiki/RS-485">See Wikipedia</a> |
 * | SDADC   | Sigma Delta Analog to Digital Converter                          |
 * | SERCOM  | Serial Communication Interface                                   |
 * | SPI     | Serial Peripheral Interface                                      |
 * | TC      | Thermocouple                                                     |
 * | UAF     | Upper Air Frame                                                  |
 * | UART    | Universal Asynchronous Receiver-Transmitter                      |
 *
 * @section mainpageToO Theory of Operation
 * The OBC firmware is broken up into five distinct sections:
 * - @ref A_mainGroup "Initialization": Initialization of the FreeRTOS kernel, tasks, and peripherals
 * - @ref B_navigationGroup "Navigation": Read data from sensors aboard the rocket
 * - @ref C_daqGroup "Data Acquisition": Transmit data to data acquisition computer
 * - @ref D_loggerGroup "Logging": Log data to SD card
 * - @ref E_statusGroup "Status Indication": Control LEDs to indicate OBC status
 *
 * The sections Navigation, Data Acquisition, Logging, and Status Indication
 * can each be created as a FreeRTOS task, each with its own priority. Using
 * these tasks and priorities the FreeRTOS kernel can switch between each task
 * enabling us to write each task independent of each other. Because each task
 * can be stopped and started by the FreeRTOS kernel at any time, communication
 * must be done via a "thread safe" data structure. This firmware primarily
 * uses FIFO queues for communication between tasks however mutexes are used on
 * occasion.
 *
 * Each task is listed below in order from highest priority to lowest:
 * -# Navigation
 * -# Data Acquisition
 * -# Logging
 * -# Status Indication
 * 
 * The highest priority task in this firmware is the Navigation task because
 * every other task depends on the Navigation task running on time, data can't
 * be sent or logged if it hasn't yet been read from the sensors. The second
 * highest priority task is the Data Acquisition task because this is how we
 * know the current status of the rocket prior to launch. Failure to run this
 * task could result in failure of the rocket if pressure tanks become
 * overpressurized and we aren't aware of that in the bunker. The third
 * highest priority task is the Logging task because if this task is unable to
 * run on time it does not result in the failure of the rocket. Lastly the
 * lowest priority task is the Status Indication because it should only update
 * when everything else is working properly, failure of this task to update is
 * is a clear indicator that the OBC is not working properly. Sometimes this
 * task is set to the same priority as the Logging task because the status
 * will not be updated if there is no SD card plugged in, setting the two tasks
 * to the same priority fixes this if that is preferred.
 *
 * @section mainpageDocGen Documentation generation
 * This section goes over the use of Doxygen for generating this documentation.
 * Use of Doxygen is not necessary for learning/using the firmware in this 
 * documentation, however it is necessary for updating this documentation if
 * you contribute to this firmware.
 *
 * The latest version of Doxygen can be download from the
 * <a href="http://doxygen.nl/download.html">Doxygen website</a>.
 * Follow these steps to compile this documentation:
 * -# Open this projects Doxygen file by selecting "File > Open" and selecting
 * the "Doxyfile" at the root directory of this project
 * -# Select the "Run" tab and click the button labeled "Run Doxygen"
 * -# The output window will display the text "Doxygen has finished" when the
 * documentation is finished compiling
 * -# You can view the documentation by clicking the "Show HTML output" button
 * at the bottom of the wizard or by opening the file
 * ".\documentation\html\index.html" in any web browser.
 *
 * For information on how to write documentation with Doxygen reference the
 * <a href="http://doxygen.nl/manual/index.html">Doxygen manual</a>.
 *
 * @section mainpageBNO055License BNO055 License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 *   Neither the name of the copyright holder nor the names of the
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER
 * OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 *
 * The information provided is believed to be accurate and reliable.
 * The copyright holder assumes no responsibility
 * for the consequences of use
 * of such information nor for any infringement of patents or
 * other rights of third parties which may result from its use.
 * No license is granted by implication or otherwise under any patent or
 * patent rights of the copyright holder.
 */

#endif /* FAR_MARS_ONBOARD_FIRMWARE_H_ */