#ifndef __TIMER_H__
#define __TIMER_H__

#ifndef __timer__
#define __timer__
#endif

#ifdef __cplusplus
extern "C" {
#endif
/* BEGIN OF EXTERN "C" */


typedef struct
{
    enum
    {
	    TIMER_STARTED,
	    TIMER_STOPPED,
    }    state;
    long start;
    long ticks;
} timer_t;      

__timer__ void   timer_init(timer_t* timer);
__timer__ void   timer_start(timer_t* timer);
__timer__ void   timer_stop(timer_t* timer);
__timer__ double timer_seconds(timer_t* timer);

/**
 * Sleep the current thread with given duration in us
 */
__timer__ int  perf_usleep(long us);
 
/**
 * Sleep the current thread with given duration in us
 */
__timer__ int  perf_nsleep(long ns);
    
/**
 * Query counter of the CPU performance
 */
__timer__ long perf_counter(void);

/**
 * Query frequency of the CPU performance
 */
__timer__ long perf_frequency(void);

#ifdef TIMER_IMPL
/* BEGIN OF TIMER_IMPL */

void timer_init(timer_t* timer)
{
    timer->state = TIMER_STOPPED;
    timer->start = 0;
    timer->ticks = 0;
}
    
void timer_start(timer_t* timer)
{
    timer->state = TIMER_START;
    timer->start = perf_counter();
}
    
void timer_stop(timer_t* timer)
{
    timer->state = TIMER_STOPPED;
    timer->ticks = perf_counter() - timer->start;
}

double timer_seconds(timer_t* timer)
{
    return timer->ticks / perf_frequency(); 
}
    
# ifdef WINDOWS
/* BEGIN OF WINDOWS */
    
# include <Windows.h>
int perf_usleep(long us)
{
    return perf_nsleep(us * 1000);
}
    
int perf_nsleep(long ns)
{
    /* Actually return NTSTATUS, and 'typedef LONG NTSTATUS;' =)) */
    /* '#define NTAPI __stdcall' =)) */
    typedef LONG (__stdcall * NtDelayExecutionFN)(BOOL, PLARGE_INTEGER);
    static NtDelayExecutionFN NtDelayExecution;
    
    if (!NtDelayExecution)
    {
        HMODULE module   = GetModuleHandle(TEXT("ntdll.dll"));
        const char* func = "NtDelayExecution";
        NtDelayExecution = (NtDelayExecutionFN)GetProcAddress(module, func);
        if (!NtDelayExecution)
        {
            return FALSE;
        }
    }
    
    LARGE_INTEGER times;
    times.QuadPart = -ns / 100;
    NtDelayExecution(FALSE, &times);
    return TRUE;
}

long perf_counter(void)
{
    LARGE_INTEGER result;
    return QueryPerformanceCounter(&result) ? (long)result.QuadPart : 0;
}

long perf_frequency(void)
{
    LARGE_INTEGER result;
    return QueryPerformanceFrequency(&result) ? (long)result.QuadPart : 0;
}
/* END OF WINDOWS */
# elif __unix__
/* BEGIN OF UNIX */
# include <sys/time.h>
# define CLOCK_ID 981999
/* Simple utils for checking the system has monotonic
 */
__attribute__((always_inline))
static int has_monotonic(void)
{
#if HAVE_CLOCK_GETTIME
    return clock_gettime(CLOCK_ID, NULL) == 0; 
#elif defined(__APPLE__)
    mach_timebase_info_data_t mach_info;
    kern_return_t ret = mach_time_base_info(&mach_info);
    return ret == 0;
#else
    return 0;
#endif
}

int perf_usleep(long us)
{
    return usleep(us) == 0;
}

int perf_nsleep(long ns)
{
    return nanosleep((struct timespec[]){{ 0, ns }}, NULL) == 0;
}
    
long perf_counter(void)
{
    long ticks;

    if (has_monotonic())
    {
#if HAVE_CLOCK_GETTIME
        struct timespec now;
        clock_gettime(CLOCK_ID, &now);
        
        /* Get counter in us */
        ticks  = now.tv_sec;
        ticks *= 1000 * 1000; /* To us */
        ticks += now.tv_usec;
        ticks += now.tv_msec * 1000;
#elif defined(__APPLE__)
	    ticks = mach_absolute_time();
#endif
    }
    else
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        
        /* Get counter in us */
        ticks  = now.tv_sec;
        ticks *= 1000 * 1000; /* To us */
        ticks += now.tv_usec;
        ticks += now.tv_msec * 1000;
    }
    return ticks;
}

long perf_frequency(void)
{
#if HAVE_CLOCK_GETTIME
    if (has_monotonic())
    {
	    return 1000 * 1000 * 1000; /* ns per second */
    }
#elif defined(__APPLE__)
    mach_timebase_info_data_t mach_info;
    kern_return_t ret = mach_time_base_info(&mach_info);
    if (ret == 0)
    {
        long frequency;
        frequency  = mach_info.denom;
        frequency *= 1000 * 1000 * 1000;
        frequency /= mach_info.numer;
        return frequency;
    }
#endif
    
    return 1000 * 1000; /* us per second */
}
    
/* END OF UNIX */    
# endif

/* END OF TIMER_IMPL */
#endif

/* END OF EXTERN "C" */
#ifdef __cpluscplus
}
#endif
    
#endif /* __TIMER_H__ */
