
/*
 * $Copyright$
 * Copyright (c) 2015 All Rights Reserved, Sean Harre
 *
 * Sean Harre is the copyright holder of all code below.
 * Do not re-use without permission.
 */

#ifndef HANDS_H
#define HANDS_H

#include "3dtarget.h"

class leapDriver;
leapDriver& getLeapDrv();

class leapDriver {
    
public:
    friend leapDriver& getLeapDrv();
private:
    leapDriver();
public:
    virtual ~leapDriver();

    // latest Leap hand positions
    HandVector::hand_vecs lhand;
    HandVector::hand_vecs rhand;

    HandVector::hand_vecs get_right_hand_filtered(const Leap::Controller& controller);
    HandVector::hand_vecs get_left_hand_filtered(const Leap::Controller& controller);
    
    bool start(const Leap::Controller& controller);
    bool stop();
    
    
    void clear_impulse_data(HandVector::hand_vecs& hand);
    
    // debug
    bool set_debug_mode(bool newv);
    
    // config
    void set_hand_filtering(bool set_on);
    bool get_hand_filtering();
    
    // for xlating the bone basis() to quaternion
    ktQuaternion LeapBasis_to_ktQuaternion(Leap::Matrix basis);
    
    // for finger 0, joint 1, is the special thumb joint w/ zero length
    bool is_this_special_thumb_joint(int finger, int joint);
    
private:
    
    // filtering
    int rfound;
    int lfound;
    int disappear_count;
    Leap::Vector flt(const Leap::Vector newv, const Leap::Vector oldv, const float coeff, const float confid);
    HandVector::hand_vecs deep_copy_hand(const Leap::Hand& hand, bool isLeft);
    
    // config
    bool do_leap_filtering;
    
    // debug
    bool debug_mode;
};

#endif  // HANDS_H
