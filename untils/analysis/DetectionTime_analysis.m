% This script is used to plot the average selfish nodes detection time for
% nodes with 3 different CONFIDANT protocols in one figure.

% Note: The format and repuirment of inputs can be found in script
% 'memory_analysi.m'.

prompt = 'CONFIDANT: input prefix name of *.csv files\n';
c_prefix = input(prompt, 's');
prompt = 'L-CONFIDANT: input prefix name of *.csv files\n';
l_prefix = input(prompt,'s');
prompt = 'L-CONFIDANT-TRADEOFF: input prefix name of *.csv files\n';
trade_prefix = input(prompt,'s');
prompt = 'input string of parameters used specific format\n';
s_params = input(prompt, 's');

selfish_size = input('selfish nodes size\n');
node_size = input('All node size\n');
time_range = input('time_range\n');
interval = input('interval\n');

record_file_name = 'selfish_node record.csv';
detection_file_name = 'SelfishNode_DetectionTime.csv';

if selfish_size ~= 0
    C_record = csvread([c_prefix, s_params, record_file_name], 1, 0);
    C_detection = csvread([c_prefix, s_params, detection_file_name], 1, 0);
    C_result = Selfish_Node_analysis(C_record, C_detection, time_range, interval, selfish_size, node_size);
    
    LC_record = csvread([l_prefix, s_params, record_file_name], 1, 0);
    LC_detection = csvread([l_prefix, s_params, detection_file_name], 1, 0);
    LC_result = Selfish_Node_analysis(LC_record, LC_detection, time_range, interval, selfish_size, node_size);
    
    T_record = csvread([trade_prefix, s_params, record_file_name], 1, 0);
    T_detection = csvread([trade_prefix, s_params, detection_file_name], 1, 0);
    T_result = Selfish_Node_analysis(T_record, T_detection, time_range, interval, selfish_size, node_size);
    
    % PLOTING FIGURES
    X_AXY = (0:interval) * time_range / interval;
    figure
    plot(X_AXY,C_result,'-gs', X_AXY,LC_result,'-ro', X_AXY,T_result, '-.b');
    %plot(X_AXY,results_tradeoff);
    %hold off
    legend('CONFIDANT', 'L-CONFIDANT', 'TRADEOFF-CONFIDANT');
    title(['Detection Time of selfish nodes_', s_params]);
    ylabel('rate of detected selfish nodes (%)');
    xlabel('time (s)');
else
    disp('no selfish node!');
end
