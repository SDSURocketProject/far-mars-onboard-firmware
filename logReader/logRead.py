import sensorMessage as sm
from sensorConversions import convertMessages
from sensorConversions import filterMessages
import sys

if (__name__ == "__main__"):
    allMessages = []
    if (len(sys.argv) != 2):
        print("Incorrect number of arguments:")
        print(sys.argv)
        input("Press Enter to continue...")
        exit(1)
    try:
        print(f"Opening log file \"{sys.argv[1]}\".")
        allMessages = sm.unpackMessages(sys.argv[1])
    except FileNotFoundError:
        print(f"Invalid file name passed \"{sys.argv[1]}\".")
        input("Press Enter to continue...")
        exit(1)
    print("Sorting log messages.")
    allMessages.sort(key=lambda tup: tup[1]) # Sorting by timestamp

    #allMessages = convertMessages(allMessages, sm.accelerationRawDataID, sm.accelerationMpssDataID)
    #allMessages = convertMessages(allMessages, sm.gyroscopeRawDataID, sm.gyroscopeRpsDataID)
    print("Converting raw pressure data to PSIG.")
    allMessages = convertMessages(allMessages, sm.pressureRawDataID, sm.pressurePSIGDataID)
    print("Converting raw thermocouple data to celcius.")
    allMessages = convertMessages(allMessages, sm.thermocoupleRawDataID, sm.thermocoupleRawDataID)
    #allMessages = convertMessages(allMessages, sm.cpuTemperatureRawDataID, sm.cpuTemperatureCelciusDataID)

    #sm.writeLog(allMessages, "formattedData.log")
    #sm.writeCSV(allMessages, sm.accelerationMpssDataID, "Acceleration.csv")
    #sm.writeCSV(allMessages, sm.gyroscopeRpsDataID, "Gyroscope.csv")
    #sm.writeCSV(allMessages, sm.pressurePSIGDataID, "Pressure.csv")
    #sm.writeCSV(allMessages, sm.thermocoupleRawDataID, "Temperature.csv")
    #sm.writeCSV(allMessages, sm.hallEffectDataID, "HallEffect.csv")
    #sm.writeCSV(allMessages, sm.cpuTemperatureCelciusDataID, "CPU_Temperature.csv")
    sm.writeCSV(allMessages, sm.CSV_ALL_SENSOR_MESSAGES, "AllData.csv")
