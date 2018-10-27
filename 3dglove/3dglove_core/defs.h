
/*
 * $Copyright$
 * Copyright (c) 2015 All Rights Reserved, Sean Harre
 *
 * Sean Harre is the copyright holder of all code below.
 * Do not re-use without permission.
 */

#ifndef _DEFS_H_
#define _DEFS_H_

#ifdef PI
#undef PI
#endif
#ifdef DEG_TO_RAD
#undef DEG_TO_RAD
#endif
#ifdef RAD_TO_DEG
#undef RAD_TO_DEG
#endif

#include "kinatouch.h"
#include "Leap.h"
#include <btBulletDynamicsCommon.h>
#include "hand_vectors.h"
#include "unistd.h"
#include <vector>
#include <string>
#include "prof.h"

#include <BulletSoftBody/btSoftBody.h>
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <BulletSoftBody/btDefaultSoftBodySolver.h>
#include <BulletSoftBody/btSoftBodyHelpers.h>
#include "BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h"

#if 0
# define dbprintf        printf
#else
# if DEBUG
#  define dbprintf        printf
# else
#  define dbprintf(a,...)
# endif
#endif

// Hardware device cfg
//----------------------
#define CFG_HAS_RIGHT_HAND      0   // has FTDI USB plug for right hand
#define CFG_HAS_LEFT_HAND       0   // has FTDI USB plug for left hand
#define CFG_HAS_LEAP_CTRL       1   // has Leap Motion controller

// soft hand
#define CFG_DRAW_SOFTBODY       0
#define CFG_DRAW_HAND_SOFT      0

// get hand data for joints or bones
#define CFG_JOINT_BONE_CENTER   0

// networking cfg
#define CFG_NETWORKING          0

// misc
//#define DISABLE_PROFILING       0   // TODO: doesn't work


//----------------------

// hand
#define JOINT_UNDEF         ofVec3f(-500,-500,-500)
#define LEAP_JOINT_UNDEF    Leap::Vector(JOINT_UNDEF.x,JOINT_UNDEF.y,JOINT_UNDEF.z)

// bullet hand soft body joints
#define BULLET_HAND_SOFT_INIT_POS    btVector3(100,150,-150)
#define BULLET_HAND_SOFT_INIT_SIZE   HAND_SPHERE_RADIUS*15
#define BULLET_HAND_SOFT_INIT_SIDES  12
#define BULLET_HAND_SOFT_INIT_WEIGHT 1.0f

// ftdi
#define BYTES_TO_TLC5947    64  // 64bytes/USB pkt <-> FTDI
#define FTDI_MAX_DEVICES    2
#define FTDI_RH_SN          "FTWY0DZK" // "FTWY0A04"  // TODO: "FTWY0DZK"
#define FTDI_LH_SN          "FTXRO1EL" // "FTWY0A04"

// tlc5947
#define GS_MAX              ((1<<12)-1) // 4095 // 2^12
#define GS_CHANNELS         24
#define GS_BYTES            ((GS_CHANNELS*12)/8)  // bytes, ie 288bits/8 -> TLC5947

// leap
#define LEAP_TIME_NO_HAND_DISAPPEAR_NOFILT  1 // (OF_MAX_FRAME_RATE/30) // actually a count
#define LEAP_TIME_NO_HAND_DISAPPEAR_FILT    (OF_MAX_FRAME_RATE/5) // actually a count
#define LEAP_FILTER_COEFF   0.9999f

// bullet
#define STEP_WORLD_UPDATE_HZ    200.f   // 200=every 5ms TODO: feels this number should be dynamic
#define STEP_WORLD_MAX_UPDATE_TIMES 10

// bullet soft body
#define BULLET_SOFT_INIT_POS    btVector3(20,350,20)
#define BULLET_SOFT_INIT_SIZE   60
#define BULLET_SOFT_INIT_SIDES  32
#define BULLET_SOFT_INIT_WEIGHT 1.0f

#endif  // _DEFS_H_
