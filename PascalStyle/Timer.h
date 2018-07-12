#ifndef __TIMER_H__
#define __TIMER_H__

#ifndef TIMER_API
#define TIMER_API
#endif

#ifndef TIMER_ASSERT
#define TIMER_ASSERT(exp, msg, ...) assert(exp)
#include <assert.h>
#endif

struct TIMER_API Timer
{
public: /* Functions */
    static bool NewFrame(Timer& timer);
    static void EndFrame(Timer& timer);

    static bool Sleep(Timer& timer, bool set = true);

    static double Seconds(const Timer& timer);
    static double GetFrameRate(const Timer& timer);

    static long   GetLimitFrameRate(const Timer& timer);
    static bool   SetLimitFrameRate(Timer& timer, long fps);

public:
    inline Timer(int fps = 60)
        : state(STOPPED)
        , start(0)
        , ticks(0)
    {
        TIMER_ASSERT(Timer::SetLimitFrameRate(*this, fps), "Internal buggy found");
    }

    inline ~Timer()
    {
        // EMPTY
    }

private: /* Fields */
    enum 
    {
        STARTED,
        STOPPED,
    } state;
    
    long start;
    long ticks;
    long limit;
};

namespace Performance
{
    TIMER_API bool NanoSleep(long ns);
    TIMER_API bool MicroSleep(long us);
    TIMER_API long GetCounter(void);
    TIMER_API long GetFrequency(void);
}

#endif /* __TIMER_H__ */

#ifdef TIMER_IMPL

bool Timer::NewFrame(Timer& timer)
{
    if (timer.state == Timer::STOPPED)
    {
        timer.state = Timer::STARTED;       
        timer.start = Performance::GetCounter();
        return true;
    }
    else
    {
        return false;
    }
}

void Timer::EndFrame(Timer& timer)
{
    if (timer.state == Timer::STARTED)
    {
        timer.state = Timer::STOPPED;
        timer.ticks = Performance::GetCounter() - timer.start;
    }
}

bool Timer::Sleep(Timer& timer, bool set)
{
    if (timer.state == Timer::STOPPED && timer.ticks < timer.limit)
    {
        double seconds = (timer.limit - timer.ticks) / (double)Performance::GetFrequency();
        Performance::MicroSleep((long)(1000 * 1000 * seconds));
        if (set)
        {
            timer.ticks = timer.limit;
        }
        return true;
    }
    else
    {
        return false;
    }
}

double Timer::Seconds(const Timer& timer)
{
    return timer.ticks / (double)Performance::GetFrequency();
}

double Timer::GetFrameRate(const Timer& timer)
{
    return 1.0f / Timer::Seconds(timer);
}

long   Timer::GetLimitFrameRate(const Timer& timer)
{
    return Performance::GetFrequency() / timer.limit;
}

bool   Timer::SetLimitFrameRate(Timer& timer, long fps)
{
    TIMER_ASSERT(fps > 0, "fps must be greater than zero, given fps is %d", fps);

    if (timer.state == Timer::STOPPED)
    {
        timer.limit = Performance::GetFrequency() / fps;
        return true;
    }
    else
    {
        return false;
    }
}

#if defined(_WIN32) || defined(__CYGWIN__)
#include <Windows.h>
namespace Performance
{
    bool NanoSleep(long ns)
    {
        typedef NTSTATUS (NTAPI * NtDelayExecutionFN)(BOOL, PLARGE_INTEGER);

        static bool               loading = true;
        static NtDelayExecutionFN NtDelayExcution;
        if (loading && !NtDelayExcution)
        {
            loading = false;
            NtDelayExcution = (NtDelayExecutionFN)GetProcAddress(GetModuleHandle(TEXT("ntdll.dll")), "NtDelayExecution");
        }

        if (NtDelayExcution)
        {
            LARGE_INTEGER times;
            times.QuadPart = -ns / 100;
            return NtDelayExcution(FALSE, &times) == 0;
        }
        else
        {
            Sleep(ns / (1000 * 1000));
            return true;
        }
    }

    bool MicroSleep(long us)
    {
        return NanoSleep(us * 1000);
    }

    long GetCounter(void)
    {
        LARGE_INTEGER result;
        return QueryPerformanceCounter(&result) ? (long)result.QuadPart : (long)GetTickCount();
    }

    long GetFrequency(void)
    {
        LARGE_INTEGER result;
        return QueryPerformanceFrequency(&result) ? (long)result.QuadPart : 1000;
    }
}

#endif

/* END OF TIMER_IMPL */
#endif /* TIMER_IMPL */