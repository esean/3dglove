
/*
 * $Copyright$
 * Copyright (c) 2015 All Rights Reserved, Sean Harre
 *
 * Sean Harre is the copyright holder of all code below.
 * Do not re-use without permission.
 */

#ifndef _3D_TARGET_H_
#define _3D_TARGET_H_

#define APP_VERSION "0.6.2"

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
#include "ofMain.h"
#include <btBulletDynamicsCommon.h>
#include "hand_vectors.h"
#include "unistd.h"
#include <vector>
#include <string>
#include "prof.h"
#include <limits.h>

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
#define CFG_HAS_RIGHT_HAND      1   // has FTDI USB plug for right hand
#define CFG_HAS_LEFT_HAND       0   // has FTDI USB plug for left hand
#define CFG_HAS_LEAP_CTRL       1   // has Leap Motion controller

// Initial physics shapes cfg
#define CFG_DRAW_BOUNDING_WALLS 1
#define CFG_DRAW_SPHERE         0
#define CFG_DRAW_CUBE           0
#define CFG_DRAW_SOFTBODY       0
#define CFG_DRAW_HAND_SOFT      0
#define CFG_DRAW_CYLINDER       0

// draw hand joints as cylinders/bones
#define CFG_DRAW_HAND_BONES     0

// ofApp draw collisions reported from API?
#define CFG_DRAW_COLLISIONS     0

// networking cfg
#define CFG_NETWORKING          0

// misc
#define DISABLE_PROFILING       0   // TODO: doesn't work


//----------------------

#define IS_NOT_FLOAT_ZERO(x)    (fabs((x)) >= std::numeric_limits<float>::min())

// glove
#define JOINTS_PER_HAND     24  // 5/finger, 4/thumb

// hand
#define HAND_SPHERE_RADIUS  7.0f
#define HAND_BONE_WIDTH     100.0f      // ??
#define JOINT_UNDEF         ofVec3f(-500,-500,-500)
#define LEAP_JOINT_UNDEF    Leap::Vector(JOINT_UNDEF.x,JOINT_UNDEF.y,JOINT_UNDEF.z)

// bullet hand soft body joints
#define BULLET_HAND_SOFT_INIT_POS    btVector3(100,150,-150)
#define BULLET_HAND_SOFT_INIT_SIZE   HAND_SPHERE_RADIUS*15
#define BULLET_HAND_SOFT_INIT_SIDES  12
#define BULLET_HAND_SOFT_INIT_WEIGHT 1.0f

// bounding walls
#if 0
// tight bounding walls, maybe too tight
#define SZ  230
#define SZH 340
#define WD  30
#define DIS 165
#define CAM_POSITION        ofPoint(-60,1750,1260)
#else
// more relaxed open space, can see more shapes and easier to interact with them
#define SZ  230
#define SZH 340
#define WD  30
#define DIS 230
#define CAM_POSITION        ofPoint(0,1950,2000)
#endif

// camera
#define CAM_DISTANCE        2500
#define CAM_TARGET          ofVec3f(0,0,0) //A: If the center is always (0,0,0) its easier to change the scale
#define CAM_SCALE           10.0f
#define TRACK_HAND_COEFF_VALID      ofVec3f(0.98, 0.98, 0.98)
#define TRACK_HAND_COEFF_NOTVALID   ofVec3f(0.995, 0.995, 0.995)

// ftdi
#define BYTES_TO_TLC5947    64  // 64bytes/USB pkt <-> FTDI
#define FTDI_MAX_DEVICES    2
#define FTDI_RH_SN          "FTWY0DZK" // "FTWY0A04"  // TODO: "FTWY0DZK"
#define FTDI_LH_SN          "FTXRO1EL" // "FTWY0A04"

// tlc5947
#define GS_MAX              ((1<<12)-1) // 4095 // 2^12
#define GS_CHANNELS         24
#define GS_BYTES            ((GS_CHANNELS*12)/8)  // bytes, ie 288bits/8 -> TLC5947

// oF
#define OF_MAX_FRAME_RATE   60

// leap
#define LEAP_TIME_NO_HAND_DISAPPEAR_NOFILT  1 // (OF_MAX_FRAME_RATE/30) // actually a count
#define LEAP_TIME_NO_HAND_DISAPPEAR_FILT    (OF_MAX_FRAME_RATE/5) // actually a count
#define LEAP_FILTER_COEFF   0.9999f

// bullet
#define STEP_WORLD_UPDATE_HZ    200.f   // 200=every 5ms TODO: feels this number should be dynamic
#define STEP_WORLD_MAX_UPDATE_TIMES 10

// bullet sphere
#define BULLET_SPHERE_RADIUS    btScalar(60.0f)
#define SPHERE_INITIAL          btVector3(0,350,-75)
#define MAX_IMPULSE_DATA        500.0f

// bullet soft body
#define BULLET_SOFT_INIT_POS    btVector3(20,350,20)
#define BULLET_SOFT_INIT_SIZE   60
#define BULLET_SOFT_INIT_SIDES  32
#define BULLET_SOFT_INIT_WEIGHT 1.0f

// bullet cube
#define START_POS               btVector3(0,0,-120)
#define START_WITH_THIS_MANY_CUBES      1   // in each x,y,z axis
#define ARRAY_SIZE_X    START_WITH_THIS_MANY_CUBES
#define ARRAY_SIZE_Y    START_WITH_THIS_MANY_CUBES
#define ARRAY_SIZE_Z    START_WITH_THIS_MANY_CUBES
#define BULLET_CUBE_INIT_SIZE   btVector3(50,50,50)

// statistics gui
#define DEFAULT_STAT_UPDATE_TIMES_PER_SEC   5
#define MOVGRAPH_DEPTH       512    // buffer depth to display in movWindow
#define GUI_WIDTH            230
#define GUI_TEXT_SIZE        OFX_UI_FONT_SMALL
#define DEFAULT_MOVWINDOW_MAX_TIME_US    500.0  // us max - TODO: want to minimize this!
#define DEFAULT_MOVWINDOW_MAX_UPDATE_HZ     120.0   // hz update rate
#define DEFAULT_MOVWINDOW_MAXOF_UPDATE_HZ    60.0   // OF is much slower

#endif  // _3D_TARGET_H_
