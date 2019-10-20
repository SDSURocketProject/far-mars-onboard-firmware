close all;
clear all;

load("Data_14.mat");
timestampMinutes = (Timestamp / 1000) / 60;

% Chamber PT was unused for this test
plot(timestampMinutes, Methane, timestampMinutes, LOX, timestampMinutes, Helium);

xlabel("Time (minutes)");
ylabel("Pressure (PSIG)");
legend("Methane", "LOX", "Helium", "Location", "Northwest");
title("Pressure data from Cryo 10-19-2019");
