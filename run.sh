TIMEFORMAT=$'%3lR'

echo -e "Serial:"
echo -n "Execution time: "
time ./bin/serial_router ./testing\ data/gr$1x$1.in ./output\ data/$1.out
./verify\ program/verify_linux ./testing\ data/gr$1x$1.in ./output\ data/$1.out

for ((i=2; i<=$2; i++))
do
    echo -e "\nThread count = $i:"
    echo -n "Execution time: "
    time ./bin/router ./testing\ data/gr$1x$1.in ./output\ data/$1.out $i
    ./verify\ program/verify_linux ./testing\ data/gr$1x$1.in ./output\ data/$1.out
done

TIMEFORMAT=$'\nreal\t%3lR\nuser\t%3lU\nsys\t%3lS'
   