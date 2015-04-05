#!/bin/bash

cd $HOME/trac/lock/

outfile=$(hostname)_lock_scalability_output.txt

rm -f $outfile

proc_num=$(cat /proc/cpuinfo | grep "processor" | wc -l)
./locks_scalability $proc_num 10000000 >> $outfile
