M_LUT = csvread('test.csv', 1, 0);
M_FIRSTHAND = csvread('', 1, 0);
M_REPUTATION = csvread('', 1, 0);
M_TRUST = csvread('', 1, 0);

node_size = 20;
time_range = 400;
interval = 5;

results = zeros(1,interval+1);
table_size = size(M_LUT);
if table_size(1) ~= 0
    tmp = Analysis(M_LUT, 4, time_range, interval, node_size);
    results = tmp + results;
end

table_size = size(M_FIRSTHAND);
if table_size(1) ~= 0
    tmp = Analysis(M_FIRSTHAND, 20, time_range, interval, node_size);
    results = tmp + results;
end

table_size = size(M_REPUTATION);
if table_size(1) ~= 0
    tmp = Analysis(M_REPUTATION, 20, time_range, interval, node_size);
    results = tmp + results;
end

table_size = size(M_TRUST);
if table_size(1) ~= 0
    tmp = Analysis(M_TRUST, 20, time_range, interval, node_size);
    results = tmp + results;
end

X_AXY = (0:interval) * time_range / interval;

plot(X_AXY,results);
title('average memory usage of nodes');
ylabel('size (B)');
xlabel('time (s)');


