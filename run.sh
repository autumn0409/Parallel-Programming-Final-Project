if [ $1 = "serial" ]
then
    ./bin/serial_router ./testing\ data/gr$2x$2.in ./output\ data/$2.out
elif [ $1 = "parallel" ]
then 
    ./bin/router ./testing\ data/gr$2x$2.in ./output\ data/$2.out
fi
./verify\ program/verify_linux ./testing\ data/gr$2x$2.in ./output\ data/$2.out
