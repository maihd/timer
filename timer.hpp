#ifndef __TIMER_HPP__
#define __TIMER_HPP__

#ifndef __cplusplus
#error "timer.hpp require C++"
#endif

#ifndef __timer__
#define __timer__
#endif

#ifndef TIMER__CONTAINEROF
#define TIMER__CONTAINEROF(ptr, type_t, member) (type_t*)((char*)ptr - ((long)&((type_t*)0)->member))
#endif

#include <assert.h>

namespace perf
{
    __timer__ bool usleep(long us);
    __timer__ bool nsleep(long ns);

    __timer__ long counter(void);
    __timer__ long frequency(void);
}

struct __timer__ timer
{
public: /* @region: Static functions */
    __timer__ static void start(timer& timer);
    __timer__ static void stop(timer& timer);
    __timer__ static bool sleep(timer& timer);

public: /* @region: Constructors */
    inline timer(int fps = 60)
        : _state(TIMER_STOPPED)
        , _start(-1)
        , _ticks(-1)
    {
        this->limitfps = fps;
    }

public: /* @region: Properties */
    union
    {
    public:
        inline operator double(void) const
        {
            const timer* timer = TIMER__CONTAINEROF(this, const ::timer, fps);
            if (timer->_state == TIMER_STOPPED)
            {
                return 1.0 / timer->seconds;
            }
            else
            {
                return 0.0;
            }
        }
    } fps;

    union
    {
    public:
        inline operator double(void) const
        {
            const timer* timer = TIMER__CONTAINEROF(this, const ::timer, seconds);
            return (double)timer->_ticks / perf::frequency();
        }
    } seconds;

    union
    {
    public:
        inline int operator=(int fps)
        {
            assert(fps > 0 && fps < perf::frequency());
    
            timer* timer = TIMER__CONTAINEROF(this, ::timer, limitfps);
            if (timer->_state == TIMER_STOPPED)
            {
                timer->_limit = perf::frequency() / fps;
            }

			return fps;
        }

        inline operator int(void) const
        {
            const timer* timer = TIMER__CONTAINEROF(this, const ::timer, limitfps);
            return (int)(perf::frequency() / timer->_limit);
        }
    } limitfps;

private: /* @region: Fields */
    enum
    {
        TIMER_STARTED,
        TIMER_STOPPED,
    }    _state;
    long _start;
    long _ticks;
    long _limit;
};

#endif /* __TIMER_HPP__ */

#ifdef TIMER_CPP_IMPL
/* BEGIN OF TIMER_CPP_IMPL */

void timer::start(timer& timer)
{
    timer._state = TIMER_STARTED;
    timer._start = perf::counter();
}

void timer::stop(timer& timer)
{
    timer._state = TIMER_STOPPED;
    timer._ticks = perf::counter() - timer._start;
}

bool timer::sleep(timer& timer)
{
    if (timer._state == TIMER_STOPPED)
    {
        if (timer._ticks < timer._limit)
        {
            double seconds = (double)(timer._limit - timer._ticks) / perf::frequency();
            perf::usleep((long)(seconds * 1000 * 1000));
            timer._ticks = timer._limit;
            return true;
        }
    }

    return false;
}

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#include <Windows.h>

namespace perf 
{
    bool usleep(long us)
    {
        return perf::nsleep(us * 1000);
    }
        
    bool nsleep(long ns)
    {
        /* 'NTSTATUS NTAPI NtDelayExecution(BOOL Alerted, PLARGE_INTEGER time);' */
        /* 'typedef LONG NTSTATUS;' =)) */
        /* '#define NTAPI __stdcall' =)) */
        typedef LONG (__stdcall * NtDelayExecutionFN)(BOOL, PLARGE_INTEGER);

        static int done_finding;
        static NtDelayExecutionFN NtDelayExecution;
        
        if (!NtDelayExecution && !done_finding)
        {
            done_finding     = 1;
            HMODULE module   = GetModuleHandle(TEXT("ntdll.dll"));
            const char* func = "NtDelayExecution";
            NtDelayExecution = (NtDelayExecutionFN)GetProcAddress(module, func);
        }
        
        if (NtDelayExecution)
        {
            LARGE_INTEGER times;
            times.QuadPart = -ns / 100;
            NtDelayExecution(FALSE, &times);
            return true;
        }
        else
        {
            Sleep(ns / (1000 * 1000));
            return true;
        }
    }

    long counter(void)
    {
        LARGE_INTEGER result;
        return QueryPerformanceCounter(&result) ? (long)result.QuadPart : GetTickCount();
    }

    long frequency(void)
    {
        LARGE_INTEGER result;
        return QueryPerformanceFrequency(&result) ? (long)result.QuadPart : 1000;
    }
}
#endif

/* END OF TIMER_CPP_IMPL */
#endif