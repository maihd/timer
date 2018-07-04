#include <stdio.h>

#define TIMER_CPP_IMPL
#include "timer.hpp"

int main(int argc, char* argv[])
{
    timer t(60);
    
    while (true)
    {
        timer::start(t);
        timer::stop(t);
        timer::sleep(t);
        
        printf("timer.fps = %lf\n", (double)t.fps);
        printf("timer.seconds = %lf\n", (double)t.seconds);
    }

    return 0;
}