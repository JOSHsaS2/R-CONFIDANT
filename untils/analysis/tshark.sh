#!/bin/sh

## USAGE: ./tshark.sh /path/to/Pcap/files/directory/pcap_file_prefix_name node_size /path/to/csv/files/directory/csv_prefix_name
#place analysis_packet.sh in the same directory

macaddrbase="00:00:00:00:00:"
ipaddrbase="10.1.1." 

#/path/to/Pcap/files/directory/prefix name of Pcap files
preindex=$1

#total node size
count=$2

f_drop="drop_rate.csv"
f_overhead="overhead.csv"

#/path/to/generated/csv/files/directory/csv_prefix_name
prefix=$3

i=0

#number of overhead
packetcount=0
#number of data packets received
received=0
#number of data packets sent
sent=0

while [ $i -lt $count ]; do
     s=""
     j=`expr $i + 1`
     if [ $i -lt 10 ]; then
        s=$( printf '%02d' $j )
     else
        s=$j
     fi    
     filter="wlan.sa==$macaddrbase$s and udp.dstport==654"
     filter2="not icmp and ip.src==$ipaddrbase$j and udp.dstport==9 and wlan.fc.retry==0"
     filter3="not icmp and ip.dst==$ipaddrbase$j and udp.dstport==9"
     result=$(tshark -r $preindex-$i-0.pcap -Y "$filter" | wc -l)
     r=$(tshark -r $preindex-$i-0.pcap -Y "$filter3" | wc -l)
     r_sent=$(tshark -r $preindex-$i-0.pcap -Y "$filter2" | wc -l)
     packetcount=`expr $packetcount + $result`
     received=`expr $received + $r`
     sent=`expr $sent + $r_sent`
     i=`expr $i + 1`
done
echo "total received: $received, total sent: $sent"
droprate=$(echo "scale=5; ($sent - $received) / $sent" |bc)
echo "droptate: $droprate; overload: $packetcount"
printf "$droprate\n" >> $prefix$f_drop
printf "$packetcount\n" >> $prefix$f_overhead



