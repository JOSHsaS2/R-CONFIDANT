#!/bin/bash

## USAGE: ./analysis_packet.sh node_size /path/to/generated/csv/files/directory/csv_prefix_name /path/to/Pcap/files/directory/prefix_name_aodv_without_selfish_nodes /path/to/Pcap/files/directory/prefix_name_aodv /path/to/Pcap/files/directory/prefix_name_confidant /path/to/Pcap/files/directory/prefix_name_l-confidant /path/to/Pcap/files/directory/prefix_name_tradeoff
#place tshark.sh in the same directory

#Number of total nodes
node_size=$1

#/path/to/generated/csv/files/directory (Drop rate & Overhead statistics)
path=$2

#name prefix of .csv files
f_prefix=$3 

#/path/to/Pcap/files/directory/prefix name of Pcap files:

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


drop="drop_rate.csv"
overhead="overhead.csv"

# write title names into new generated .csv files
printf "Drop rate\n" >> $path$f_prefix$drop
printf "Overhead\n" >> $path$f_prefix$overhead

for i in $pcap_prefix_aodv_0_m $pcap_prefix_aodv $pcap_prefix_c $pcap_prefix_lc $pcap_prefix_t; do
    ./tshark.sh $i $node_size $path$f_prefix
done
