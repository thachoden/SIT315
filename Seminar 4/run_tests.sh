#!/bin/bash
# SIT315 Seminar 4 - builds both programs and prints a timing table (median of 3 runs)
# Usage: bash run_tests.sh
set -e
g++ -O2 -o VectorAdd VectorAdd.cpp 2>/dev/null
g++ -O2 -pthread -o vector_add_pthread vector_add_pthread.cpp

median() { sort -n | sed -n '2p'; }

echo "Machine: $(grep -m1 'model name' /proc/cpuinfo | cut -d: -f2 | xargs) | logical cores: $(nproc)"
echo

seq_us=$(for r in 1 2 3; do ./VectorAdd | grep -o '[0-9]\+ micro' | grep -o '[0-9]\+'; done | median)
echo "Sequential program (provided): ${seq_us} us"
echo

base=""
printf "%-8s %-15s %-12s %-22s %-20s %s\n" "Threads" "Partition size" "Time (us)" "Speedup vs sequential" "Speedup vs 1 thread" "Check"
for t in 1 2 4 8 16 32 64; do
  out=$(for r in 1 2 3; do ./vector_add_pthread $t; done)
  us=$(echo "$out" | grep -o 'taken: [0-9]\+' | grep -o '[0-9]\+' | median)
  part=$(echo "$out" | head -1 | grep -o 'size: [0-9]\+' | grep -o '[0-9]\+')
  chk=$(echo "$out" | grep -c PASS)
  [ -z "$base" ] && base=$us
  printf "%-8s %-15s %-12s %-22s %-20s %s\n" "$t" "$part" "$us" "$(echo "scale=2; $seq_us/$us" | bc)" "$(echo "scale=2; $base/$us" | bc)" "$chk/3 PASS"
done
