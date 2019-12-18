echo -e "Serial:"
./bin/serial_router ./testing\ data/gr$1x$1.in ./output\ data/$1.out
./verify\ program/verify_linux ./testing\ data/gr$1x$1.in ./output\ data/$1.out
for ((i=1; i<=$2; i++))
do
	echo -e "\nThread count = $i:"
	./bin/router ./testing\ data/gr$1x$1.in ./output\ data/$1.out $i
	./verify\ program/verify_linux ./testing\ data/gr$1x$1.in ./output\ data/$1.out
done
