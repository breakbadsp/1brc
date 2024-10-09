#!/bin/sh

rm -f ./bin/analyse
make -j4

for i in {1..10}; do
  time ./bin/analyse ./input/input_file.txt > /dev/null;
  sleep 10
done
