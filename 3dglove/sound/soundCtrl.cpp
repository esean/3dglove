
/*
 * $Copyright$
 * Copyright (c) 2015 All Rights Reserved, Sean Harre
 *
 * Sean Harre is the copyright holder of all code below.
 * Do not re-use without permission.
 */

#include "soundCtrl.h"

// this file handles playing sounds when collisions occur
// where L/R pan and collision_force are taken into account.

//--------------------------
SoundCtrl::SoundCtrl()
    : debug_mode(false)
{ }

//--------------------------
SoundCtrl::~SoundCtrl()
{
    (void)stop();
}

//--------------------------
bool SoundCtrl::start(string filename)
{
    dbprintf("Init sound:%s...\n",filename.c_str());
    hit.loadSound(filename);
    hit.setVolume(0.75f);
    hit.setMultiPlay(true);
}

//--------------------------
bool SoundCtrl::stop()
{
    return true;
}

//--------------------------
void SoundCtrl::make_collision_sounds(const HandVector::hand_vecs left, const HandVector::hand_vecs right)
{
    hit.setPan(-1.0f); //Pans to the left
    for (int f=0; f<MAX_FINGERS; ++f) {
        for (int j=0; j<MAX_JOINTS; ++j) {
            
            // is_this_special_thumb_joint(f,j)
            if ( ! ((f==0) && (j==1)) ) {
                
                float cf = left.fingers[f].joints[j].collision_force;
                if (cf > 0.0f) {
                    // TODO: float ofNormalize(float value, float min, float max)
                    float vol = ::min(1.0f,cf/MAX_IMPULSE_DATA);    // TODO: ofMap()
                    hit.setPan(-1.0f);  // TODO: need each time?
                    hit.setVolume(vol);
                    hit.play();
                    dbprintf("SND[%d:%d] Left imp=%f vol=%f\n",f,j,cf,vol);
                }
            }
        }
    }
    hit.setPan(1.0f); //Pans to the right
    for (int f=0; f<MAX_FINGERS; ++f) {
        for (int j=0; j<MAX_JOINTS; ++j) {
            
            // is_this_special_thumb_joint(f,j)
            if ( ! ((f==0) && (j==1)) ) {
                
                float cf = right.fingers[f].joints[j].collision_force;
                if (cf > 0.0f) {
                    // TODO: float ofNormalize(float value, float min, float max)
                    float vol = ::min(1.0f,cf/MAX_IMPULSE_DATA);    // TODO: ofMap()
                    hit.setPan(1.0f);   // TODO: need each time?
                    hit.setVolume(vol);
                    hit.play();
                    dbprintf("SND[%d:%d] Right imp=%f vol=%f\n",f,j,cf,vol);
                }
            }
        }
    }
}

//--------------------------
bool SoundCtrl::set_debug_mode(bool newv)
{
    bool was = debug_mode;
    debug_mode = newv;
    return was;
}

