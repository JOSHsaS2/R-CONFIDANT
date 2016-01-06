function [ result ] = Selfish_Node_analysis( m_record, m_detection, max_time, interval, selfish_size, node_size )
%function Selfish_Node_analysis shows to what extent all nodes in a
%simulation have already detected all selfish nodes.

%   m_record: the data matrix read from a 'xxx_selfish_node record.csv' file.
%   m_detection: the data matrix read from a 'xxx_SelfishNode_DetectionTime.csv' file.
%   max_time: total running time of one simulation.
%   interval: number of time interval displayed in figures plotted 
%   (default: 5).
%   node_size: total number of nodes in one simulation.
%   selfish_size: number of selfish nodes.

%   average percentage of detected selfish nodes for all nodes during
%   specific time intervals.

[r,c] = size(m_record);
if r~=selfish_size
    disp('number of selfish nodes do not match, maybe due to providing wrong files');
    result = -1;
else
    result = 0:node_size-1;
    d_time = m_detection(:,2);
    d_node = m_detection(:,3);
    d_nid = m_detection(:,1);
    for j = 1:selfish_size
        tmp_time_all = [];
        for s = 0:node_size-1
            tmp_time = d_time((d_node == m_record(j)) & (d_nid == s));
            [r,c] = size(tmp_time);
            if r == 0
                tmp_time_all = [tmp_time_all, max_time+1];
            else
                [v, index] = min(tmp_time);
                tmp_time_all = [tmp_time_all, v];
            end
        end
        result = [result; tmp_time_all];
    end
    time_interval = max_time / interval;
    c_result = result;
    result = zeros(1, interval+1);
    for k = 1:interval
        [r,c] = size(c_result(c_result <= k*time_interval));
        result(k+1) = r/(selfish_size* node_size);
    end
end
end

