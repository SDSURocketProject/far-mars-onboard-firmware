import struct
import sensorConversions
from collections import Iterable
from enum import Enum

accelerationRawDataID = 0
gyroscopeRawDataID = 1
eulerRawDataID = 2
quaternionRawDataID = 3
linearAccelerationRawDataID = 4
gravityRawDataID = 5
accelerationMpssDataID = 6
gyroscopeRpsDataID = 7
gyroscopeDpsDataID = 8
eulerRadianDataID = 9
eulerDegreeDataID = 10
linearAccelerationMpssDataID = 11
gravityMpssDataID = 12
cpuTemperatureRawDataID = 13
cpuTemperatureCelciusDataID = 14
cpuTemperatureFahrenheitDataID = 15
cpuTemperatureKelvinDataID = 16
pressureRawDataID = 17
pressurePSIADataID = 18
pressurePSIGDataID = 19
strDataID = 20
NUM_SENSOR_MESSAGES = 21

messageSizes = [
11, #accelerationRawDataID
11, #gyroscopeRawDataID
11,	#eulerRawDataID
13, #quaternionRawDataID
11,	#linearAccelerationRawDataID
11,	#gravityRawDataID
17, #accelerationMpssDataID
17, #gyroscopeRpsDataID
17, #gyroscopeDpsDataID
17, #eulerRadianDataID
17, #eulerDegreeDataID
17, #linearAccelerationMpssDataID
17, #gravityMpssDataID
7,  #cpuTemperatureRawDataID
9,  #cpuTemperatureCelciusDataID
9,  #cpuTemperatureFahrenheitDataID
9,  #cpuTemperatureKelvinDataID
11, #pressureRawDataID
17, #pressurePSIADataID
17, #pressurePSIGDataID
0   #strDataID
]

sensorMessageHeaderFmt = '<BI'

messagePackedFormats = [
'<hhh',  #accelerationRawDataID 0
'<hhh',  #gyroscopeRawDataID 1
'<hhh',	 #eulerRawDataID 2
'<hhhh', #quaternionRawDataID 3
'<hhh',	 #linearAccelerationRawDataID 4
'<hhh',	 #gravityRawDataID 5
'<fff',  #accelerationMpssDataID 6
'<fff',  #gyroscopeRpsDataID 7
'<fff',  #gyroscopeDpsDataID 8
'<fff',  #eulerRadianDataID 9
'<fff',  #eulerDegreeDataID 10
'<fff',  #linearAccelerationMpssDataID 11
'<fff',  #gravityMpssDataID 12
'<h',    #cpuTemperatureRawDataID 13
'<f',    #cpuTemperatureCelciusDataID 14
'<f',    #cpuTemperatureFahrenheitDataID 15
'<f',    #cpuTemperatureKelvinDataID 16
'<hhh',  #pressureRawDataID 17
'<fff',  #pressurePSIADataID 18
'<fff',  #pressurePSIGDataID 19
's'      #strDataID 20
]

messageLogFormats = [
"Raw Acceleration - X=%i, Y=%i, Z=%i\n",           #accelerationRawDataID
"Raw Gyroscope - X=%i, Y=%i, Z=%i\n",              #gyroscopeRawDataID
"Raw Euler - X=%i, Y=%i, Z=%i\n",	               #eulerRawDataID
"Quaternion - %i, %i, %i, %i\n",                   #quaternionRawDataID
"Raw Linear Acceleration - X=%i, Y=%i, Z=%i\n",	   #linearAccelerationRawDataID
"Raw Gravity - X=%i, Y=%i, Z=%i\n",	               #gravityRawDataID
"Meters per second squared - X=%f, Y=%f, Z=%f\n",  #accelerationMpssDataID
"Radians per second - X=%f, Y=%f, Z=%f\n",         #gyroscopeRpsDataID
"Degrees per second - X=%f, Y=%f, Z=%f\n",         #gyroscopeDpsDataID
"Radians - X=%f, Y=%f, Z=%f\n",                    #eulerRadianDataID
"Degrees - X=%f, Y=%f, Z=%f\n",                    #eulerDegreeDataID
"Meters per second squared - X=%f, Y=%f, Z=%f\n",  #linearAccelerationMpssDataID
"Meters per second squared - X=%f, Y=%f, Z=%f\n",  #gravityMpssDataID
"Raw CPU Temperature - %i\n",                      #cpuTemperatureRawDataID
"CPU Celcius - %f\n",                              #cpuTemperatureCelciusDataID
"CPU Fahrenheit - %f\n",                           #cpuTemperatureFahrenheitDataID
"CPU Kelvin - %f\n",                               #cpuTemperatureKelvinDataID
"Raw Pressures - Methane=%i, LOX=%i, Helium=%i\n", #pressureRawDataID
"PSI absolute - Methane=%f, LOX=%f, Helium=%f\n",  #pressurePSIADataID
"PSI guage - Methane=%f, LOX=%f, Helium=%f\n",     #pressurePSIGDataID
"%s"                                               #strDataID
]

messageCSVHeaders = [
"Timestamp, X Raw, Y Raw, Z Raw,\n",              #accelerationRawDataID
"Timestamp, X Raw, Y Raw, Z Raw,\n",              #gyroscopeRawDataID
"Timestamp, X Raw, Y Raw, Z Raw,\n",	          #eulerRawDataID
"Timestamp, W, X, Y, Z,\n",                       #quaternionRawDataID
"Timestamp, X Raw, Y Raw, Z Raw,\n",	          #linearAccelerationRawDataID
"Timestamp, X Raw, Y Raw, Z Raw,\n",	          #gravityRawDataID
"Timestamp, X, Y, Z,\n",                          #accelerationMpssDataID
"Timestamp, X, Y, Z,\n",                          #gyroscopeRpsDataID
"Timestamp, X, Y, Z,\n",                          #gyroscopeDpsDataID
"Timestamp, X, Y, Z,\n",                          #eulerRadianDataID
"Timestamp, X, Y, Z,\n",                          #eulerDegreeDataID
"Timestamp, X, Y, Z,\n",                          #linearAccelerationMpssDataID
"Timestamp, X, Y, Z,\n",                          #gravityMpssDataID
"Timestamp, Temp Raw,\n",                         #cpuTemperatureRawDataID
"Timestamp, Celcius,\n",                          #cpuTemperatureCelciusDataID
"Timestamp, Fahrenheit,\n",                       #cpuTemperatureFahrenheitDataID
"Timestamp, Kelvin,\n",                           #cpuTemperatureKelvinDataID
"Timestamp, Methane Raw, LOX Raw, Helium Raw,\n", #pressureRawDataID
"Timestamp, Methane, LOX, Helium,\n",             #pressurePSIADataID
"Timestamp, Methane, LOX, Helium,\n",             #pressurePSIGDataID
"Timestamp, Strings,\n"                           #strDataID
]

messageCSVFormats = [
"%i, %i, %i,\n",     #accelerationRawDataID
"%i, %i, %i,\n",     #gyroscopeRawDataID
"%i, %i, %i,\n",	 #eulerRawDataID
"%i, %i, %i, %i,\n", #quaternionRawDataID
"%i, %i, %i,\n",	 #linearAccelerationRawDataID
"%i, %i, %i,\n",	 #gravityRawDataID
"%f, %f, %f,\n",     #accelerationMpssDataID
"%f, %f, %f,\n",     #gyroscopeRpsDataID
"%f, %f, %f,\n",     #gyroscopeDpsDataID
"%f, %f, %f,\n",     #eulerRadianDataID
"%f, %f, %f,\n",     #eulerDegreeDataID
"%f, %f, %f,\n",     #linearAccelerationMpssDataID
"%f, %f, %f,\n",     #gravityMpssDataID
"%i,\n",             #cpuTemperatureRawDataID
"%f,\n",             #cpuTemperatureCelciusDataID
"%f,\n",             #cpuTemperatureFahrenheitDataID
"%f,\n",             #cpuTemperatureKelvinDataID
"%i, %i, %i,\n",     #pressureRawDataID
"%f, %f, %f,\n",     #pressurePSIADataID
"%f, %f, %f,\n",     #pressurePSIGDataID
"%s,\n"              #strDataID
]

# For debugging
def packMessage(msgID, timestamp, data):
    packed = struct.pack(sensorMessageHeaderFmt, msgID, timestamp)
    if (isinstance(data, Iterable)):
        packed += struct.pack(messagePackedFormats[msgID], *data)
    else:
        packed += struct.pack(messagePackedFormats[msgID], data)
    return packed

def unpackMessages(fileName):
    with open(fileName, 'rb') as inFile:
        data = inFile.read()

    messages = []
    dataIdx = 0
    while (dataIdx < len(data)):
        (msgID, timestamp, msg, dataIdx) = unpackMessage(dataIdx, data)
        messages.append((msgID, timestamp, msg))

    return messages

def unpackMessage(dataIdx, data):
    ( msgID, timestamp ) = struct.unpack_from(sensorMessageHeaderFmt, data, dataIdx)
    
    if (msgID == strDataID):
        dataIdx = dataIdx + 5
        (msg, dataIdx) = unpackString(dataIdx, data)
    else:
        ( msg ) = struct.unpack_from(messagePackedFormats[msgID], data, dataIdx+5)
        dataIdx = dataIdx + messageSizes[msgID]

    return (msgID, timestamp, msg, dataIdx)

def unpackString(dataIdx, data):
    char = 'a'
    msg = ""
    while (char[0] != b'\x00'):
        ( char ) = struct.unpack_from('c', data, dataIdx)
        dataIdx = dataIdx + 1
        msg += char[0].decode("ascii")
    
    return (msg[0:-1], dataIdx)

def formatMessage(message, style='log'):
    if (style == 'log'):
        # This is the most amazing thing I've ever seen
        return (f"{message[1]}ms: ")+(messageLogFormats[message[0]] % message[2])
    elif (style == 'csv'):
        return (f"{message[1]}, ")+(messageCSVFormats[message[0]] % message[2])
    return

def writeLog(messages, fileName):
    with open(fileName, 'w') as logFile:
        for message in messages:
            logFile.write(formatMessage(message))
    return

def writeCSV(messages, msgID, fileName):
    if msgID >= NUM_SENSOR_MESSAGES:
        return

    with open(fileName, 'w') as csvFile:
        csvFile.write(messageCSVHeaders[msgID])
        for message in messages:
            if (message[0] != msgID):
                continue
            csvFile.write(formatMessage(message, style='csv'))
    return

