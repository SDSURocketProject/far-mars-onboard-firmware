close all;
clear all;

load("pressure.mat");
timestamp = (Timestamp / 1000) / 60;

% Chamber PT was unused for this test
plot(timestamp, Methane, timestamp, LOX, timestamp, Helium);

xlabel("Time (minutes)");
ylabel("Pressure (PSIG)");
legend("Methane", "LOX", "Helium");
title("Pressure data from Cryo 4-14-2019 Dead battery");
