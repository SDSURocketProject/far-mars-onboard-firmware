close all;
clear all;

load("Data_14.mat");
PressureTimestampMinutes = (PressureTimestamp / 1000) / 60;
HallTimestampMinutes = (HallTimestamp / 1000) / 60;
ThermocoupleTimestampMinutes = (ThermocoupleTimestamp / 1000) / 60;

darkBlue = [0 0.4470 0.7410];
darkOrange = [0.8500 0.3250 0.0980];
lightBlue = [0.3010 0.7450 0.9330];
darkRed = [0.6350 0.0780 0.1840];

pressureMax = max([PressureMethane PressureLOX PressureHelium]);

yyaxis left
hold on
plot(PressureTimestampMinutes, PressureMethane, 'Color', darkBlue, 'LineStyle', '-');
plot(PressureTimestampMinutes, PressureLOX, 'Color', darkOrange, 'LineStyle', '-');
plot(PressureTimestampMinutes, PressureHelium, 'Color', lightBlue, 'LineStyle', '-');
plot(HallTimestampMinutes, HallMethane*max(pressureMax), 'Color', darkBlue, 'LineStyle', '--');
plot(HallTimestampMinutes, HallLOX*max(pressureMax), 'Color', darkOrange, 'LineStyle', '--', 'Marker', 'none');
xlabel("Time (minutes)");
ylabel("Pressure (PSIG)");
axis([220 270 -400 400]);

yyaxis right
hold on
plot(ThermocoupleTimestampMinutes, ThermocoupleUAF, 'Color', darkRed, 'LineStyle', '-');
ylabel("Temperature (Celcius)");
axis([220 270 -40 40]);

legend("Methane Pressure", "LOX Pressure", "Helium Pressure", "Methane Vent Valve", "LOX Vent Valve", "UAF Temperature", "Location", "Southwest");
title("Data from Cryo 10-19-2019");
