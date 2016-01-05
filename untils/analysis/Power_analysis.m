prompt = 'CONFIDANT: input prefix name of *.csv files\n';
c_prefix = input(prompt, 's');
prompt = 'L-CONFIDANT: input prefix name of *.csv files\n';
l_prefix = input(prompt,'s');
prompt = 'L-CONFIDANT-TRADEOFF: input prefix name of *.csv files\n';
trade_prefix = input(prompt,'s');
prompt = 'input string of parameters used specific format\n';
s_params = input(prompt, 's');

node_size = input('node_size\n');
time_range = input('time_range\n');
interval = input('interval\n');

file_name = 'Remaining_Power.csv';

C_power = csvread([c_prefix, s_params, file_name], 1, 0);
C_result = Analysis(C_power, 1, time_range, interval, node_size);

LC_power = csvread([l_prefix, s_params, file_name], 1, 0);
LC_result = Analysis(LC_power, 1, time_range, interval, node_size);

T_power = csvread([trade_prefix, s_params, file_name], 1, 0);
T_result = Analysis(T_power, 1, time_range, interval, node_size);

% PLOTING FIGURES
X_AXY = (1:interval) * time_range / interval;
figure
plot(X_AXY,C_result(2:interval+1),'-gs', X_AXY,LC_result(2:interval+1),'-ro', X_AXY,T_result(2:interval+1), '-.b');
%plot(X_AXY,results_tradeoff);
%hold off
legend('CONFIDANT', 'L-CONFIDANT', 'TRADEOFF-CONFIDANT');
title(['average remaining power of nodes_', s_params]);
ylabel('remaining power (J)');
xlabel('time (s)');
