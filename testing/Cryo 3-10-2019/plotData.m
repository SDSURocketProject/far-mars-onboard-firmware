
% Log started at about 1:30pm
% Indexes are 10ms apart

load ("Cryo PT 3-10-2019.mat")

% Cryo
min = 1530000; % ~5:45pm
max = 1670000; % ~23 minutes total
plot(timestamp(min:max), methane(min:max), timestamp(min:max), lox(min:max), timestamp(min:max), helium(min:max));
legend("Timestamp", "Methane", "LOX", "Helium")
xlabel("Time (ms)")
ylabel("Pressure (PSIG)")
title("Cryo Data 3/10/2019")

% Other points of interest

% Leak Check, I think
% min = 350000; % ~2:30pm
% max = 500000;
% plot(timestamp(min:max), methane(min:max), timestamp(min:max), lox(min:max), timestamp(min:max), helium(min:max));

% Leak Check, I think
% min = 1150000; % ~4:45pm
% max = 1200000;
% plot(timestamp(min:max), methane(min:max), timestamp(min:max), lox(min:max), timestamp(min:max), helium(min:max));

