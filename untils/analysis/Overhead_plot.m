prompt = 'Overhead: input prefix name of *.csv files\n';
c_prefix = input(prompt, 's');
s_params = input('parameters: ', 's');
file_name = 'overhead.csv';
M = csvread([c_prefix, file_name], 1,0);
% PLOTING FIGURES
string_lables = {'AODV_0_M';'AODV';'CONFIDANT';'L-CONFIDANT'; 'L-CONFIDANT-TRADEOFF'};
bar(M);
title(['Overhead_', s_params]);
ylabel('Overhead (times)');
xlabel('Protocol');
set(gca, 'XTickLabel',string_lables, 'XTick',1:numel(string_lables));
