#include "timer.h"
#include <ctime>

void Timer::Begin(void)
{
    begin_clock = clock();
}

double Timer::End(void)
{
    return ((double)(clock() - begin_clock) / (double)CLOCKS_PER_SEC);
}