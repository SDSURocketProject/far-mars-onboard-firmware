import sensorMessage as sm
from collections import Iterable

#------------------------------------------------------------------------------
# Stuff that can be used outside of this module
#------------------------------------------------------------------------------

# Returns a list of messages with messages of type inputID converted to type outputID
def convertMessages(messages, inputID, outputID):
    inputID = [inputID]
    outputID = [outputID]

    if (isPressure(inputID) and isPressure(outputID)):
        pass
    elif (inputID == [sm.thermocoupleRawDataID] and outputID == [sm.thermocoupleRawDataID]):
        pass
    elif (isAcceleration(inputID) and isAcceleration(outputID)):
        pass
    elif (isGyroscope(inputID) and isGyroscope(outputID)):
        pass
    elif (isMagnetometer(inputID) and isMagnetometer(outputID)):
        pass
    elif (isCpuTemp(inputID) and isCpuTemp(outputID)):
        pass
    else:
        return
    outputID = outputID[0]

    for i in range(len(messages)):
        if ( messages[i][0] != inputID[0]):
            continue
        
        if (isPressure(inputID)):
            messages[i] = pressureConvert(messages[i], outputID)
        elif (inputID == [sm.thermocoupleRawDataID]):
            messages[i] = thermocoupleConvert(messages[i])
        elif (isAcceleration(inputID)):
            messages[i] = accelerationConvert(messages[i], outputID)
        elif (isGyroscope(inputID)):
            messages[i] = gyroscopeConvert(messages[i], outputID)
        elif (isMagnetometer(inputID)):
            messages[i] = magnetometerConvert(messages[i], outputID)
        elif (isCpuTemp(inputID)):
            messages[i] = cpuTempConvert(messages[i], outputID)

    return messages

# Returns a list of messages that have been filtered based on IDs
def filterMessages(messages, filter, IDs):
    if not (isinstance(IDs, Iterable)):
        IDs = [IDs]
        
    filteredMessages = []
    if (filter == "Keep"):
        for i in range(len(messages)):
            if (messages[i][0] in IDs):
                filteredMessages.append(messages[i])
    elif (filter == "Remove"):
        for i in range(len(messages)):
            if (messages[i][0] not in IDs):
                filteredMessages.append(messages[i])
    else:
        pass
    return filteredMessages

#------------------------------------------------------------------------------
# Pressure Conversions
#------------------------------------------------------------------------------
PRESSURE_DIVISION_CONSTANT = 2**12 #(2^adc bit rate)
PRESSURE_METHANE_MAX_PRESSURE = 3000
PRESSURE_LOX_MAX_PRESSURE = 3000
PRESSURE_HELIUM_MAX_PRESSURE = 5800
PRESSURE_CHAMBER_MAX_PRESSURE = 1500
PRESSURE_METHANE_BIAS = -8
PRESSURE_LOX_BIAS = 375
PRESSURE_CHAMBER_BIAS = 23

# Returns true if the message contains pressure data
def isPressure(message):
    out = False
    if (message[0] == sm.pressureRawDataID):
        out = True
    elif (message[0] == sm.pressurePSIADataID):
        out = True
    elif (message[0] == sm.pressurePSIGDataID):
        out = True
    return out

# Returns pressure message converted to the type specified by outputID
def pressureConvert(message, outputID):
    if (outputID == sm.pressureRawDataID):
        message = pressureConvertRAW(message)
    elif (outputID == sm.pressurePSIADataID):
        message = pressureConvertPSIA(message)
    elif (outputID == sm.pressurePSIGDataID):
        message = pressureConvertPSIG(message)
    else:
        pass
    return message

# Convert the message to RAW
def pressureConvertRAW(message):
    if (message[0] ==  sm.pressurePSIGDataID):
        message = pressurePSIGToRaw(message)
    elif (message[0] ==  sm.pressurePSIADataID):
        message = pressurePSIAToRaw(message)
    else:
        pass
    return message
# Convert the message to PSIA
def pressureConvertPSIA(message):
    if (message[0] ==  sm.pressureRawDataID):
        message = pressureRawToPSIA(message)
    elif (message[0] ==  sm.pressurePSIGDataID):
        message = pressurePSIGToRaw(message)
        message = pressureRawToPSIA(message)
    else:
        pass
    return message
# Convert the message to PSIG
def pressureConvertPSIG(message):
    if (message[0] ==  sm.pressureRawDataID):
        message = pressureRawToPSIG(message)
    elif (message[0] ==  sm.pressurePSIADataID):
        message = pressurePSIAToRaw(message)
        message = pressureRawToPSIG(message)
    else:
        pass
    return message

# Convert raw pressure to PSIG
def pressureRawToPSIG(message):
    data = []

    # Methane
    temp = (message[2][0]/PRESSURE_DIVISION_CONSTANT)*5.0-0.5
	temp = (temp/4.0)*PRESSURE_METHANE_MAX_PRESSURE
    temp = temp - PRESSURE_METHANE_BIAS
    data.append(temp)
    # LOX
    temp = (message[2][1]/PRESSURE_DIVISION_CONSTANT)*5.0-0.5
    temp = (temp/4.0)*PRESSURE_LOX_MAX_PRESSURE
    temp = temp - PRESSURE_LOX_BIAS
    data.append(temp)
    # Helium
    temp = (message[2][2]/PRESSURE_DIVISION_CONSTANT)*PRESSURE_HELIUM_MAX_PRESSURE - PRESSURE_HELIUM_BIAS
    data.append(temp)
    # Chamber
    temp = (message[2][3]/PRESSURE_DIVISION_CONSTANT)*5.0-0.5
    temp = (temp/4.0)*PRESSURE_CHAMBER_MAX_PRESSURE
    temp = temp - PRESSURE_CHAMBER_BIAS
    data.append(temp)
    
    return (sm.pressurePSIGDataID, message[1], tuple(data))
# Convert raw pressure to PSIA
def pressureRawToPSIA(message):
    message = pressureRawToPSIG(message)
    return (sm.pressurePSIADataID, message[1], (message[2][0]-14.7, message[2][1]-14.7, message[2][2]-14.7, message[2][3]-14.7) )
# Convert PSIG to raw pressure
def pressurePSIGToRaw(message):
    data = []
    
    # Methane
    temp = (PRESSURE_METHANE_MAX_PRESSURE+8*message[2][0])
    temp = (0.1*PRESSURE_DIVISION_CONSTANT*temp)/PRESSURE_METHANE_MAX_PRESSURE
    data.append(temp)
    # LOX
    temp = (PRESSURE_LOX_MAX_PRESSURE+8*message[2][1])
    temp = (0.1*PRESSURE_DIVISION_CONSTANT*temp)/PRESSURE_LOX_MAX_PRESSURE
    data.append(temp)
    # Helium
    temp = (PRESSURE_DIVISION_CONSTANT*message[2][2])/PRESSURE_HELIUM_MAX_PRESSURE
    data.append(temp)
    # Chamber
    temp = (PRESSURE_DIVISION_CONSTANT*message[2][3])/PRESSURE_CHAMBER_MAX_PRESSURE
    data.append(temp)

    return (sm.pressureRawDataID, message[1], tuple(data))
# Convert PSIA to raw pressure
def pressurePSIAToRaw(message):
    message = pressurePSIGToRaw(sm.pressurePSIADataID, message[1], (message[2][0]+14.7, message[2][1]+14.7, message[2][2]+14.7, message[2][3]+14.7))
    return message

#------------------------------------------------------------------------------
# Pressure Conversions
#------------------------------------------------------------------------------

def thermocoupleConvert(message):
    data = []

    # UAF
    temp = (4.959 * (message[2][0] / 4095.0) - 1.25) / 0.005; # result in degrees C
    data.append(temp)
    return (sm.thermocoupleRawDataID, message[1], tuple(data))

#------------------------------------------------------------------------------
# Acceleration Conversions
#------------------------------------------------------------------------------
BNO055_ACCEL_DIV_MSQ = 100.0

# Returns true if the message contains acceleration data
def isAcceleration(message):
    out = False
    if (message[0] == sm.accelerationRawDataID):
        out = True
    elif (message[0] == sm.accelerationMpssDataID):
        out = True
    return out

# Returns acceleration message converted to the type specified by outputID
def accelerationConvert(message, outputID):
    if (outputID == sm.accelerationRawDataID):
        message = accelerationMpssToRaw(message)
    elif (outputID == sm.accelerationMpssDataID):
        message = accelerationRawToMpss(message)
    else:
        pass
    return message

# Convert raw acceleration to Meters per second squared
def accelerationRawToMpss(message):
    data = []
    data.append(message[2][0]/BNO055_ACCEL_DIV_MSQ)
    data.append(message[2][1]/BNO055_ACCEL_DIV_MSQ)
    data.append(message[2][2]/BNO055_ACCEL_DIV_MSQ)
    return (sm.accelerationMpssDataID, message[1], tuple(data))
# Convert Meters per second squared to raw acceleration
def accelerationMpssToRaw(message):
    data = []
    data.append(message[2][0]*BNO055_ACCEL_DIV_MSQ)
    data.append(message[2][1]*BNO055_ACCEL_DIV_MSQ)
    data.append(message[2][2]*BNO055_ACCEL_DIV_MSQ)
    return (sm.accelerationRawDataID, message[1], tuple(data))

#------------------------------------------------------------------------------
# Gyroscope Conversions
#------------------------------------------------------------------------------
#BNO055_GYRO_DIV_DPS = 16.0
BNO055_GYRO_DIV_RPS = 900.0

# Returns true if the message contains gyroscope data
def isGyroscope(message):
    out = False
    if (message[0] == sm.gyroscopeRawDataID):
        out = True
    elif (message[0] == sm.gyroscopeRpsDataID):
        out = True
    return out

# Returns gyroscope message converted to the type specified by outputID
def gyroscopeConvert(message, outputID):
    if (outputID == sm.gyroscopeRawDataID):
        message = gyroscopeRadiansToRaw(message)
    elif (outputID == sm.gyroscopeRpsDataID):
        message = gyroscopeRawToRadians(message)
    else:
        pass
    return message

# Convert raw gyroscope to Meters per second squared
def gyroscopeRawToRadians(message):
    data = []
    data.append(message[2][0]/BNO055_GYRO_DIV_RPS)
    data.append(message[2][1]/BNO055_GYRO_DIV_RPS)
    data.append(message[2][2]/BNO055_GYRO_DIV_RPS)
    return (sm.gyroscopeRpsDataID, message[1], tuple(data))
# Convert Meters per second squared to raw gyroscope
def gyroscopeRadiansToRaw(message):
    data = []
    data.append(message[2][0]*BNO055_GYRO_DIV_RPS)
    data.append(message[2][1]*BNO055_GYRO_DIV_RPS)
    data.append(message[2][2]*BNO055_GYRO_DIV_RPS)
    return (sm.gyroscopeRawDataID, message[1], tuple(data))

#------------------------------------------------------------------------------
# Magnetometer Conversions, Not currently supported
#------------------------------------------------------------------------------
BNO055_MAG_DIV_UT = 16.0

# Returns true if the message contains gyroscope data
def isMagnetometer(message):
    out = False

    return out

# Returns magnetometer message converted to the type specified by outputID
def magnetometerConvert(message, outputID):
    if (outputID == sm.NUM_SENSOR_MESSAGES):
        message = gyroscopeRawToRadians(message)
    elif (outputID == sm.NUM_SENSOR_MESSAGES):
        message = gyroscopeRadiansToRaw(message)
    else:
        pass
    return message

# Convert raw magnetometer to uTesla
def magnetometerRawToUT(message):
    data = []
    data.append(message[2][0]/BNO055_GYRO_DIV_RPS)
    data.append(message[2][1]/BNO055_GYRO_DIV_RPS)
    data.append(message[2][2]/BNO055_GYRO_DIV_RPS)
    return (sm.NUM_SENSOR_MESSAGES, message[1], tuple(data))
# Convert uTesla to raw magnetometer
def magnetometerUTToRaw(message):
    data = []
    data.append(message[2][0]*BNO055_GYRO_DIV_RPS)
    data.append(message[2][1]*BNO055_GYRO_DIV_RPS)
    data.append(message[2][2]*BNO055_GYRO_DIV_RPS)
    return (sm.NUM_SENSOR_MESSAGES, message[1], tuple(data))

#------------------------------------------------------------------------------
# CPU Temperature Conversions
#------------------------------------------------------------------------------
CPU_TEMP_DIV_CELCIUS = 100.0

# Returns true if the message contains CPU temperature data
def isCpuTemp(message):
    out = False
    if (message[0] == sm.cpuTemperatureRawDataID):
        out = True
    elif (message[0] == sm.cpuTemperatureCelciusDataID):
        out = True
    return out

# Returns CPU temperature message converted to the type specified by outputID
def cpuTempConvert(message, outputID):
    if (outputID == sm.cpuTemperatureRawDataID):
        message = cpuTempCelciusToRaw(message)
    elif (outputID == sm.cpuTemperatureCelciusDataID):
        message = cpuTempRawToCelcius(message)
    else:
        pass
    return message

# Convert raw CPU temperature to Celcius
def cpuTempRawToCelcius(message):
    return (sm.cpuTemperatureCelciusDataID, message[1], message[2][0]/CPU_TEMP_DIV_CELCIUS)
# Convert Celcius to raw CPU temperature
def cpuTempCelciusToRaw(message):
    return (sm.cpuTemperatureRawDataID, message[1], message[2][0]*CPU_TEMP_DIV_CELCIUS)

if (__name__ == "__main__"):
    print("Do not use this file directly, use \"logRead.py\" instead.")
    input("Press Enter to continue...")