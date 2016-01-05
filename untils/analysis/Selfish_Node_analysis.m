function [ result ] = Selfish_Node_analysis( m_record, m_detection, max_time, interval, selfish_size, node_size )
%UNTITLED2 Summary of this function goes here
%   Detailed explanation goes here
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

