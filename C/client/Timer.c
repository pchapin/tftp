/*! \file    Timer.c
    \brief   Implementation of a simple timer abstract type.
    \author  Peter C. Chapin <PChapin@vtc.vsc.edu>

*/

#include <stddef.h>
#include "Timer.h"

//
// The following function looks up the system time and returns it in the timer_t type. Note
// that timer_t is intended to be an integer but might be a structure type on some systems.
//
static void get_time_as_integer( timer_time_t *result )
{
    #if eOPSYS == eDOS
    _dos_gettime( result );
    #endif

    #if eOPSYS == eWIN32
    SYSTEMTIME raw;
    FILETIME   cooked;

    GetSystemTime( &raw );
    SystemTimeToFileTime( &raw, &cooked );
    result->u.LowPart  = cooked.dwLowDateTime;
    result->u.HighPart = cooked.dwHighDateTime;
    #endif

    #if eOPSYS == ePOSIX
    struct timeval time_info;
    gettimeofday( &time_info, NULL );
    result->seconds      = time_info.tv_sec;
    result->milliseconds = time_info.tv_usec / 1000;
    #endif
}


//
// The following function takes the system dependent timer_time_t and returns the number of
// milliseconds it represents as a long integer.
//
static long get_adjusted_time( const timer_time_t the_time )
{
    #if eOPSYS == eDOS
    long temp = the_time.hsecond * 10L;
    temp += the_time.second * 1000L;
    temp += the_time.minute * 60000L;
    temp += the_time.hour * ( 60 * 60000L );
    return temp;
    #endif

    #if eOPSYS == eWIN32
    long long temp = the_time.QuadPart;
    temp /= 10000;
    return (long)temp;
    #endif

    #if eOPSYS == ePOSIX
    long temp = the_time.seconds;
    temp *= 1000;
    temp += the_time.milliseconds;
    return temp;
    #endif
}


//
// The following two operator functions implement the usual semantics for the timer_time_t type. On
// some systems these functions are trivial.
//
static timer_time_t add( const timer_time_t left, const timer_time_t right )
{
    timer_time_t result;

    #if eOPSYS == eDOS
    result.hour    = (char)( left.hour    + right.hour );
    result.minute  = (char)( left.minute  + right.minute );
    result.second  = (char)( left.second  + right.second );
    result.hsecond = (char)( left.hsecond + right.hsecond );
    if( result.hsecond > 100 ) {
        ++result.second;
        result.hsecond -= 100;
    }
    if( result.second > 60 ) {
        ++result.minute;
        result.second -= 60;
    }
    if( result.minute > 60 ) {
        ++result.hour;
        result.minute -= 60;
    }
    #endif

    #if eOPSYS == eWIN32
    result.QuadPart = left.QuadPart + right.QuadPart;
    #endif

    #if eOPSYS == ePOSIX
    result.seconds      = left.seconds + right.seconds;
    result.milliseconds = left.milliseconds + right.milliseconds;
    if( result.milliseconds > 1000 ) {
        ++result.seconds;
        result.milliseconds -= 1000;
    }
    #endif

    return result;
}


static timer_time_t subtract( const timer_time_t left, const timer_time_t right )
{
    timer_time_t result;

    #if eOPSYS == eDOS
    long difference = ( get_adjusted_time( left ) - get_adjusted_time( right ) ) / 10;
    result.hsecond = (unsigned char)( difference % 100 );
    difference /= 100;
    result.second = (unsigned char)( difference % 60 );
    difference /= 60;
    result.minute = (unsigned char)( difference % 60 );
    difference /= 60;
    result.hour = (unsigned char)( difference );
    #endif

    #if eOPSYS == eWIN32
    result.QuadPart = left.QuadPart - right.QuadPart;
    #endif

    #if eOPSYS == ePOSIX
    result.seconds      = left.seconds - right.seconds;
    result.milliseconds = left.milliseconds - right.milliseconds;
    if( result.seconds > 0 && result.milliseconds < 0 ) {
        --result.seconds;
        result.milliseconds += 1000;
    }
    if( result.seconds < 0 && result.milliseconds > 0 ) {
        ++result.seconds;
        result.milliseconds -= 1000;
    }
    #endif

    return result;
}


void Timer_initialize( Timer *object )
{
    object->internal_state = RESET;

    #if eOPSYS == eDOS
    object->accumulated.hour    = 0;
    object->accumulated.minute  = 0;
    object->accumulated.second  = 0;
    object->accumulated.hsecond = 0;
    #endif

    #if eOPSYS == eWIN32
    object->accumulated.u.LowPart  = 0;
    object->accumulated.u.HighPart = 0;
    #endif

    #if eOPSYS == ePOSIX
    object->accumulated.seconds = 0;
    object->accumulated.milliseconds = 0;
    #endif
}


void Timer_reset( Timer *object )
{
    object->internal_state = RESET;

    #if eOPSYS == eDOS
    object->accumulated.hour    = 0;
    object->accumulated.minute  = 0;
    object->accumulated.second  = 0;
    object->accumulated.hsecond = 0;
    #endif

    #if eOPSYS == eWIN32
    object->accumulated.u.LowPart = 0;
    object->accumulated.u.HighPart = 0;
    #endif

    #if eOPSYS == ePOSIX
    object->accumulated.seconds = 0;
    object->accumulated.milliseconds = 0;
    #endif
}


void Timer_start( Timer *object )
{
    object->internal_state = RUNNING;
    get_time_as_integer( &object->start_time );
    return;
}


void Timer_stop( Timer *object )
{
    timer_time_t stop_time;

    get_time_as_integer( &stop_time );
    object->internal_state = STOPPED;
    object->accumulated =
        add( object->accumulated, subtract( stop_time, object->start_time ) );
    return;
}


long Timer_time( Timer *object )
{
    timer_time_t total_time;
    timer_time_t current_time;

    if( object->internal_state != RUNNING ) {
        total_time = object->accumulated;
    }
    else {
        get_time_as_integer( &current_time );
        total_time = add( object->accumulated, subtract( current_time, object->start_time ) );
    }
    return get_adjusted_time( total_time );
}
