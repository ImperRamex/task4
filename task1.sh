#!/bin/bash

echo "Start 100 clients" >> result.txt

for i in {1..100}
do
    (./client $i 1000 < numbers) >> /dev/null &
    sleep 0.005
done

wait

echo "100 clients finished" >> result.txt
echo "Check zero state" >> result.txt

touch zero
echo 0 > zero

./client 101 1 < zero >> /dev/null

tail -n 1 /tmp/server.log >> result.txt

rm -f zero
