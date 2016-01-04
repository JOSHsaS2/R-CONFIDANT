function [ results] = Plot_memory( prefix, node_size, time_range, interval, fcontents)
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here

s_lut = strcat(prefix,'LUT_SIZE.csv');
s_firsthand = strcat(prefix,'Firsthand_SIZE.csv');
s_reputation = strcat(prefix ,'Rating_SIZE.csv');
s_trust = strcat(prefix, 'Trust_SIZE.csv');

results = zeros(1,interval+1);


if fcontents(1) ~= 0
    M_LUT = csvread(s_lut, 1, 0);
    tmp = Analysis(M_LUT, 4, time_range, interval, node_size);
    results = tmp + results;
end


if fcontents(2) ~= 0
    M_FIRSTHAND = csvread(s_firsthand, 1, 0);
    tmp = Analysis(M_FIRSTHAND, 20, time_range, interval, node_size);
    results = tmp + results;
end


if fcontents(3) ~= 0
    M_REPUTATION = csvread(s_reputation, 1, 0);
    tmp = Analysis(M_REPUTATION, 20, time_range, interval, node_size);
    results = tmp + results;
end


if fcontents(4) ~= 0
    M_TRUST = csvread(s_trust, 1, 0);
    tmp = Analysis(M_TRUST, 20, time_range, interval, node_size);
    results = tmp + results;
end

end

