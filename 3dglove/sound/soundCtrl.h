
/*
 * $Copyright$
 * Copyright (c) 2015 All Rights Reserved, Sean Harre
 *
 * Sean Harre is the copyright holder of all code below.
 * Do not re-use without permission.
 */

#ifndef SOUNDCTRL_H
#define SOUNDCTRL_H

#include "3dtarget.h"

class SoundCtrl {
    
public:
    
    SoundCtrl();
    virtual ~SoundCtrl();

    bool start(string filename);
    bool stop();
    
    void make_collision_sounds(const HandVector::hand_vecs left, const HandVector::hand_vecs right);
    
    // debug
    bool set_debug_mode(bool newv);
    
private:
    bool debug_mode;
    
    ofSoundPlayer hit;
};

#endif  // SOUNDCTRL_H
