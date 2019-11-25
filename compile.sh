if [ $1 = "serial" ]
then
    g++ ./code/serial/serial_router.cpp ./code/serial/parser.cpp ./code/serial/timer.cpp -o ./bin/serial_router
elif [ $1 = "parallel" ]
then 
    g++ ./code/parallel/router.cpp ./code/parallel/parser.cpp ./code/serial/timer.cpp -o ./bin/router
fi