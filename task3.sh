clients=(20 60 100)
delays=(0 0.2 0.4 0.6 0.8 1)



for c in ${clients[@]}
do
    for d in ${delays[@]}
    do
        echo "Test clients: $c delay: $d" >> result.txt
        touch /tmp/client.log
        SECONDS=0
        for ((i=1; i<=$c;i++));
        do
            (./client $i 30 $d < numbers) >> /tmp/client.log &

        done
        wait
        client_time=$(cat /tmp/client.log | grep "Client time:" | grep -Eo '[0-9]+' | sort -rn | head -n 1)
        echo "Client time: $client_time" >> result.txt
        duration=$SECONDS
        effective_time=$((duration - client_time))
        echo "Total time: $duration" >> result.txt
        echo "Efficiency: $effective_time" >> result.txt
        rm /tmp/client.log
    done
done
