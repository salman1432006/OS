#!/bin/bash

echo "===== Skill 22 Automated Test ====="

echo "Test 1: Normal input"
echo "Hello OSSP" | ./skill/Skill-22/skill22

echo
echo "Test 2: Empty input"
echo "" | ./skill/Skill-22/skill22

echo
echo "Tests completed."