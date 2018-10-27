
/*
 * $Copyright$
 * Copyright (c) 2015 All Rights Reserved, Sean Harre
 *
 * Sean Harre is the copyright holder of all code below.
 * Do not re-use without permission.
 */

#ifndef PROF_H
#define PROF_H

#include "3dtarget.h"
#include "LinearMath/btQuickprof.h" // profiling

class prof : public btClock {
    
public:
    prof();
    
    // Main API:
    // call these when enter/exiting function you want to profile
    void fcn_start();
    void fcn_end();
    
    // reset statistics
    void reset();
    
    // getters
    struct prof_stat {
        // these are the amount of time spent processing function
        float avg;
        float max;
        float recent;
        
        // these are amount of time spent between calls to function
        float update_avg;
        float update_recent;
    };
    prof_stat get_stats();
    // TODO: void clear_prof_stat();
    
    
    // Only call these is you know what you're doing:
    // starts timer
    void start();
    
    // stops timer and adds to stats, restarts timer
    // returns amount of delta time stince last re/started
    float stop();
    
    // most recent microsec count
    float get_current_timer_us();
    
private:
    btClock tmr;
    float this_time_us;
    float max_time_us;
    float time_avg;
    
    btClock update;
    float away_time_avg;
    float away_time_recent;
    
    float flt_avg(const float newv, const float oldv, const float coeff);
    
};

#endif
