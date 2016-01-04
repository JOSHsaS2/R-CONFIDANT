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
C_fcontents = input('CONFIDANT: input 0 or 1 to indicate whether the corresponding file is empty.\n 0 represent empty file, otherwise 1\n');
LC_fcontents = input('L-X-CONFIDANT: input 0 or 1 to indicate whether the corresponding file is empty.\n 0 represent empty file, otherwise 1\n');

result_confidant = Plot_memory([c_prefix,s_params], node_size, time_range, interval, C_fcontents);
result_lc = Plot_memory([l_prefix, s_params], node_size, time_range, interval, LC_fcontents);
result_tradeoff = Plot_memory([trade_prefix, s_params], node_size, time_range, interval, LC_fcontents);

% PLOTING FIGURES
X_AXY = (0:interval) * time_range / interval;
%plot(X_AXY,results_confidant);
%hold on
figure
plot(X_AXY,result_confidant,'-gs', X_AXY,result_lc,'-ro', X_AXY,result_tradeoff, '-.b');
%plot(X_AXY,results_tradeoff);
%hold off
legend('CONFIDANT', 'L-CONFIDANT', 'TRADEOFF-CONFIDANT');
title(['average memory usage of nodes_', s_params]);
ylabel('size (Byte)');
xlabel('time (s)');


