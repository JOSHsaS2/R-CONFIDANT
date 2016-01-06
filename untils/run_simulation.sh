#!/bin/sh
## USAGE: ./debug.sh /path/to/ns-3.22/root/directory output_directory_name
#  The directory for output of simulation is located in the PARENT directory of ns-3.22 root direcroty

#  protocol_type: 0=AODV; 1=CONFIDANT; 2=L-CONFIDANT; 3=TradeOff-L-CONFIDANT 

rd_name=$2

cd $1
cd ..

rm -rf $rd_name

mkdir $rd_name
mkdir $rd_name/aodv_0_m $rd_name/aodv $rd_name/confidant $rd_name/l-confidant $rd_name/tradeoff-l-confidant

cd $1/ns-3.22

## Change number of sefish nodes here
selfish_count=6

## protocol type id
j=0

for i in "aodv" "confidant" "l-confidant" "tradeoff-l-confidant" "aodv_0_m"; do
    if [ $j -gt 4 ]; then
       exit
    fi
    tmp=$selfish_count
    tmp_j=$j
    if [ $j -eq 4 ]; then
       tmp=0
       j=0
    fi
    
## run simulation
    NS_LOG="manet-routing-compare" 
    ./waf --cwd="../../$rd_name/$i" --run manet-routing-compare --command-template="%s --protocol=$j --pauseTime=5 --totalNodes=20 --sinks=7 --sizeX=1000 --sizeY=1000 --totalTime=400 --malicious=$tmp"
     j=$tmp_j  
     j=`expr $j + 1`

done


