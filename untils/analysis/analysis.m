function [ result ] = Analysis( csv_m, size_item, max_time, interval, node_n )
% function 'Analysis' is used to generate vectors for plotting results 
% related to Memory Usage and Remaining Power of different protocols 

%   csv_m: the data matrix read from a .csv file.
%   size_item: unit memory usage each item in table; for Remaing Power, 
%   set it as 1.
%   max_time: total running time of one simulation.
%   interval: number of time interval displayed in figures plotted 
%   (default: 5).
%   node_n: total number of nodes in one simulation.

%   result: 1xX vertor, each element indicates the memory or power usage
%   for corresponding time interval.

result = zeros(1, interval+1);
t = max_time / interval;
id = csv_m(:,1);
time = csv_m(:,2);
number = csv_m(:,3);

tmp_number = zeros(1,node_n);
for s = 1:interval
    tmp = 0; 
    for j = 1:node_n
        c_time = time((time > (s-1)*t)& (time <= s * t) & (id == j-1));
        [r, c] = size(c_time); 
        if(r == 0)
            tmp = tmp + tmp_number(j);
        else
        c_number = number((time > (s-1)*t) &(time <= s * t) & (id == j-1));
        [v, index] = max(c_time);
        tmp = tmp + c_number(index);
        tmp_number(j) = c_number(index);
        end
    end
    result(1+s) = tmp;
end
result = result * size_item / node_n;
end

