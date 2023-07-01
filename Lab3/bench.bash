#!/bin/bash
for ic in 3, 6 16 32
do
for oc in 1, 2, 8, 32 64
do
for k in 3 5 7 11
do
  for m in 0 1 2
  do
    ./lab3.bin 1 $m $ic 128 $k $oc 10
  done
done
done
done


