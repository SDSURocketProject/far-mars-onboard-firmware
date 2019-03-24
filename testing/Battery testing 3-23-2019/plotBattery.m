clear all;
close all;

load("battery.mat");

plot(minutes, voltage);
hold on;
legend("Battery voltage");
xlabel("Time (minutes)");
ylabel("Voltage (volts)");
title("Nano-tech 450mAh discharge curve");