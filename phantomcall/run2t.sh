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

taskset -c $CORE2 ./workload | taskset -c $CORE1 ./recursive_pcall
