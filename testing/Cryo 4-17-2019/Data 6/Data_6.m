clear all;
close all;

load("Data 6.mat");
voltage = (VoltageRaw/4095)*4.959*11;
plot(VoltageTimestamp, voltage);