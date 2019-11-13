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
pressureRawADC1DataID = 20
thermocoupleRawDataID = 21
hallEffectDataID = 22
batteryRawDataID = 23
batteryFloatDataID = 24
strDataID = 25
NUM_SENSOR_MESSAGES = 26
CSV_ALL_SENSOR_MESSAGES = 100

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
13, #pressureRawDataID
21, #pressurePSIADataID
21, #pressurePSIGDataID
7,  #pressureRawADC1DataID
7,  #thermocoupleRawDataID
7,  #hallEffectDataID
7,  #batteryRawDataID
9,  #batteryFloatDataID
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
'<hhhh', #pressureRawDataID 17
'<IIII', #pressurePSIADataID 18
'<IIII', #pressurePSIGDataID 19
'<h',    #pressureRawADC1DataID
'<h',    #thermocoupleRawDataID
'<BB',   #hallEffectDataID
'<h',    #batteryRawDataID 20
'<f',    #batteryFloatDataID 21
's'      #strDataID 22
]

messageLogFormats = [
"Raw Acceleration - X=%i, Y=%i, Z=%i\n",                       #accelerationRawDataID
"Raw Gyroscope - X=%i, Y=%i, Z=%i\n",                          #gyroscopeRawDataID
"Raw Euler - X=%i, Y=%i, Z=%i\n",	                           #eulerRawDataID
"Quaternion - %i, %i, %i, %i\n",                               #quaternionRawDataID
"Raw Linear Acceleration - X=%i, Y=%i, Z=%i\n",	               #linearAccelerationRawDataID
"Raw Gravity - X=%i, Y=%i, Z=%i\n",	                           #gravityRawDataID
"Meters per second squared - X=%f, Y=%f, Z=%f\n",              #accelerationMpssDataID
"Radians per second - X=%f, Y=%f, Z=%f\n",                     #gyroscopeRpsDataID
"Degrees per second - X=%f, Y=%f, Z=%f\n",                     #gyroscopeDpsDataID
"Radians - X=%f, Y=%f, Z=%f\n",                                #eulerRadianDataID
"Degrees - X=%f, Y=%f, Z=%f\n",                                #eulerDegreeDataID
"Meters per second squared - X=%f, Y=%f, Z=%f\n",              #linearAccelerationMpssDataID
"Meters per second squared - X=%f, Y=%f, Z=%f\n",              #gravityMpssDataID
"Raw CPU Temperature - %i\n",                                  #cpuTemperatureRawDataID
"CPU Celcius - %f\n",                                          #cpuTemperatureCelciusDataID
"CPU Fahrenheit - %f\n",                                       #cpuTemperatureFahrenheitDataID
"CPU Kelvin - %f\n",                                           #cpuTemperatureKelvinDataID
"Raw Pressures - Methane=%i, LOX=%i, Helium=%i, Chamber=%i\n", #pressureRawDataID
"PSI absolute - Methane=%i, LOX=%i, Helium=%i, Chamber=%i\n",  #pressurePSIADataID
"PSI guage - Methane=%i, LOX=%i, Helium=%i, Chamber=%i\n",     #pressurePSIGDataID
"Raw ADC 1 pressure - HE Reg=%i\n",                            #pressureRawADC1DataID
"Raw thermocouple - UAF=%i\n",                                 #thermocoupleRawDataID
"Hall effect sensors = Methane=%i, LOX=%i\n",                  #hallEffectDataID
'Battery raw voltage - %i\n',                                  #batteryRawDataID
'Battery voltage - %f\n',                                      #batteryFloatDataID
"%s"                                                           #strDataID
]

messageCSVHeaders = [
"Timestamp, X Raw, Y Raw, Z Raw,\n",                          #accelerationRawDataID
"Timestamp, X Raw, Y Raw, Z Raw,\n",                          #gyroscopeRawDataID
"Timestamp, X Raw, Y Raw, Z Raw,\n",	                      #eulerRawDataID
"Timestamp, W, X, Y, Z,\n",                                   #quaternionRawDataID
"Timestamp, X Raw, Y Raw, Z Raw,\n",	                      #linearAccelerationRawDataID
"Timestamp, X Raw, Y Raw, Z Raw,\n",	                      #gravityRawDataID
"Timestamp, X, Y, Z,\n",                                      #accelerationMpssDataID
"Timestamp, X, Y, Z,\n",                                      #gyroscopeRpsDataID
"Timestamp, X, Y, Z,\n",                                      #gyroscopeDpsDataID
"Timestamp, X, Y, Z,\n",                                      #eulerRadianDataID
"Timestamp, X, Y, Z,\n",                                      #eulerDegreeDataID
"Timestamp, X, Y, Z,\n",                                      #linearAccelerationMpssDataID
"Timestamp, X, Y, Z,\n",                                      #gravityMpssDataID
"Timestamp, Temp Raw,\n",                                     #cpuTemperatureRawDataID
"Timestamp, Celcius,\n",                                      #cpuTemperatureCelciusDataID
"Timestamp, Fahrenheit,\n",                                   #cpuTemperatureFahrenheitDataID
"Timestamp, Kelvin,\n",                                       #cpuTemperatureKelvinDataID
"Timestamp, Methane Raw, LOX Raw, Helium Raw, Chamber Raw\n", #pressureRawDataID
"Timestamp, Methane, LOX, Helium, Chamber,\n",                #pressurePSIADataID
"Timestamp, Methane, LOX, Helium, Chamber,\n",                #pressurePSIGDataID
"Timestamp, He Reg,\n",                                       #pressureRawADC1DataID
"Timestamp, UAF,\n",                                          #thermocoupleRawDataID
"Timestamp, Methane, LOX,\n",                                 #hallEffectDataID
'Timestamp, Voltage Raw\n',                                   #batteryRawDataID
'Timestamp, Voltage\n',                                       #batteryFloatDataID
"Timestamp, Strings,\n"                                       #strDataID
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
"%i, %i, %i, %i\n",  #pressureRawDataID
"%i, %i, %i, %i\n",  #pressurePSIADataID
"%i, %i, %i, %i\n",  #pressurePSIGDataID
"%i, %i,\n",         #pressureRawADC1DataID
"%i,\n",             #thermocoupleRawDataID
"%i, %i,\n",         #hallEffectDataID
'%i\n',              #batteryRawDataID
'%f\n',              #batteryFloatDataID
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
        try:
            (msgID, timestamp, msg, dataIdx) = unpackMessage(dataIdx, data)
            messages.append((msgID, timestamp, msg))
        except IndexError:
            print(f"Parsing \"{fileName}\" failed at byte {hex(dataIdx)}")
            return []
            
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

CSV_ALL_TIMESTAMP = 0
CSV_ALL_PT_METHANE = 1
CSV_ALL_PT_LOX = 2
CSV_ALL_PT_HELIUM = 3
CSV_ALL_TC = 4
CSV_ALL_HE_METHANE = 5
CSV_ALL_HE_LOX = 6
CSV_ALL_MAX = 7
def writeCSV(messages, msgID, fileName):
    if (msgID == CSV_ALL_SENSOR_MESSAGES): # It aint pretty, but it works (mostly)
        outData = []
        data = [0]*CSV_ALL_MAX
        for message in messages:
            if (message[0] == pressurePSIGDataID):
                data[CSV_ALL_TIMESTAMP] = message[1]
                data[CSV_ALL_PT_METHANE] = int(message[2][0])
                data[CSV_ALL_PT_LOX] = int(message[2][1])
                data[CSV_ALL_PT_HELIUM] = int(message[2][2])
            elif (message[0] == thermocoupleRawDataID):
                data[CSV_ALL_TC] = int(message[2][0])
            elif (message[0] == hallEffectDataID):
                data[CSV_ALL_HE_METHANE] = int(message[2][0])
                data[CSV_ALL_HE_LOX] = int(message[2][1])
                outData.append(data[:])
        with open(fileName, 'w') as csvFile:
            print(f"Logging data to \"{fileName}\".")
            csvFile.write("Timestamp, PT_Methane, PT_LOX, PT_Helium, TC_UAF, HE_Methane, HE_LOX\n")
            for dataPoint in outData:
                csvFile.write(f"{dataPoint[CSV_ALL_TIMESTAMP]}, {dataPoint[CSV_ALL_PT_METHANE]}, {dataPoint[CSV_ALL_PT_LOX]}, {dataPoint[CSV_ALL_PT_HELIUM]}, {dataPoint[CSV_ALL_TC]}, {dataPoint[CSV_ALL_HE_METHANE]}, {dataPoint[CSV_ALL_HE_LOX]}\n")
        return
    elif msgID >= NUM_SENSOR_MESSAGES:
        return
    
    with open(fileName, 'w') as csvFile:
        print(f"Logging data to \"{fileName}\".")
        csvFile.write(messageCSVHeaders[msgID])
        for message in messages:
            if (message[0] != msgID):
                continue
            csvFile.write(formatMessage(message, style='csv'))
    return

if (__name__ == "__main__"):
    print("Do not use this file directly, use \"logRead.py\" instead.")
    input("Press Enter to continue...")