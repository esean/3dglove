
/*
 * $Copyright$
 * Copyright (c) 2015 All Rights Reserved, Sean Harre
 *
 * Sean Harre is the copyright holder of all code below.
 * Do not re-use without permission.
 */

#include "bulletdriver.h"

// standalone function to instantiate a singleton bulletDriver
bulletDriver& getBullet()
{
    static bulletDriver* bulletPtr = 0;
    if (!bulletPtr) {
        cout<< "create instance of bullet" <<endl;
        bulletPtr = new bulletDriver;
    }
    return *bulletPtr;
}

//--------------------------
bulletDriver::bulletDriver()
{
    allObjectVector.reserve(1000);
}

//--------------------------
bulletDriver::~bulletDriver()
{
    stop();
}

//--------------------------
btVector3 ktVec3_to_btVec3(ktVector in)         { return btVector3(in.x,in.y,in.z); }
btQuaternion ktQuat_to_btQuat(ktQuaternion in)  { return btQuaternion(in.x,in.y,in.z,in.w); }
ktVector btVec3_to_ktVec3(btVector3 in)         { return ktVector(in.x(),in.y(),in.z()); }
ktQuaternion btQuat_to_ktQuat(btQuaternion in)  { return ktQuaternion(in.x(),in.y(),in.z(),in.w()); }


//--------------------------
void bulletDriver::start()
{
    // Set up the collision configuration and dispatcher
    collisionConfiguration = new btSoftBodyRigidBodyCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
 
    // Build the broadphase
    broadphase = new btDbvtBroadphase();
    
    // The actual physics solver
    solver = new btSequentialImpulseConstraintSolver;
    soft_solver = 0; // new btDefaultSoftBodySolver;
    
    // The world._
    dynamicsWorld = new btSoftRigidDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration,soft_solver);
    dynamicsWorld->setGravity(btVector3(0,-9.8,0));
    
    btSoftBody* psb;

    // soft bodies are experimental
#if CFG_DRAW_SOFTBODY
    psb = make_kinematic_softJoint(BULLET_SOFT_INIT_WEIGHT, BULLET_SOFT_INIT_SIZE,
                                    BULLET_SOFT_INIT_POS);
    psb->setUserPointer(RB_INDEX_SOFT);
#endif
    
    // start timer for stepping engine
    btClk.reset();
}

//--------------------------
void bulletDriver::add_hand_kinematic_objects_to_bullet(const HandVector::hand_vecs& lhand, const HandVector::hand_vecs& rhand)
{
    btRigidBody* prb;
    btSoftBody* psb;
    
    // all leap joints
    {
        //create a dynamic rigidbody object for kinematic hands
        
        btCollisionShape* colShape = new btSphereShape(btScalar(HAND_SPHERE_RADIUS));
        collisionShapes.push_back(colShape);
        
        btScalar	mass(0.f);
        btVector3 localInertia(0,0,0);
        
        // add r/l hand rigidbodies
        // first do thumb, it only has 4 joints
        for (int j=0; j<MAX_JOINTS; ++j)
        {
            // is_this_special_thumb_joint(0,j)
            if (j != 1)
            {
                prb = make_kinematic_joint(colShape,mass,localInertia);
                prb->setUserPointer((void*)&lhand.fingers[0].joints[j].idx);
                prb = make_kinematic_joint(colShape,mass,localInertia);
                prb->setUserPointer((void*)&rhand.fingers[0].joints[j].idx);
                
#if 0 //CFG_DRAW_HAND_SOFT
                psb = make_kinematic_softJoint(BULLET_HAND_SOFT_INIT_WEIGHT, BULLET_HAND_SOFT_INIT_SIZE,
                                               BULLET_HAND_SOFT_INIT_POS + btVector3(j*2,j*2,j*2));
                psb->setUserPointer((void*)&lhand.fingers[0].joints[j].idx);
                psb = make_kinematic_softJoint(BULLET_HAND_SOFT_INIT_WEIGHT, BULLET_HAND_SOFT_INIT_SIZE,
                                               BULLET_HAND_SOFT_INIT_POS + btVector3(j*3,j*3,j*3));
                psb->setUserPointer((void*)&rhand.fingers[0].joints[j].idx);
#endif
            }
        }
        // do other 4 fingers
        for (int f=1; f<MAX_FINGERS; ++f)
        {
            for (int j=0; j<MAX_JOINTS; ++j)
            {
                prb = make_kinematic_joint(colShape,mass,localInertia);
                prb->setUserPointer((void*)&lhand.fingers[f].joints[j].idx);
                prb = make_kinematic_joint(colShape,mass,localInertia);
                prb->setUserPointer((void*)&rhand.fingers[f].joints[j].idx);
                
#if 0 //CFG_DRAW_HAND_SOFT
                psb = make_kinematic_softJoint(BULLET_HAND_SOFT_INIT_WEIGHT, BULLET_HAND_SOFT_INIT_SIZE,
                                               BULLET_HAND_SOFT_INIT_POS + btVector3(j*4,j*4,j*4));
                psb->setUserPointer((void*)&lhand.fingers[0].joints[j].idx);
                psb = make_kinematic_softJoint(BULLET_HAND_SOFT_INIT_WEIGHT, BULLET_HAND_SOFT_INIT_SIZE,
                                               BULLET_HAND_SOFT_INIT_POS + btVector3(j*5,j*5,j*5));
                psb->setUserPointer((void*)&rhand.fingers[0].joints[j].idx);
#endif
            }
        }
        
#if CFG_DRAW_HAND_SOFT
        psb = make_kinematic_softJoint(BULLET_HAND_SOFT_INIT_WEIGHT, BULLET_HAND_SOFT_INIT_SIZE,
                                       BULLET_HAND_SOFT_INIT_POS);
        psb->setUserPointer(RB_INDEX_HAND_SOFT | RB_INDEX_RH);
        psb = make_kinematic_softJoint(BULLET_HAND_SOFT_INIT_WEIGHT, BULLET_HAND_SOFT_INIT_SIZE,
                                       BULLET_HAND_SOFT_INIT_POS);
        psb->setUserPointer(RB_INDEX_HAND_SOFT | RB_INDEX_LH);
#endif
    }

}

//--------------------------
void bulletDriver::removeRigidBody(btRigidBody* rb)
{
    assert(rb);
    dynamicsWorld->removeRigidBody(rb);
    assert(rb->getMotionState());
    delete rb->getMotionState();
    delete rb;
}

// BIG OL HACK to get bullet to call back to threadWorker
static bulletDriver::myBtCallback cb_fcn;
static void* cb_this;

//--------------------------
static void bullet_callback(btDynamicsWorld *world, btScalar timeStep)
{
    // call thread worker...
    cb_fcn(cb_this,world,timeStep);
}

//--------------------------
void bulletDriver::set_cb(myBtCallback cb, void* this_info)
{
    // substep callback fcn, we will callback to bullet
    // layer, then bullet will callback to class that
    // wants the callback... c++ member callbacks are difficult
    dynamicsWorld->setInternalTickCallback(&bullet_callback);
    
    // store away callback info to threadWorker
    cb_fcn = cb;
    cb_this = this_info;
}

//--------------------------
vector<btSoftBody*> bulletDriver::get_soft_body()
{
    vector<btSoftBody*> ret;
    for (int j=0; j<dynamicsWorld->getNumCollisionObjects(); ++j)
    {
        btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[j];
        ktObjectInfo* userInfo = (ktObjectInfo*)obj->getUserPointer();
        
         //todo - if (body && body->getMotionState() && is_a_soft(*userInfo))
        if (is_a_soft(*userInfo))
            ret.push_back(btSoftBody::upcast(obj));
    }
    return ret;
}

//--------------------------
void bulletDriver::step_world(btScalar update_rate)
{
    // how long has it been since last step_world?
    float us = btClk.getTimeMicroseconds() / 1000000.0f;
    btClk.reset();
    
    // simulate forwards
    dynamicsWorld->stepSimulation(us,STEP_WORLD_MAX_UPDATE_TIMES,1.f/update_rate);
}

//--------------------------
void bulletDriver::stop()
{
	//remove the rigidbodies from the dynamics world and delete them
	for (int i=dynamicsWorld->getNumCollisionObjects()-1; i>=0 ;i--)
	{
		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		dynamicsWorld->removeCollisionObject( obj );
		//delete obj;   // sbh0816 crashes soft body
	}
    
	//delete collision shapes
	for (int j=0;j<collisionShapes.size();j++)
	{
		btCollisionShape* shape = collisionShapes[j];
		collisionShapes[j] = 0;
		delete shape;
	}
    
    delete collisionConfiguration;
    delete soft_solver;
    
    delete solver;
    delete dispatcher;
    delete broadphase;
    
    delete dynamicsWorld;
    
    //next line is optional: it will be cleared by the destructor when the array goes out of scope
	collisionShapes.clear();
}

//--------------------------
// push Leap data into physics engine
//
void bulletDriver::update_kinematics(const HandVector::hand_vecs& lhand, const HandVector::hand_vecs& rhand)
{
    // set positions of all objects
    for (int j=dynamicsWorld->getNumCollisionObjects()-1; j>=0 ; --j)
    {
        btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[j];
        btRigidBody* body = btRigidBody::upcast(obj);
        
        // only want to update fingers
        if (body)
        {
            ktObjectInfo* userInfo = (ktObjectInfo*)body->getUserPointer();
            
            if (body->getMotionState() && is_a_hand(*userInfo))
            {
                int finger = give_finger_info(*userInfo);
                int joint = give_joint_info(*userInfo);
                bool isLeft = is_left_hand(*userInfo);
                
                btTransform trans;
                body->getMotionState()->getWorldTransform(trans);
                Leap::Vector t;
                if (isLeft)
                    t = lhand.fingers[finger].joints[joint].pos;
                else
                    t = rhand.fingers[finger].joints[joint].pos;
                trans.setOrigin(btVector3(t.x,t.y,t.z));
                dynamic_cast<testMotionState*>(body->getMotionState())->setKinematicWorldTransform(trans);
            }
        }
    }
}

// HACK - should give 'this' to bullet, then we call back and have our this info
typedef struct {
    btVector3 impulse;
    btScalar distance;
    btRigidBody* prb;
} impulse_location_data;
vector<impulse_location_data> m_vecimp;

//--------------------------
// When creating a softBody, add a soft-rigid callback with,
//
//        psb->soft_collision_cb = &bulletDriver::myKtSoftBody_cb;
//
// Then that CB is called for each collision, passing impulse vector.
//
void bulletDriver::myKtSoftBody_cb(btSoftBody* psb, btRigidBody* prb, btVector3 imp)
{
    ktObjectInfo* userInfo = (ktObjectInfo*)psb->getUserPointer();
    
    if ( ! is_a_wall( *userInfo) )
    {
        btScalar impulse = sqrt(imp.getX()*imp.getX() + imp.getY()*imp.getY() + imp.getZ()*imp.getZ());
        if (impulse > 0.0f)
        {
            //printf("%f,%f,%f\n",imp.getX(),imp.getY(),imp.getZ()); // GOT SOFT COLLISION! 0x%x %f\n",userInfo,impulse);
            
            impulse_location_data new_imp;
            new_imp.impulse = btVector3(imp.getX(),imp.getY(),imp.getZ());
            new_imp.distance = 0.0f;
            new_imp.prb = prb;  // save ptr to btRigidBody
            m_vecimp.push_back(new_imp);
        }
    }
}

//--------------------------
// with updated Leap data, add impulse&depth info for any collision objects
//
bool bulletDriver::update_impulses(HandVector::hand_vecs& lhand, HandVector::hand_vecs& rhand)
{
    // TODO: clear force&depth values for those joints not in this list
    bool new_collisions = false;
    
    int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
    for (int i=0; i<numManifolds; ++i)
    {
        btPersistentManifold* contactManifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
        btCollisionObject* obA = const_cast<btCollisionObject*>(contactManifold->getBody0());
        btCollisionObject* obB = const_cast<btCollisionObject*>(contactManifold->getBody1());

        btRigidBody* bodyA = btRigidBody::upcast(obA);
        btRigidBody* bodyB = btRigidBody::upcast(obB);

        ktObjectInfo* userA = (ktObjectInfo*)bodyA->getUserPointer();
        ktObjectInfo* userB = (ktObjectInfo*)bodyB->getUserPointer();
        
        // don't care about collisions with walls
        // SBH050515 - optimization -> since bullet doesn't tell us about interactions with objects
        //             less than 1.0f mass, we don't need to look at collisions with walls
        //if ( !is_a_wall(*userA) && !is_a_wall(*userB) && (is_a_hand(*userA) || is_a_hand(*userB)))
        if ( is_a_hand(*userA) || is_a_hand(*userB) )
        {
            int numContacts = contactManifold->getNumContacts();
            
            if (numContacts > 0)
                dbprintf("RBCOLL: 0x%X -> 0x%X num=%d\n",userA,userB,numContacts);
            
            for (int j=0; j<numContacts; ++j)
            {
                btManifoldPoint& pt = contactManifold->getContactPoint(j);
                
                btScalar impulse = pt.m_appliedImpulse;
                btScalar distance = pt.m_distance1;
                
                if (impulse > 0.0f)
                {
                    ktObjectInfo handInfo;
                    ktObjectInfo shapeInfo;
                    btVector3 ptA = pt.getPositionWorldOnA();
                    btVector3 ptB = pt.getPositionWorldOnB();
                    ktVector pos;
                    
                    if (is_a_hand(*userA))
                    {
                        handInfo = *userA;
                        shapeInfo = *userB;
                        pos = btVec3_to_ktVec3(ptB);
                    }
                    else if (is_a_hand(*userB))
                    {
                        shapeInfo = *userA;
                        handInfo = *userB;
                        pos = btVec3_to_ktVec3(ptA);
                    }
                    else
                    {
                        // something has collided but it's not collided with the glove hand elements
                        //dbprintf("... A=0x%X(%d) B=0x%X(%d) -> NOT HANDS\n",userA,userA,userB,userB);
                        continue;
                    }
                    dbprintf("\n");

                    int finger = give_finger_info(handInfo);
                    int joint = give_joint_info(handInfo);
                    bool isLeft = is_left_hand(handInfo);
                    
                    // callback if requested for this shape
                    if (shapeInfo.coll_cb != NULL)
                    {
                        ktCollisionInfo cb;
                        cb.whichShape = shapeInfo.shapePtr;
                        cb.shape_type = shapeInfo.shape_type;
                        cb.impactImpulse = ktVector(impulse, pt.m_appliedImpulseLateral1, pt.m_appliedImpulseLateral2);
                        cb.handCollisionPoint.finger = finger;
                        cb.handCollisionPoint.joint = joint;
                        cb.handCollisionPoint.which_hand = (isLeft) ? KT_LEFT_HAND : KT_RIGHT_HAND;
                        cb.handCollisionPoint.position = pos;
                        shapeInfo.coll_cb(cb);
                    }
                    
                    dbprintf("LeftHand?=%d F[%d] J[%d] => [%d:%d] IMP=%f DIST=%f\n",isLeft,finger,joint,i,j,impulse,distance);
                    
                    if (isLeft)
                    {
                        lhand.fingers[finger].joints[joint].collision_force = impulse;
                        lhand.fingers[finger].joints[joint].depth = distance;
                    }
                    else
                    {
                        rhand.fingers[finger].joints[joint].collision_force = impulse;
                        rhand.fingers[finger].joints[joint].depth = distance;
                    }
                    
                    new_collisions = true;
                }
            }
        }
        
        //you can un-comment out this line, and then all points are removed
        contactManifold->clearManifold();
    }
    
    for(std::vector<impulse_location_data*>::size_type i=0; i != m_vecimp.size(); i++)
    {
        impulse_location_data* pild = &m_vecimp[i];
        
        // take magnitude of impulse
        btScalar impulse = sqrt(pild->impulse.getX()*pild->impulse.getX() +
                                pild->impulse.getY()*pild->impulse.getY() + pild->impulse.getZ()*pild->impulse.getZ());
        
        if (impulse > 0.0f)
        {
            btRigidBody* prb = pild->prb;
            ktObjectInfo* userInfo = (ktObjectInfo*)prb->getUserPointer();
            btScalar distance = pild->distance;
            
            int finger = give_finger_info(*userInfo);
            int joint = give_joint_info(*userInfo);
            bool isLeft = is_left_hand(*userInfo);
            
            dbprintf("SOFTCOLL LeftHand?=%d F[%d] J[%d] => [%d] IMP=%f DIST=%f\n",isLeft,finger,joint,i,impulse,0.0f); // distance);
            //printf("%f,%f,%f\n",pild->impulse.getX(),pild->impulse.getY(),pild->impulse.getZ());
            
            if (isLeft)
            {
                lhand.fingers[finger].joints[joint].collision_force = impulse;
                lhand.fingers[finger].joints[joint].depth = distance;
            }
            else
            {
                rhand.fingers[finger].joints[joint].collision_force = impulse;
                rhand.fingers[finger].joints[joint].depth = distance;
            }
            
            //                    btVector3 ptA = pt.getPositionWorldOnA();
            //                    btVector3 ptB = pt.getPositionWorldOnB();
            
            new_collisions = true;
        }
    }
    m_vecimp.clear();
    
    return new_collisions;
}

//--------------------------
btRigidBody* bulletDriver::make_sphere(btVector3 position, btQuaternion orientation, btVector3 size, float mass, btVector3 localInertia)
{
    btCollisionShape* colShape = new btSphereShape(btScalar(size.x()));
    collisionShapes.push_back(colShape);
    
    /// Create Dynamic Objects
    btTransform startTransform;
    startTransform.setIdentity();
    
//    //btScalar	mass(100.0f);
//    btScalar	mass(1.0f); //A: We should try to normalize everything we can
    
    //rigidbody is dynamic if and only if mass is non zero, otherwise static
    bool isDynamic = (mass != 0.f);
    
    //btVector3 localInertia(0,-1,0);
//    btVector3 localInertia(0,0,0); //A: I think for a sphere this should be (0,0,0) or (1,1,1)
    if (isDynamic)
        colShape->calculateLocalInertia(mass,localInertia);
    
    startTransform.setOrigin(position);
    
    //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
    btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,colShape,localInertia);
    rbInfo.m_restitution = 0.6f; //A: This looked more realistic from 0.8f
    rbInfo.m_friction = 0.8f;
    btRigidBody* body = new btRigidBody(rbInfo);
    
    //body->setLinearVelocity(btVector3(10,0,-5)); // for testing
    
    dynamicsWorld->addRigidBody(body);
    
    return body;
}

//--------------------------
btRigidBody* bulletDriver::make_cylinder(btVector3 position, btQuaternion orientation, btVector3 size, float mass, btVector3 localInertia)
{
    //create a dynamic rigidbody object for blocks
    
    btCollisionShape* shape = new btCylinderShape(size);
    collisionShapes.push_back(shape);
    
    //rigidbody is dynamic if and only if mass is non zero, otherwise static
    bool isDynamic = (mass != 0.f);
    
    if (isDynamic)
        shape->calculateLocalInertia(mass,localInertia);
    
    btTransform trans;
    trans.setIdentity();
    trans.setOrigin(position);
    
    //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
    testMotionState* myMotionState = new testMotionState(trans);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,shape,localInertia);
    rbInfo.m_restitution = 0.6f; //A: This looked more realistic from 0.8f
    rbInfo.m_friction = 0.8f;
    rbInfo.m_angularDamping = 0.99f;
    rbInfo.m_linearDamping = 0.99f;
    btRigidBody* body = new btRigidBody(rbInfo);
    body->setActivationState( DISABLE_DEACTIVATION );
    
    //body->setLinearVelocity(btVector3(0,0,10));  // todo - testing
    
    dynamicsWorld->addRigidBody(body);
    
    return body;
}

//--------------------------
btRigidBody* bulletDriver::make_cube(btVector3 position, btQuaternion orientation, btVector3 size, float mass, btVector3 localInertia)
{
    //create a dynamic rigidbody object for blocks
    
    btCollisionShape* shape = new btBoxShape(size);
    collisionShapes.push_back(shape);
    
    //rigidbody is dynamic if and only if mass is non zero, otherwise static
    bool isDynamic = (mass != 0.f);
    
    if (isDynamic)
        shape->calculateLocalInertia(mass,localInertia);
    
    btTransform trans;
    trans.setIdentity();
    trans.setOrigin(position);
    
    //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
    testMotionState* myMotionState = new testMotionState(trans);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,shape,localInertia);
    rbInfo.m_restitution = 0.6f; //A: This looked more realistic from 0.8f
    rbInfo.m_friction = 0.8f;
    rbInfo.m_angularDamping = 0.99f;
    rbInfo.m_linearDamping = 0.99f;
    btRigidBody* body = new btRigidBody(rbInfo);
    body->setActivationState( DISABLE_DEACTIVATION );

    //body->setLinearVelocity(btVector3(0,0,10));  // todo - testing
    
    dynamicsWorld->addRigidBody(body);
    
    return body;
}

//--------------------------
//    {
//        // Sphere seems to work better than the cube, but still buggy
//
//        btSoftBody*	psb=btSoftBodyHelpers::CreateEllipsoid(dynamicsWorld->getWorldInfo(),BULLET_SOFT_INIT_POS,
//                                                           btVector3(1,1,1)*BULLET_SOFT_INIT_SIZE,
//                                                           BULLET_SOFT_INIT_SIDES);
//
////        psb->m_materials[0]->m_kLST	=	0.1;      // Linear stiffness
////        psb->m_materials[0]->m_kAST	=	0.9;      // Area stiffness
////        psb->m_materials[0]->m_kVST	=	0.9;      // volume stiffness
////        psb->m_cfg.kDF				=	1;        // Fricition
////        psb->m_cfg.kDP				=	0.01;    // Dampening
////        psb->m_cfg.kPR				=	25000;    // Pressure
//        psb->m_materials[0]->m_kLST	=	0.5;      // Linear stiffness
//        psb->m_materials[0]->m_kAST	=	0.99;      // Area stiffness
//        psb->m_materials[0]->m_kVST	=	0.99;      // volume stiffness
//        psb->m_cfg.kDF				=	1;        // Fricition
//        psb->m_cfg.kDP				=	0.01;    // Dampening
//        psb->m_cfg.kPR				=	1000; // 150000;    // Pressure
//
//        psb->setTotalMass(BULLET_SOFT_INIT_WEIGHT,true);
//
//        psb->soft_collision_cb = &bulletDriver::myKtSoftBody_cb;
//
//        dynamicsWorld->addSoftBody(psb);
//
//
//        // This code is for the CUBE, which needs work i believe becase the way the nodes are set up.
//
//        /*
//        btSoftBody* softbody = new btSoftBody(&dynamicsWorld->getWorldInfo());
//
//        btSoftBody::Material* pm = softbody->appendMaterial();
//        pm->m_kLST = 0.05;
//        pm->m_kAST = 0.90;
//        pm->m_kVST = 0.90;
//
//
//        softbody->generateBendingConstraints(2,pm);
//        softbody->m_cfg.piterations   =   2;
//        softbody->m_cfg.collisions   =   btSoftBody::fCollision::SDF_RS;
////      softbody->m_cfg.kPR = .1;     // The pressure seems to rip the cube apart....
//        softbody->m_cfg.kDF = 1;
//        softbody->m_cfg.kDP = 0.001;
//
//        softbody->randomizeConstraints();
//
//        softbody->setTotalMass(30.0,true);
//
//        createSoftCube(softbody, 5, 60); // this has to be called after the material is made.
//
//        softbody->generateClusters(64);
//
//        btTransform trans;
//        trans.setIdentity();
//        trans.setOrigin( btVector3(50,300,50) );
//        softbody->transform(trans);
//
//        dynamicsWorld->addSoftBody(softbody);
//         */
//	}
//--------------------------
btSoftBody* bulletDriver::make_kinematic_softJoint(btScalar mass, btScalar size, btVector3 position)
{
    btSoftBody*	psb=btSoftBodyHelpers::CreateEllipsoid(dynamicsWorld->getWorldInfo(),position,
                                                       btVector3(1,1,1)*size,
                                                       BULLET_HAND_SOFT_INIT_SIDES);
    
    //        psb->m_materials[0]->m_kLST	=	0.1;      // Linear stiffness
    //        psb->m_materials[0]->m_kAST	=	0.9;      // Area stiffness
    //        psb->m_materials[0]->m_kVST	=	0.9;      // volume stiffness
    //        psb->m_cfg.kDF				=	1;        // Fricition
    //        psb->m_cfg.kDP				=	0.01;    // Dampening
    //        psb->m_cfg.kPR				=	25000;    // Pressure
    psb->m_materials[0]->m_kLST	=	0.5;      // Linear stiffness
    psb->m_materials[0]->m_kAST	=	0.99;      // Area stiffness
    psb->m_materials[0]->m_kVST	=	0.99;      // volume stiffness
    psb->m_cfg.kDF				=	1;        // Fricition
    psb->m_cfg.kDP				=	0.01;    // Dampening
    psb->m_cfg.kPR				=	1000; // 150000;    // Pressure
    
    psb->setTotalMass(mass,true);
    
    psb->soft_collision_cb = &bulletDriver::myKtSoftBody_cb;
    //psb->m_cfg.collisions = btSoftBody::fCollision::SDF_RS;
    
    dynamicsWorld->addSoftBody(psb);
    
    return psb;
}

//--------------------------
btRigidBody* bulletDriver::make_kinematic_joint(btCollisionShape* shape, btScalar mass, btVector3 localInertia)
{
    //rigidbody is dynamic if and only if mass is non zero, otherwise static
    bool isDynamic = (mass != 0.f);
    
    if (isDynamic)
        shape->calculateLocalInertia(mass,localInertia);
    
    btTransform trans;
    trans.setIdentity();
    btVector3 pos(JOINT_UNDEF.x,JOINT_UNDEF.y,JOINT_UNDEF.z);
    trans.setOrigin(pos);
    
    //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
    testMotionState* myMotionState = new testMotionState(trans);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,shape,localInertia);
    rbInfo.m_restitution = 0.0f;
    rbInfo.m_friction = 2.0f;
    rbInfo.m_angularDamping = 2.0f;
    rbInfo.m_linearDamping = 2.0f;
    btRigidBody* body = new btRigidBody(rbInfo);
    body->setCollisionFlags( body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    body->setActivationState( DISABLE_DEACTIVATION );
    
    dynamicsWorld->addRigidBody(body);
    
    return body;
}

#if 0
//--------------------------
// Main call to create a DefaultMotionState btBoxShape
btRigidBody* bulletDriver::localCreateRigidBody(btVector3 position, btQuaternion orientation, btVector3 size, float mass, btVector3 localInertia, float margin)
{
    btCollisionShape* shape = new btBoxShape(size);
    btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));
    btTransform startTransform;
    
    collisionShapes.push_back(shape);
    startTransform.setIdentity();
    shape->setMargin(margin);
    startTransform.setOrigin(position);
    
	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);
    
	if (isDynamic)
		shape->calculateLocalInertia(mass,localInertia);
    
	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo cInfo(mass,myMotionState,shape,localInertia);
    cInfo.m_restitution = 1.0f;
    cInfo.m_friction = 0.2f;
	btRigidBody* body = new btRigidBody(cInfo);
    body->setActivationState( DISABLE_DEACTIVATION );
    
	dynamicsWorld->addRigidBody(body);
    
	return body;
}
#endif

//--------------------------
//res is the resolution of nodes in the cube. res=3 means a 3x3x3 cube.
btSoftBody* bulletDriver::createSoftCube(btSoftBody* softbody, int res, float size)
{

    // This generates a grid of nodes, with faces on the edges.
    // I believe it would be better to just have a hollow cube,
    // and set the internal pressure and other constraints to
    // give it structure.
    
    
    for(int i = 0; i < res; i++)
        for(int j = 0; j < res; j++)
            for(int k =0; k < res; k++)
                softbody->appendNode(btVector3((float)i/res*size,(float)j/res*size,(float)k/res*size),1);


    int index=0;
    for(int i = 0; i < res; i++)
        for(int j = 0; j < res; j++)
            for(int k =0; k < res-1; k++)
            {
                index = i*res*res+j*res+k;
                softbody->appendLink(index,index+1);
            }
    
    for(int i = 0; i < res; i++)
        for(int j = 0; j < res-1; j++)
            for(int k =0; k < res; k++)
            {
                index = i*res*res+j*res+k;
                softbody->appendLink(index,index+res);
            }
    
    for(int i = 0; i < res-1; i++)
        for(int j = 0; j < res; j++)
            for(int k =0; k < res; k++)
            {
                index = i*res*res+j*res+k;
                softbody->appendLink(index,index+res*res);
            }
    
    for(int j = 0; j < res-1; j++)
        for(int k = 0; k < res-1; k++)
            {
                index = j*res+k;
                softbody->appendTetra(index, index+1, index+1+res, index+res);
                
                index = (res-1)*res*res+j*res+k;
                softbody->appendTetra(index, index+1, index+1+res, index+res);
            }
    
    for(int i = 0; i < res-1; i++)
        for(int j = 0; j < res-1; j++)
        {
            index = i*res*res+j*res+(res-1);
            softbody->appendTetra(index, index+res, index+res+res*res, index+res*res);
            
            index = i*res*res+j*res+0;
            softbody->appendTetra(index, index+res, index+res+res*res, index+res*res);
        }
    
    for(int i = 0; i < res-1; i++)
        for(int k = 0; k < res-1; k++)
        {
            index = i*res*res+0+k;
            softbody->appendTetra(index, index+1, index+1+res*res, index+res*res);
            
            index = i*res*res+(res-1)*res+k;
            softbody->appendTetra(index, index+1, index+1+res*res, index+res*res);
        }
    
    
    return softbody;
    
}


