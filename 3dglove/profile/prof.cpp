
/*
 * $Copyright$
 * Copyright (c) 2015 All Rights Reserved, Sean Harre
 *
 * Sean Harre is the copyright holder of all code below.
 * Do not re-use without permission.
 */

#include "prof.h"

prof::prof() :
    this_time_us(0.0f),max_time_us(0.0f),time_avg(0.0f),
    away_time_avg(0.0f),away_time_recent(0.0f)
{
    tmr.reset();
    update.reset();
}

#if !DISABLE_PROFILING

//--------------------------
void prof::fcn_start()
{
    // handle update times
    away_time_recent = update.getTimeMicroseconds();
    update.reset();
    away_time_avg = flt_avg(away_time_recent, away_time_avg, 0.99f);
    
    // handle function processing time
    start();
}

//--------------------------
void prof::fcn_end()
{
    // handle function processing time
    (void)stop();
}

//--------------------------
void prof::start()
{
    // reset timer to 0
    tmr.reset();
}

//--------------------------
float prof::stop()
{
    this_time_us = get_current_timer_us();
    tmr.reset();
    
    // update avg
    time_avg = flt_avg(this_time_us,time_avg,0.99f);

    // update max
    if (this_time_us > max_time_us)
        max_time_us = this_time_us;
    
    // return delta time that just passed from last re/start
    return this_time_us;
}

//--------------------------
void prof::reset()
{
    this_time_us = 0.0f;
    max_time_us = 0.0f;
    time_avg = 0.0f;
    away_time_recent = 0.0f;
    away_time_avg = 0.0f;
}

//--------------------------
float prof::get_current_timer_us()
{
    return tmr.getTimeMicroseconds();
}

//--------------------------
prof::prof_stat prof::get_stats()
{
    prof_stat ret;
    ret.avg = time_avg;
    ret.recent = this_time_us;
    ret.max = max_time_us;
    ret.update_avg = away_time_avg;
    ret.update_recent = away_time_recent;
    return ret;
}

//--------------------------
// filter:
//  y = x0*m + x1*(1-m)
//
float prof::flt_avg(const float newv, const float oldv, const float coeff)
{
    return (oldv * coeff) + (newv * (1.0f-coeff));
}

#else

void prof::fcn_start() {}
void prof::fcn_end() {}
void prof::start() {}
float prof::stop() { return 0.0f; }
void prof::reset() {}
float prof::get_current_timer_us() { return 0.0f; }
prof::prof_stat prof::get_stats() { return static_cast<prof::prof_stat>(nil); } // TODO:

#endif
