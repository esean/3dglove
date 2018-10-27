
/*
 * $Copyright$
 * Copyright (c) 2015 All Rights Reserved, Sean Harre
 *
 * Sean Harre is the copyright holder of all code below.
 * Do not re-use without permission.
 */

#ifndef BULLET_H
#define BULLET_H

#include "3dtarget.h"

//
// prototypes
//
class bulletDriver;
bulletDriver& getBullet();

class bulletDriver {
public:

    friend bulletDriver& getBullet();
private:
    bulletDriver();
public:
    
    // any object created
    // container for any user created objects, made from ktMakeShape() in API
    // hand joints are not included in this
    vector<ktObjectInfo> allObjectVector;
    
    virtual ~bulletDriver();
    
    void start();
    void stop();
    void step_world(btScalar update_rate);
    
    // remove a rigid body from bullet
    void removeRigidBody(btRigidBody* rb);
    
    // adds kinematic shapes to bullet for a hand
    void add_hand_kinematic_objects_to_bullet(const HandVector::hand_vecs& lhand, const HandVector::hand_vecs& rhand);
    
    // sets callback fcn for tick callbacks, must set after start()
    typedef void (*myBtCallback)(void* cb_this,btDynamicsWorld* world, btScalar timeStep);
    void set_cb(myBtCallback cb, void* this_info);
    // KT: cb for softbody collisions
    static void myKtSoftBody_cb(btSoftBody* psb, btRigidBody* prb, btVector3 impulse);
    
    vector<btSoftBody*> get_soft_body();
    
    void update_kinematics(const HandVector::hand_vecs& lhand, const HandVector::hand_vecs& rhand);
    bool update_impulses(HandVector::hand_vecs& lhand, HandVector::hand_vecs& rhand);
    
    //
    // SHAPE MAKERS
    //
    btRigidBody* make_sphere(btVector3 position, btQuaternion orientation, btVector3 size, float mass, btVector3 localInertia);
    btRigidBody* make_cube(btVector3 position, btQuaternion orientation, btVector3 size, float mass, btVector3 localInertia);
    btRigidBody* make_cylinder(btVector3 position, btQuaternion orientation, btVector3 size, float mass, btVector3 localInertia);
    
private:
    
    btBroadphaseInterface* broadphase;
    btSoftBodyRigidBodyCollisionConfiguration* collisionConfiguration;
    btCollisionDispatcher* dispatcher;
    btSequentialImpulseConstraintSolver* solver;
    btDefaultSoftBodySolver* soft_solver;
    btSoftRigidDynamicsWorld* dynamicsWorld;
    
    //keep track of the shapes, we release memory at exit.
	//make sure to re-use collision shapes among rigid bodies whenever possible!
	btAlignedObjectArray<btCollisionShape*> collisionShapes;
    
    // todo - create a hand creator
    btRigidBody* make_kinematic_joint(btCollisionShape* shape, btScalar mass, btVector3 localInertia);
    btSoftBody* make_kinematic_softJoint(btScalar mass, btScalar size, btVector3 position);
    btSoftBody* createSoftCube(btSoftBody* softbody, int res, float size);
    
    // provides delta time to stepSimulation
    btClock btClk;
};


struct	testMotionState : public btMotionState
{
	btTransform m_transform;
	btTransform	m_centerOfMassOffset;
	btTransform m_startWorldTrans;
    
	testMotionState(const btTransform& transform = btTransform::getIdentity())
        : m_transform(transform),
        m_startWorldTrans(transform)
	{
	}
    
	///synchronizes world transform from user to physics
	virtual void	getWorldTransform(btTransform& transform ) const
	{
        transform = m_transform;
	}
    
	///synchronizes world transform from physics to user
	///Bullet only calls the update of worldtransform for active objects
	virtual void	setWorldTransform(const btTransform& transform)
	{
            m_transform = transform;
	}
	
	void setKinematicWorldTransform(const btTransform& transform)
	{
            m_transform = transform;
	}
};

btVector3 ktVec3_to_btVec3(ktVector in);
btQuaternion ktQuat_to_btQuat(ktQuaternion in);
ktVector btVec3_to_ktVec3(btVector3 in);
ktQuaternion btQuat_to_ktQuat(btQuaternion in);

#endif  // BULLET_H

