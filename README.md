# Parallel-Programming-Project

(The compile and run script only works in the linux environment)

## Compile

* Serial code:

`./compile.sh serial`

* Parallel code:

`./compile_jacob.sh parallel`

## Run

* Serial code:

`./run.sh serial [input size]` (ex: `./run.sh serial 20`)

* Parallel code:

`./run.sh parallel [input size] [thread count]` (ex: `./run.sh parallel 60 4`)

* Serial + Parallel code:

`./run_for.sh [input size] [thread count]` (ex: `./run_for.sh 60 4`)

## Reference

* [Global routing ppt](http://cc.ee.ntu.edu.tw/~eda/Course/IntroEDA06/LN/routing1.pdf)
  
* [NCTU-GR 2.0: Multithreaded Collision-Aware
Global Routing with Bounded-Length Maze Routing](https://ir.nctu.edu.tw/bitstream/11536/21646/1/000318163800005.pdf)

* [Global and detailed
routing](http://cc.ee.ntu.edu.tw/~ywchang/Courses/PD_Source/EDA_routing.pdf)
