
/*
 * $Copyright$
 * Copyright (c) 2015 All Rights Reserved, Sean Harre
 *
 * Sean Harre is the copyright holder of all code below.
 * Do not re-use without permission.
 */

#ifndef _NETWORKER_OBJECT
#define _NETWORKER_OBJECT

#include "3dtarget.h"

class netWorker : public ofThread {

	public:
    
        netWorker();
    
        bool start();
        void stop();
        void restart();
    
        // debug
        bool set_debug_mode(bool);
    
        // profiling
        prof nwprof;
    
        void tx_current_hand_location_data(const HandVector::hand_vecs& lhand, const HandVector::hand_vecs& rhand);

private:
    
        // called by ofThread
        void threadedFunction();
    
        bool got_update;
        bool request_restart;
    
        // debug
        bool debug_mode;
};

#endif

