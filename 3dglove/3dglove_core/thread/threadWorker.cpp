
/*
 * $Copyright$
 * Copyright (c) 2015 All Rights Reserved, Sean Harre
 *
 * Sean Harre is the copyright holder of all code below.
 * Do not re-use without permission.
 */

#ifndef _THREADED_OBJECT_IMPL
#define _THREADED_OBJECT_IMPL

// this file runs thread

#include "threadWorker.h"

// standalone function to instantiate a singleton threadWorker
threadWorker& getTW()
{
    static threadWorker* twPtr = 0;
    if (!twPtr) {
        cout<< "create instance of threadworker" <<endl;
        twPtr = new threadWorker;
    }
    return *twPtr;
}

//--------------------------
threadWorker::threadWorker() :
    debug_mode(false), got_update(false),update_rate(STEP_WORLD_UPDATE_HZ),
    running_test_lh(false),running_test_rh(false),request_restart(false),
    dbg_finger(0),dbg_joint(0),dbg_connections(false)
{
}

//--------------------------
bool threadWorker::start() {
    
    twprof.start();
    
    // networker thread
    nworker.start();
    
    // sound
    snd.start("sounds/punch_or_slap_face.mp3");
    
    // Need to be careful about init so doesn't blow up
    // We need to get hand data first, then update bullet
    // so that when we add the leap listener the bullet
    // is ready for that first callback.
    // start leap
    if ( ! getLeapDrv().start(LeapController) )
    {
        fprintf(stderr,"FAILED to init Leap!\n");
        assert(0);
    }
    
    // start bullet and create our hand kinematic objects in the physics world
    getBullet().start();
    getBullet().add_hand_kinematic_objects_to_bullet(getLeapDrv().lhand,getLeapDrv().rhand);
    
    // set tick callback fcn ptr
    getBullet().set_cb(&threadWorker::bulletTickCallback,this);
    // now enable leap listener callbacks
    LeapController.addListener(*this);
    
    // thread
    #if CFG_HAS_RIGHT_HAND
    if (! right_hand.start(FTDI_RH_SN) )
    {
        fprintf(stderr,"FAILED to init FTDI right hand\n");
        assert(0);
    }
    dbprintf("Found right hand\n");
    #endif
    #if CFG_HAS_LEFT_HAND
    if (! left_hand.start(FTDI_LH_SN) )
    {
        fprintf(stderr,"FAILED to init FTDI left hand\n");
        assert(0);
    }
    dbprintf("Found left hand\n");
    #endif
    
    startThread(true);   // blocking
    
    // profiling
    dbprintf("TW:start time = %f us\n",twprof.get_current_timer_us());
    
    return true;
}

//--------------------------
void threadWorker::stop() {
    
    dbprintf("thread stopping\n");

    LeapController.removeListener(*this);
    nworker.stop();
    
    // stop thread
    waitForThread(true);

    // stop the thread
#if CFG_HAS_RIGHT_HAND
    right_hand.stop();
#endif
#if CFG_HAS_LEFT_HAND
    left_hand.stop();
#endif
}

//--------------------------
bool threadWorker::set_debug_mode(bool newv)
{
    bool was = debug_mode;
    debug_mode = newv;
    
    getLeapDrv().set_debug_mode(debug_mode);
    snd.set_debug_mode(debug_mode);
    
    dbg_connections = false;
    dbg_finger = dbg_joint = 0;
    
    (void)nworker.set_debug_mode(newv);
    
    return was;
}

//--------------------------
void threadWorker::debug_set_bullet_update_hz(btScalar newVal)
{
    update_rate = newVal;
}

//--------------------------
void threadWorker::restart_physics()
{
    request_restart = true;
    nworker.restart();
}

//--------------------------
bool threadWorker::done_restarting()
{
    return ( ! request_restart );
}


//--------------------------
void threadWorker::debug_hands()
{
    #if CFG_HAS_RIGHT_HAND
    if (running_test_rh)
        for (DWORD idx=0; idx<GS_CHANNELS; idx++)
            right_hand.update(idx,GS_MAX);
    #endif
    #if CFG_HAS_LEFT_HAND
    if (running_test_lh)
        for (DWORD idx=0; idx<GS_CHANNELS; idx++)
            left_hand.update(idx,GS_MAX);
    #endif
}

//--------------------------
void threadWorker::debug_step_thru_hand_joints()
{
    if ( ! dbg_connections)
    {
        dbg_finger = dbg_joint = 0;
        for (int f=0; f<MAX_FINGERS; ++f)
        {
            for (int j=0; j<MAX_JOINTS; ++j)
            {
                getLeapDrv().rhand.fingers[f].joints[j].collision_force = 0;
                getLeapDrv().lhand.fingers[f].joints[j].collision_force = 0;
            }
        }
    }
    else
    {
        // clear previous before advancing
        getLeapDrv().rhand.fingers[dbg_finger].joints[dbg_joint].collision_force = 0;
        getLeapDrv().lhand.fingers[dbg_finger].joints[dbg_joint].collision_force = 0;
        
        // next finger/joint
        ++dbg_joint;
        if (dbg_joint >= MAX_JOINTS)
        {
            dbg_joint = 0;
            ++dbg_finger;
            if (dbg_finger >= MAX_FINGERS)
                dbg_finger = 0;
        }
    }
    
    dbprintf("# ++++ DebugConnections finger:%d joint:%d R:%d L:%d\n",dbg_finger,dbg_joint,
             getLeapDrv().rhand.fingers[dbg_finger].joints[dbg_joint].ftdi_idx,
             getLeapDrv().lhand.fingers[dbg_finger].joints[dbg_joint].ftdi_idx);
    dbg_connections = true;
}

//--------------------------
void threadWorker::debug_light_right_hand()
{
    #if CFG_HAS_RIGHT_HAND
    running_test_rh = !running_test_rh;
    right_hand.set_red_LED(running_test_rh);
    #endif
}

//--------------------------
void threadWorker::debug_light_left_hand()
{
    #if CFG_HAS_LEFT_HAND
    running_test_lh = !running_test_lh;
    left_hand.set_red_LED(running_test_lh);
    #endif
}


/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
////
//// ::getBullet().ickCallback() is called for physics tick substeps
////    called multiple times from inside running step_world()
////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

//--------------------------
void threadWorker::bulletTickCallback(void* cb_this, btDynamicsWorld *world, btScalar timeStep)
{
    // get pointer to our object that requested the callbacks
    threadWorker* tmp = (threadWorker*)cb_this;
    
    tmp->btCbProf.fcn_start();
    //printf("BCB:%f timeStep=%f\n",tmp->btCbProf.get_current_timer_us(),timeStep);
    
    if (tmp->lock())
    {
        #if 0
            if (tmp->got_update)
                dbprintf("PREV_SAMPLE!\n");
        #endif
        
        // notify: data should now be processed in our thread
        tmp->got_update = true;
    
        tmp->unlock();
    }
    
    tmp->btCbProf.fcn_end();
}


/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
////
//// ::onFrame() is called from Leap thread
////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

//--------------------------
void threadWorker::onFrame(const Leap::Controller& controller) {
    
    // Process this frame's data...
    
    //printf("L:%f\n",leapTProf.get_current_timer_us());
    //printf("[+L]");
    
    // start the profiling timer
    leapTProf.fcn_start();

    // profiling
    leap_frame_rate = controller.frame().currentFramesPerSecond();
    
    // here we are being called from Leap thread, pass
    // controller to these to get data
    (void)getLeapDrv().get_left_hand_filtered(controller);
    (void)getLeapDrv().get_right_hand_filtered(controller);
    
    //////////////////////////////////////////////////////////////////////
    //
    // -- NOTE: We are running in Leap thread right now, so we do physics
    //      work with new hand data. As physics computes subticks and we
    //      get above callbacks, the thread is told there's something to do.
    //
    //////////////////////////////////////////////////////////////////////

    // push out our current hand dataset
    nworker.tx_current_hand_location_data(getLeapDrv().lhand,getLeapDrv().rhand);

    // push to getBullet(). step, see collisions, update sound and glove, clear data
    {
        btProf.fcn_start();
        
        // push all kinematic joint vectors to bullet
        getBullet().update_kinematics(getLeapDrv().lhand,getLeapDrv().rhand);
        
        // step physics forward
        // when we exec step_world, it will callback getBullet().ickCallback for each processed world tick
        getBullet().step_world(update_rate);

        btProf.fcn_end();
    }
    
    if (request_restart)
    {
        if (lock())
        {
            getBullet().stop();
            getBullet().start();
            getBullet().add_hand_kinematic_objects_to_bullet(getLeapDrv().lhand,getLeapDrv().rhand);
            // set tick callback fcn ptr
            getBullet().set_cb(&threadWorker::bulletTickCallback,this);
            
            // done restart, now we can signal we are done
            request_restart = false;
            
            unlock();
        }
    }
    
    leapTProf.fcn_end();
    //printf("[-L]\n");
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
////
//// ::threadedFunction() is OF thread
////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

//--------------------------
void threadWorker::threadedFunction() {
    
    while( isThreadRunning() != 0 ) {

        if (lock())
        {
            if (got_update)
            {
                //printf("<+T>");
                //printf("T:%f\n",twprof.get_current_timer_us());
                
                // start the profiling timer
                twprof.fcn_start();
                
                //
                // This is thread doing main lifting behind scenes.
                // It allows leap, physics and ftdi to go as fast as possible.
                //
                
                // pass hand_vecs to getBullet(). it fills in impulse and distance for any collisions
                bool saw_collisions = getBullet().update_impulses(getLeapDrv().lhand,getLeapDrv().rhand);
            
                unlock();
                
                // is any collisions, make sounds proportional
                sndProf.fcn_start();
                if (saw_collisions)
                    snd.make_collision_sounds(getLeapDrv().lhand, getLeapDrv().rhand);
                sndProf.fcn_end();
                
                //  update data to "draw" to ftdi
                {
                    ftProf.fcn_start();
                    
                    // dev
                    if (dbg_connections)
                    {
                        getLeapDrv().rhand.fingers[dbg_finger].joints[dbg_joint].collision_force = GS_MAX;
                        getLeapDrv().lhand.fingers[dbg_finger].joints[dbg_joint].collision_force = GS_MAX;
                        
                        #if CFG_HAS_RIGHT_HAND
                        right_hand.update(getLeapDrv().rhand.fingers[dbg_finger].joints[dbg_joint].ftdi_idx,getLeapDrv().rhand.fingers[dbg_finger].joints[dbg_joint].collision_force);
                        #endif
                        #if CFG_HAS_LEFT_HAND
                        left_hand.update(getLeapDrv().lhand.fingers[dbg_finger].joints[dbg_joint].ftdi_idx,getLeapDrv().lhand.fingers[dbg_finger].joints[dbg_joint].collision_force);
                        #endif
                    }
                    
                    // update TLC w/ new impulse numbers
                    #if CFG_HAS_RIGHT_HAND
                    right_hand.make_collision_updates(getLeapDrv().rhand);
                    right_hand.draw();
                    #endif
                    #if CFG_HAS_LEFT_HAND
                    left_hand.make_collision_updates(getLeapDrv().lhand);
                    left_hand.draw();
                    #endif
                    
                    ftProf.fcn_end();
                }
                
                // done with impulse info, clear now
                if ( ! dbg_connections)
                {
                    getLeapDrv().clear_impulse_data(getLeapDrv().rhand);
                    getLeapDrv().clear_impulse_data(getLeapDrv().lhand);
                }
                
                // all done
                got_update = false;
                
                // stop the profiling timer and accumulate results
                (void)twprof.fcn_end();
                
                //printf("<-T>\n");
                
            }
            else
            {
                unlock();
            }
        }
    }
    dbprintf("THREAD BYE!\n");
}

#endif
