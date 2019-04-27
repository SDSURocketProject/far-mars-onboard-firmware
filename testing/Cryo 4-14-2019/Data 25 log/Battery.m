close all;
clear all;

load("battery.mat");
timestamp = (Timestamp / 1000) / 60;
voltage = (VoltageRaw / 4095) * 5 * 11;
plot(timestamp, voltage);

xlabel("Time (minutes)");
ylabel("Voltage (volts)");
legend("Battery voltage");
title("Data log 25 battery voltage");
