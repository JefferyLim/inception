#!/bin/bash

# extract core number
CORE1=$(echo "$USERNAME" | grep -oE '[0-9]+$')

if [ -z "$CORE1" ]; then
    CORE1=0
    CORE2=6
fi

if [ -n "$1" ]; then
    CORE1="$1"
    CORE2=$((CORE1 + 6))
fi

taskset -c $CORE1 clang -no-pie -DZEN2 recursive_pcall.c -o recursive_pcall
taskset -c $CORE1 clang -no-pie -DSET=33 workload.c -o workload

