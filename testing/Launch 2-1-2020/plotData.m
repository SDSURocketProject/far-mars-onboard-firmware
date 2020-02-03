clear all;
close all;

load('Data5.mat')

TimestampMinutes = (Timestamp / 1000) / 60;

darkBlue = [0 0.4470 0.7410];
darkOrange = [0.8500 0.3250 0.0980];
lightBlue = [0.3010 0.7450 0.9330];
darkRed = [0.6350 0.0780 0.1840];

pressureMax = max([PT_Methane PT_LOX PT_Helium]);

graphStartTime = 146; % Minutes
graphEndTime = 154; % Minutes

yyaxis left
hold on
plot(TimestampMinutes, PT_Methane, 'Color', darkBlue, 'LineStyle', '-');
plot(TimestampMinutes, PT_LOX, 'Color', darkOrange, 'LineStyle', '-');
plot(TimestampMinutes, PT_Helium, 'Color', lightBlue, 'LineStyle', '-');
%plot(TimestampMinutes, HE_Methane*max(pressureMax), 'Color', darkBlue, 'LineStyle', '--');
%plot(TimestampMinutes, HE_LOX*max(pressureMax), 'Color', darkOrange, 'LineStyle', '--', 'Marker', 'none');
xlabel("Time (minutes)");
ylabel("Pressure (PSIG)");
axis([graphStartTime graphEndTime -5000 5000]);

yyaxis right
hold on
plot(TimestampMinutes, TC_UAF, 'Color', darkRed, 'LineStyle', '-');
ylabel("Temperature (Celcius)");
axis([graphStartTime graphEndTime -150 150]);

%legend("Methane Pressure", "LOX Pressure", "Helium Pressure", "Methane Vent Valve", "LOX Vent Valve", "UAF Temperature", "Location", "Southwest");
legend("Methane Pressure", "LOX Pressure", "Helium Pressure", "UAF Temperature", "Location", "Southwest");
title("Data from Cryo 10-19-2019");
