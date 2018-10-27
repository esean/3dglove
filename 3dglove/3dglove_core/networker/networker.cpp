
/*
 * $Copyright$
 * Copyright (c) 2015 All Rights Reserved, Sean Harre
 *
 * Sean Harre is the copyright holder of all code below.
 * Do not re-use without permission.
 */

#ifndef _NETWORKER_OBJECT_IMPL
#define _NETWORKER_OBJECT_IMPL

#include "networker.h"

#if CFG_NETWORKING

//--------------------------
netWorker::netWorker() :
    debug_mode(false), got_update(false),
    request_restart(false)
{
}

//--------------------------
bool netWorker::start() {
    
    nwprof.start();
    
    startThread(true);   // blocking
    
    // profiling
    printf("NW:start time = %f us\n",nwprof.get_current_timer_us());
    
    return true;
}

//--------------------------
void netWorker::stop() {
    
    dbprintf("NW thread stopping\n");
    
    // stop thread
    waitForThread(true);
}

//--------------------------
bool netWorker::set_debug_mode(bool newv)
{
    bool was = debug_mode;
    debug_mode = newv;
    return was;
}

//--------------------------
void netWorker::restart()
{
    request_restart = true;
}

//--------------------------
void netWorker::tx_current_hand_location_data(const HandVector::hand_vecs& lhand, const HandVector::hand_vecs& rhand)
{
    got_update = true;
    // todo - copy hand locations for threadfcn to send out
}



/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
////
//// ::threadedFunction() is OF thread
////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

//--------------------------
void netWorker::threadedFunction() {
    
    while( isThreadRunning() != 0 ) {

        if (lock())
        {
            if (got_update)
            {
                //printf("NW:%f\n",nwprof.get_current_timer_us());
                
                // start the profiling timer
                nwprof.fcn_start();
                
                //
                // This is thread doing main lifting behind scenes.
                // It allows leap, physics and ftdi to go as fast as possible.
                //
                unlock();
                
                /////
                /////
                /////
                /////
                ///// DO ALL WORK HERE
                //      send anything out
                //      listen for anything
                printf("nw!");
                /////
                /////
                /////
                /////
                /////
                
                // all done, make sure to do after hand_data has been used
                got_update = false;
                
                // stop the profiling timer and accumulate results
                (void)nwprof.fcn_end();
                
                //printf("<-NW>\n");
                
            }
            else
            {
                unlock();
            }
        }
    }
    dbprintf("NW THREAD BYE!\n");
}










#else 

netWorker::netWorker() :
debug_mode(false), got_update(false),
request_restart(false)
{
}

//--------------------------
bool netWorker::start() {
}

//--------------------------
void netWorker::stop() {
}

//--------------------------
bool netWorker::set_debug_mode(bool newv)
{
}

//--------------------------
void netWorker::restart()
{
}

//--------------------------
void netWorker::threadedFunction()
{
}

void netWorker::tx_current_hand_location_data(const HandVector::hand_vecs& lhand, const HandVector::hand_vecs& rhand)
{
}

#endif  // CFG_NETWORKING



#endif
