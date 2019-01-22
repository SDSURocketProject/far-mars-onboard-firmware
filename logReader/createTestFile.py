import sensorMessage as sm
import random

ACCELERATION_RATE = 10
GYROSCOPE_RATE = 20
MAGNETOMETER_RATE = 50
PRESSURE_RATE = 10
CPU_TEMP_RATE = 1000

#maxTime = 30*60*1000+1
maxTime = 1001

random.seed()

with open("data.log", 'wb') as testFile:
    for timestamp in range(0, maxTime):
        if not (timestamp % ACCELERATION_RATE):
            data = (random.randint(0, 32767), random.randint(0, 32767), random.randint(0, 32767))
            testFile.write(sm.packMessage(sm.accelerationRawDataID, timestamp, data))
        if not (timestamp % GYROSCOPE_RATE):
            data = (random.randint(0, 32767), random.randint(0, 32767), random.randint(0, 32767))
            testFile.write(sm.packMessage(sm.gyroscopeRawDataID, timestamp, data))
        
        if not (timestamp % PRESSURE_RATE):
            data = (random.randint(0, 32767), random.randint(0, 32767), random.randint(0, 32767))
            testFile.write(sm.packMessage(sm.pressureRawDataID, timestamp, data))
        if not (timestamp % CPU_TEMP_RATE):
            data = (random.randint(0, 32767))
            testFile.write(sm.packMessage(sm.cpuTemperatureRawDataID, timestamp, data))

#    j = 1
#    for i in range(0, maxTime, 10):
#        testFile.write(sm.packMessage(sm.accelerationRawDataID, i, (j, j+1, j+2)))
#        j = j + 3
#        j = j % 32760
#    j = 1
#    for i in range(0, maxTime, 20):
#        testFile.write(sm.packMessage(sm.gyroscopeRawDataID, i, (j, j+1, j+2)))
#        j = j + 3
#        j = j % 32760
#    
#    j = 1
#    for i in range(0, maxTime, 20):
#        testFile.write(sm.packMessage(sm.pressureRawDataID, i, (j, j+1, j+2)))
#        j = j + 3
#        j = j % 32760
#
#    j = 1
#    for i in range(0, maxTime, 100):
#        testFile.write(sm.packMessage(sm.cpuTemperatureRawDataID, i, (j)))
#        j = j + 1
#        j = j % 32760
