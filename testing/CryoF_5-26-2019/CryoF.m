clear all;
close all;

load("CryoF.mat");
PSIGTimestamp = (PSIGTimestamp / 1000) / 60;
VoltageTimestamp = (VoltageTimestamp / 1000) / 60;
voltage = (VoltageRaw/4095)*4.959*11;
Methane = Methane-31;
LOX = LOX-36;

% Remove spikes from Helium, graph is unreadable otherwise
last = Helium(1);
for i = 1:length(Helium)
    % Assuming Helium does not increase at a rate faster than 400PSIG/second
    if (abs(Helium(i)-last)>20)
        Helium(i) = last;
    end
    last = Helium(i);
end
% Lowpass helium so further remove spikes
Helium = lowpass(Helium, 1, 20);

min = 244000;
max = 256000;

plot(PSIGTimestamp(min:max), Methane(min:max), PSIGTimestamp(min:max), LOX(min:max), PSIGTimestamp(min:max), Helium(min:max));
%plot(VoltageTimestamp, voltage);
legend("Methane", "LOX", "Helium", "Location", "Northwest");
xlabel("Time (minutes)");
ylabel("Pressure");
title("Pressure data from Cryo 5-25-2019");
