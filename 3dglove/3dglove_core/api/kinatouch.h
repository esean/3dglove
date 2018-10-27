
/*
 * $Copyright$
 * Copyright (c) 2015 All Rights Reserved, Sean Harre
 *
 * Sean Harre is the copyright holder of all code below.
 * Do not re-use without permission.
 */

#ifndef _KINATOUCH_API_
#define _KINATOUCH_API_

#define KINATOUCH_VERSION   "0.1"

#include "Leap.h"   // todo
#include <vector>


//------------------------------------------------
// basic types
//------------------------------------------------

struct ktVector {
    float x,y,z;
    ktVector() {}
    ktVector(float x0, float y0, float z0): x(x0), y(y0), z(z0) {}
    void operator+=(ktVector& rhs)
    {
        this->x += rhs.x;
        this->y += rhs.y;
        this->z += rhs.z;
    }
};
struct ktQuaternion {
    float x,y,z,w;
    ktQuaternion() {}
    ktQuaternion(float x0, float y0, float z0, float w0): x(x0), y(y0), z(z0), w(w0) {}
};
// todo - use those defined in Leap.h?
enum ktHandType {
    KT_LEFT_HAND,
    KT_RIGHT_HAND
};
enum ktShapeType {
    KT_SPHERE,
    KT_CUBE,
    KT_CYLINDER,
    KT_BOUNDING_WALL,
    
    KT_HAND,             // can't call ktMakeShape()
    KT_SOFT,            //   "
    KT_SOFT_HAND,       //    "
    KT_PALM            //     "
};


//------------------------------------------------
// Only hands and shapes exist in KT world, and
// developer can see collisions between them.
//------------------------------------------------

// info about a single hand location/joint
struct ktJointInfo {
    ktHandType which_hand;
    // todo - replace with finger & joint enums (use from Leap.h)
    int finger;
    int joint;
    ktVector position;
    // todo: add in further finger detail from Jackson, then push to bullet, and draw
};

// info about a created shape
struct ktShapeInfo {
    ktShapeType type;
    ktVector position;
    ktQuaternion orientation;
    ktVector size;
    int mass;
    ktVector inertia;
};

// basic type of a kinatouch shape
typedef void* ktShapePtr;

// info about a single collision point
struct ktCollisionInfo {
    ktShapePtr whichShape;          // which of leap dev created shapes collided?
    ktShapeType shape_type;
    ktJointInfo handCollisionPoint;
    ktVector impactImpulse;
};

// shape collision callback, called when hands collide with a ktShape
typedef void (*myKtShapeCollisionCallback)(ktCollisionInfo);

// Main interface - creates a shape and returns a unique pointer to ktShape
ktShapePtr ktMakeShape(ktShapeInfo, myKtShapeCollisionCallback);

// delete a shape
void ktDeleteShape(ktShapePtr);

// get information about a shape
ktShapeInfo ktCurrentInformation(ktShapePtr);

// get information about multiple shapes of a certain type
std::vector<ktShapeInfo> ktCurrentInformationOfType(ktShapeType);

void ktMakeBoundingWalls(int dis, int sz, int szh, int wd);


//------------------------------------------------
// Local hand data
//------------------------------------------------

#define MAX_FINGERS     5
#define MAX_BONES       4
#define MAX_JOINTS      (MAX_BONES+1)

// a container for storing information about any collision object
// used by physics engine to quickly sort collisions and FTDI writes
// pertains mainly to hand data, but ktObjectInfo is used for any
// physics collision object so we can identify which shape it is.
// A void* to this information is in the btCollisionObject using
// the setUserPointer() method. So it travels with the shape.
struct ktObjectInfo {
    ktShapeType shape_type;
    myKtShapeCollisionCallback coll_cb; // fcn to call on collision
    ktShapePtr shapePtr;                // pointer to btRigidBody
    
    // these only pertain to hands and should be ignore for shapes
    bool is_left;
    int hand_finger;
    int hand_joint;
};

struct HandVector {
    
    // named joint_vecs but this is really info about the bones, from
    // leap we get the center of bone, length, width and orientation
    struct joint_vecs {
        Leap::Vector pos;   // actual (x,y,z) position of joint from Leap
        float width;
        float length;
        ktQuaternion orientation;
        
        float collision_force;  // cannot be trusted, is cleared in library, refer to impulse data in collision_cb instead
        float depth;    // not used today
        uint16_t ftdi_idx;       // this is mapping from finger/joint -> ftdi[] "location" mapping
        
        // this was the old 16bit mask mapping stored in physics userInfo int field
        // now we store a void* to thise ktObjectInfo in bullet UserPointer
        ktObjectInfo idx;
    };
    struct finger_vecs {
        joint_vecs joints[MAX_JOINTS];     // [0]=back of palm, [4]=finger tip
    };
    struct hand_vecs {
        finger_vecs fingers[MAX_FINGERS];     // uses Finger::Type enum
        joint_vecs palm_location;
        bool is_valid;
    };
    
};

// returns hand data for our local hands
HandVector::hand_vecs ktGet_left_hand_vector();
HandVector::hand_vecs ktGet_right_hand_vector();


//------------------------------------------------
// Initialize
//------------------------------------------------

void ktLibInit();

// remove any shapes and restart API
void ktRestart();


#endif
