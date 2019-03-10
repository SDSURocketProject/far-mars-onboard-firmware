import sensorMessage as sm
from sensorConversions import convertMessages
from sensorConversions import filterMessages

allMessages = sm.unpackMessages("data.log")

allMessages.sort(key=lambda tup: tup[1]) # Sorting by timestamp

# Check for lost data
last = allMessages[1][1]
for message in allMessages:
    if (message[1] - last > 10):
        print(f"{last}, {message[1]}, {message[1] - last}")
    
    last = message[1]
    

#allMessages = convertMessages(allMessages, sm.accelerationRawDataID, sm.accelerationMpssDataID)
#allMessages = convertMessages(allMessages, sm.gyroscopeRawDataID, sm.gyroscopeRpsDataID)
#allMessages = convertMessages(allMessages, sm.pressureRawDataID, sm.pressurePSIGDataID)
#allMessages = convertMessages(allMessages, sm.cpuTemperatureRawDataID, sm.cpuTemperatureCelciusDataID)

#sm.writeLog(allMessages, "formattedData.log")
#sm.writeCSV(allMessages, sm.accelerationMpssDataID, "Acceleration.csv")
#sm.writeCSV(allMessages, sm.gyroscopeRpsDataID, "Gyroscope.csv")
#sm.writeCSV(allMessages, sm.pressurePSIGDataID, "Pressure.csv")
#sm.writeCSV(allMessages, sm.cpuTemperatureCelciusDataID, "CPU_Temperature.csv")
