
/*
 * $Copyright$
 * Copyright (c) 2015 All Rights Reserved, Sean Harre
 *
 * Sean Harre is the copyright holder of all code below.
 * Do not re-use without permission.
 */

#include "hand_vectors.h"

//--------------------------
// Helper fcns
//
bool is_a_hand      (ktObjectInfo userInfo)  {    return (userInfo.shape_type == KT_HAND); }
bool is_a_sphere    (ktObjectInfo userInfo)  {    return (userInfo.shape_type == KT_SPHERE); }
bool is_a_wall      (ktObjectInfo userInfo)  {    return (userInfo.shape_type == KT_BOUNDING_WALL); }
bool is_a_cylinder  (ktObjectInfo userInfo)  {    return (userInfo.shape_type == KT_CYLINDER); }
bool is_a_soft      (ktObjectInfo userInfo)  {
    switch (userInfo.shape_type)
    {
        case KT_SOFT:
        case KT_SOFT_HAND:
            return true;
    }
    return false;
}
bool is_a_cube  (ktObjectInfo userInfo)  {    return (userInfo.shape_type == KT_CUBE); }
bool is_left_hand (ktObjectInfo userInfo)  {    return (userInfo.is_left); }
int  give_finger_info (ktObjectInfo userInfo)  {    return (userInfo.hand_finger); }
int  give_joint_info  (ktObjectInfo userInfo)  {    return (userInfo.hand_joint); }

//--------------------------
ktObjectInfo gen_and_stuff_UserInfo(int finger, int joint, bool is_left) {
    ktObjectInfo lcl;
    lcl.shape_type = KT_HAND;
    lcl.coll_cb = NULL;
    lcl.shapePtr = NULL;
    if (is_left)
        lcl.is_left = true;
    else
        lcl.is_left = false;
    lcl.hand_finger = finger;
    lcl.hand_joint = joint;
    return lcl;
}

//--------------------------
// Copies items to a joint_vecs ref
void copy_one_joint_vecs(HandVector::joint_vecs& jv, Leap::Vector pos, float collision_force, float depth, ktObjectInfo idx, int ftdi_idx, int width, int length, ktQuaternion orientation)
{
    jv.pos = pos;
    jv.width = width;
    jv.length = length;
    jv.orientation = orientation;
    jv.collision_force = collision_force;
    jv.depth = depth;
    jv.ftdi_idx = ftdi_idx;
    jv.idx = idx;
}

