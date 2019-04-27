clear all;
close all;

load("Data 10.mat");
PressureTimestamp = (PressureTimestamp / 1000) / 60;
VoltageTimestamp = (VoltageTimestamp / 1000) / 60;
voltage = (VoltageRaw/4095)*4.959*11;
Methane = Methane-30;
LOX = LOX - 41;

min = 60000;
%max = 190000;
max = 190198;

plot(PressureTimestamp(min:max), Methane(min:max), PressureTimestamp(min:max), LOX(min:max), PressureTimestamp(min:max), Helium(min:max));
%plot(VoltageTimestamp, voltage);
legend("Methane", "LOX", "Helium", "Location", "Northwest");
xlabel("Time (minutes)");
ylabel("Pressure");
title("Pressure data from Cryo 4-17-2019");
