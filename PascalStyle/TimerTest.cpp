#include <stdio.h>

#define TIMER_IMPL
#include "Timer.h"

int main(int argc, char* argv[])
{
    Timer timer(60); 
    
    while (true)
    {
        Timer::NewFrame(timer);

        Timer::EndFrame(timer);

        // Working with info of timer
        Timer::Sleep(timer);
        
        printf("Framerate: %lf\n", Timer::GetFrameRate(timer));
    }

    return 0;
}