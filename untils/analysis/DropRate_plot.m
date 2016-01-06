% This script aims to plot a bar chart about drop rate of simulation which
% nodes are equipped with defferent protocols: 
% AODV without selfish nodes, AODV, CONFIDANT, L-CONFIDANT, L-CONFIDANT-TRADEOFF. 

prompt = 'Drop Rate: input prefix name of *.csv files\n';
c_prefix = input(prompt, 's');
s_params = input('parameters: ', 's');
file_name = 'drop_rate.csv';
M = csvread([c_prefix, file_name], 1,0);
% PLOTING FIGURES
string_lables = {'AODV_0_M'; 'AODV';'CONFIDANT';'L-CONFIDANT'; 'L-CONFIDANT-TRADEOFF'};
bar(M);
title(['Drop Rate_', s_params]);
ylabel('Drop rate');
xlabel('Protocol');
set(gca, 'XTickLabel',string_lables, 'XTick',1:numel(string_lables));
