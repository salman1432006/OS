#!/bin/bash

echo "===== Pipeline Stress Test ====="

for i in {1..5}
do
    echo "Pipeline test $i"
    ls /usr/bin | grep -E "gcc|python|bash" | wc -l
done

echo "===== Stress Test Complete ====="