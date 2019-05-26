# FAR MARS Onboard Firmware
The FAR MARS Onboard Firmware is the software used for data acquisition aboard the SDSU Lady Elizabeth rocket.

The primary tasks for the onboard computer are:
1. Read data from the following sensors:
    - Methane, Liquid Oxygen, Helium, Chamber, and Helium Regulator pressure transducers
    - Upper air frame type K thermocouple
    - Methane and Liquid Oxygen vent valve hall effect sensors
    - BNO055 Internal Measurement Unit
2. Send the above sensor data to the off board data acquisition computer prior to launch
3. Log the above sensor data to an SD card inserted into the onboard computer.

# Testing data
Recordings of the data collected during cryo, static, and launch are located in the folder testing. In each folder is are .mat files containing the data from the test and a .m Matlab script which uses that data to create graphs.

# Documentation
Open the file "./documentation/html/index.html" in any web browser to view the firmware documentation.
