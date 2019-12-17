if [ $1 = "serial" ]
then
    g++ ./code/serial/serial_router.cpp ./code/util/parser.cpp ./code/util/timer.cpp -o ./bin/serial_router
elif [ $1 = "parallel" ]
then 
    g++ ./code/parallel_jacob/router.cpp ./code/util/parser.cpp ./code/util/timer.cpp -lpthread -o ./bin/router 
fi