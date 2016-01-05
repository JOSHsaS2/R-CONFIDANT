#!/bin/sh

#packet prefix for:
#aodv without selfish nodes
pcap_prefix_aodv_0_m=$4

#aodv with selfish nodes
pcap_prefix_aodv=$5

#confidant with selfish nodes
pcap_prefix_c=$6

#l-confidant with selfish nodes
pcap_prefix_lc=$7

#tradeoff-confidant with selfish nodes
pcap_prefix_t=$4

#No. of nodes
node_size=$1

#path name of generated .csv files
path=$2

#prefix name of .csv files
f_prefix=$3 

drop="drop_rate.csv"
overhead="overhead.csv"

echo  $path$f_prefix$drop
printf "Drop rate\n" >> $path$f_prefix$drop
echo $path$f_prefix$overhead
printf "Overhead\n" >> $path$f_prefix$overhead

for i in $pcap_prefix_aodv_0_m $pcap_prefix_aodv $pcap_prefix_c $pcap_prefix_lc $pcap_prefix_t; do
    ./tshark.sh $i $node_size $path$f_prefix
done
