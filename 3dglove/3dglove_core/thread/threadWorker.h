
/*
 * $Copyright$
 * Copyright (c) 2015 All Rights Reserved, Sean Harre
 *
 * Sean Harre is the copyright holder of all code below.
 * Do not re-use without permission.
 */

#ifndef _THREADED_OBJECT
#define _THREADED_OBJECT

#include "3dtarget.h"

#include "leapDriver.h"     // leap
#include "ftdiDriver.h"     // ftdi
#include "bulletDriver.h"   // bullet
#include "soundCtrl.h"      // sound
#include "networker.h"      // networker

class threadWorker;
threadWorker& getTW();

class threadWorker : public ofThread, public Leap::Listener {

	public:
    
        friend threadWorker& getTW();
    private:
        threadWorker();
    public:
    
        bool start();
        void stop();
        void restart_physics();
        bool done_restarting();
    
        // callback from Leap Listener thread
        void onFrame(const Leap::Controller& controller);
    
        // debug
        bool set_debug_mode(bool newv);
        void debug_hands();
        void debug_step_thru_hand_joints();
        void debug_light_right_hand();
        void debug_light_left_hand();
        void debug_set_bullet_update_hz(btScalar newVal);
    
        // profiling
        float leap_frame_rate;
        prof twprof;    // thread
        prof btProf;    // bullet
        prof ftProf;    // ftdi
        prof sndProf;   // sound
        prof leapTProf; // leap thread
        prof btCbProf;  // bullet callback
        prof nwprof;    // networker
    
        // public api to enable/disable leap filtering
        void set_leap_filtering(bool set_on) { getLeapDrv().set_hand_filtering(set_on); }
        bool get_leap_filtering() { return getLeapDrv().get_hand_filtering(); }
    
    private:
    
        // bullet tick callback
        static void bulletTickCallback(void* cb_this, btDynamicsWorld *world, btScalar timeStep);
        btScalar update_rate;
    
        Leap::Controller LeapController;
            
        // called by ofThread
        void threadedFunction();
    
        SoundCtrl snd;          // sound
        netWorker nworker;          // networker
    
        #if CFG_HAS_RIGHT_HAND
        ftdiDriver right_hand;  // ftdi
        #endif
        #if CFG_HAS_LEFT_HAND
        ftdiDriver left_hand;   // ftdi
        #endif
    
        bool got_update;
        bool request_restart;
    
        // debug
        bool debug_mode;
        bool running_test_rh;
        bool running_test_lh;
        bool dbg_connections;
        int dbg_finger;
        int dbg_joint;
};

#endif  // _THREADED_OBJECT

