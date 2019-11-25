#ifndef _DEFINE_TIMER_
#define _DEFINE_TIMER_

class Timer
{
public:
    Timer(void) {}

    // Strat the timer
    void Begin(void);

    // Return the accumulated time in seconds
    double End(void);

private:
    long begin_clock;
};

#endif
