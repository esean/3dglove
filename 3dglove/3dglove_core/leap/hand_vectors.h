
/*
 * $Copyright$
 * Copyright (c) 2015 All Rights Reserved, Sean Harre
 *
 * Sean Harre is the copyright holder of all code below.
 * Do not re-use without permission.
 */

#include "3dtarget.h"

#ifndef HAND_VECS_H
#define HAND_VECS_H


//--------------------------
// Helper fcns
//
bool is_a_hand      (ktObjectInfo userInfo);
bool is_a_sphere    (ktObjectInfo userInfo);
bool is_a_wall      (ktObjectInfo userInfo);
bool is_a_cylinder  (ktObjectInfo userInfo);
bool is_a_soft      (ktObjectInfo userInfo);
bool is_a_cube      (ktObjectInfo userInfo);
bool is_left_hand   (ktObjectInfo userInfo);
int  give_finger_info (ktObjectInfo userInfo);
int  give_joint_info  (ktObjectInfo userInfo);

ktObjectInfo gen_and_stuff_UserInfo(int finger, int joint, bool is_left);

//--------------------------
// Copies items to a joint_vecs ref
void copy_one_joint_vecs(HandVector::joint_vecs& jv, Leap::Vector pos, float collision_force, float depth, ktObjectInfo idx, int ftdi_idx, int width, int length, ktQuaternion orientation);

#endif  // HAND_VECS_H
