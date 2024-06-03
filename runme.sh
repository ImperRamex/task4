make build_server
make build_client
touch /tmp/server.log



./server &
sleep 1
echo "task1:" >> result.txt
./task1.sh
echo "" >> result.txt
echo "task2:" >> result.txt
./task1.sh
echo "" >> result.txt
./task1.sh
echo "" >> result.txt
echo "Descriptor num and sbrk()" >> result.txt
echo "First accept:" >> result.txt
cat /tmp/server.log | grep "Accept" | head -n 1 >> result.txt
echo "Last accept:" >> result.txt
cat /tmp/server.log | grep "Accept" | tail -n 1 >> result.txt
echo "task3:" >> result.txt
./task3.sh

