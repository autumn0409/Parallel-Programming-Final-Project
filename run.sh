for ((i=1; i<=$2; i++))
do
	echo -e "Thread count = $i:"
	./bin/router ./testing\ data/gr$1x$1.in ./output\ data/$1.out $i
	./verify\ program/verify_linux ./testing\ data/gr$1x$1.in ./output\ data/$1.out
    if [ $i -lt $2 ]
    then
        echo -e ""
    fi
done
