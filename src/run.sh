#!/bin/bash
files=("../traces/fp_1" "../traces/fp_2" "../traces/int_1" "../traces/int_2" "../traces/mm_1" "../traces/mm_2")

for file in "${files[@]}"; do
    ./predictor --static "$file"
done

echo "==============="
for file in "${files[@]}"; do
    ./predictor --gshare:13 "$file"
done

echo "==============="
for file in "${files[@]}"; do
    ./predictor --tournament:9:10:10 "$file"
done

