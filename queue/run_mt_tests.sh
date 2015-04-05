#!/bin/bash

cd $HOME/trac/queue/

outfile=$(hostname)_mt_test_output.txt

rm -f $outfile

./mt_test 128 10 1 | grep Thread >> $outfile

for qs in 128 192 256 1024 2048
do
    for nanosecs in 1 10 50 100 200 500 1000 2000
    do
        ./mt_test $qs 10000000 $nanosecs | grep -i cycles_per_iter >> $outfile
    done
done
