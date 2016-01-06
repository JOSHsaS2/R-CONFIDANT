function [ results] = Plot_memory( prefix, node_size, time_range, interval, fcontents)
%function 'Plot_memory' calculate the final combined memory usage used
%to plot, which includes memory usage from tables of LUT, FIRSTHAND, RATING,
%and TRUST.

%   prefix: /path/to/csv/files/directory/name_prefix_csv.
%   node_size: total number of nodes in one simulation.
%   time_range: total running time of one simulation.
%   interval: number of time interval displayed in figures plotted.

%   fcontents: 1x4 vectors used to indicate whether .csv files are empty or
%   not. 0 represens empty, otherwise 1.
%   fcontents(1): LUT table.
%   fcontents(1): Firsthand infomation table.
%   fcontents(1): Reputation Rating table.
%   fcontents(1): Trust table.
%   e.g. [0 1 1 1] means xxx_LUT_SIZE.csv is empty, other instead.

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

