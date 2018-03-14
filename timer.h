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
    
#ifdef WINDOWS
/**
 * Sleep the current thread with given duration in us
 */
__timer__ int  usleep(long us);
 
/**
 * Sleep the current thread with given duration in us
 */
__timer__ int  nanosleep(long ns);
#else
# include <unistd.h> /* for usleep and nanosleep */
#endif

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
int usleep(long us)
{
    return nanosleep(us * 1000);
}
    
int nanosleep(long ns)
{
    HANDLE        timer;
    LARGE_INTEGER times;

    /* Create timer for counting
     */
    timer = CreateWaitableTimer(NULL, TRUE, NULL);
    if (!timer) return FALSE;

    /* Settings timer with due time per 100ns units
     */
    times.QuadPart = -ns / 100;
    if (!SetWaitableTimer(timer, &times, 0, NULL, NULL, FALSE))
    {
	CloseHandle(timer);
	return FALSE;
    }

    /* Wait until the timer end
     */
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);

    /* Progress done
     */
    return TRUE;
}

long perf_counter(void)
{
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return result.QuadPart;
}

long perf_frequency(void)
{
    LARGE_INTEGER result;
    QueryPerformanceFrequency(&result);
    return result.QuadPart;
}
/* END OF WINDOWS */
# else
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
