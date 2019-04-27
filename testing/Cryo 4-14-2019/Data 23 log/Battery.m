close all;
clear all;

load("battery.mat");
load("pressure.mat");
timestamp = (Timestamp / 1000) / 60;
voltage = (VoltageRaw / 4095) * 5 * 11;
yyaxis left;
plot(timestamp, voltage);
xlabel("Time (minutes)");
ylabel("Voltage (volts)");
yyaxis right;
plot(timestamp, Methane, 'm', timestamp, LOX, 'g', timestamp, Helium, 'b');
ylabel("Pressure (PSIG)");

legend("Battery voltage", "Methane", "LOX", "Helium", "Location", "West");
title("Medium sized battery voltage and pressure over time");
