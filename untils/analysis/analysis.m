function [ result ] = Analysis( csv_m, size_item, max_time, interval, node_n )
%Analysis Summary of this function goes here
%   Detailed explanation goes here

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

