#!/bin/sh

macaddrbase="00:00:00:00:00:"
ipaddrbase="10.1.1." 
preindex=$1
count=$2

i=0
packetcount=0
received=0
sent=0

while [ $i -lt $count ]; do
     s=""
     j=`expr $i + 1`
     if [ $i -lt 10 ]; then
        s=$( printf '%02d' $j )
     else
        s=$j
     fi
     #echo $i
     
     filter="wlan.sa==$macaddrbase$s and udp.dstport==654"
     filter2="not icmp and ip.src==$ipaddrbase$j and udp.dstport==9"
     filter3="not icmp and ip.dst==$ipaddrbase$j and udp.dstport==9"
     #echo $filter2, $filter3  
     result=$(tshark -r $preindex-$i-0.pcap -Y "$filter" | wc -l)
     r=$(tshark -r $preindex-$i-0.pcap -Y "$filter3" | wc -l)
     r_sent=$(tshark -r $preindex-$i-0.pcap -Y "$filter2" | wc -l)
     #echo $r, $r_sent
     packetcount=`expr $packetcount + $result`
     received=`expr $received + $r`
     sent=`expr $sent + $r_sent`
     i=`expr $i + 1`
done

droprate=$(echo "scale=5; $received / $sent" |bc)
echo "droptate: $droprate; overload: $packetcount"



