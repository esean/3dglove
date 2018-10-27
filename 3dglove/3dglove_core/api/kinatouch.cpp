
/*
 * $Copyright$
 * Copyright (c) 2015 All Rights Reserved, Sean Harre
 *
 * Sean Harre is the copyright holder of all code below.
 * Do not re-use without permission.
 */

#include <kinatouch.h>
#include "bulletDriver.h"
#include "threadworker.h"
#include "leapDriver.h"

struct makeShapeShapes {
    ktShapeType shape;
    btRigidBody* prb;
};
vector<makeShapeShapes> all_shapes;

//--------------------------
// Main interface - creates a shape and returns a unique pointer to ktShape
ktShapePtr ktMakeShape(ktShapeInfo shapeInfo, myKtShapeCollisionCallback shape_cb)
{
    btRigidBody* prb;
    ktObjectInfo lcl;
    
    lcl.shape_type = shapeInfo.type;
    lcl.coll_cb = shape_cb;
    
    dbprintf("ktADD:%d, ",shapeInfo.type);
    switch (shapeInfo.type)
    {
        case KT_SPHERE:
            prb = getBullet().make_sphere(ktVec3_to_btVec3(shapeInfo.position),
                                    ktQuat_to_btQuat(shapeInfo.orientation),
                                    ktVec3_to_btVec3(shapeInfo.size),
                                    shapeInfo.mass,
                                    ktVec3_to_btVec3(shapeInfo.inertia));
            break;

        case KT_CUBE:
        case KT_BOUNDING_WALL:
            prb = getBullet().make_cube(ktVec3_to_btVec3(shapeInfo.position),
                                            ktQuat_to_btQuat(shapeInfo.orientation),
                                            ktVec3_to_btVec3(shapeInfo.size),
                                            shapeInfo.mass,
                                            ktVec3_to_btVec3(shapeInfo.inertia));
            break;
            
        case KT_CYLINDER:
            prb = getBullet().make_cylinder(ktVec3_to_btVec3(shapeInfo.position),
                                                ktQuat_to_btQuat(shapeInfo.orientation),
                                                ktVec3_to_btVec3(shapeInfo.size),
                                                shapeInfo.mass,
                                                ktVec3_to_btVec3(shapeInfo.inertia));
            break;
            
        default:
            assert(0);
            break;
    }
    
    lcl.shapePtr = prb;
    getBullet().allObjectVector.push_back(lcl);
    prb->setUserPointer(&getBullet().allObjectVector.back());
    
    // record this shape in our all_shapes memory
    makeShapeShapes this_shape;
    this_shape.prb = prb;
    this_shape.shape = shapeInfo.type;
    all_shapes.push_back(this_shape);
    dbprintf("size=%lu (%p)\n",all_shapes.size(),this_shape.prb);
    
    return (ktShapePtr)prb;
}

//--------------------------
// delete a shape
void ktDeleteShape(ktShapePtr this_one)
{
    if (!this_one)
        assert(0);
    
    for(std::vector<makeShapeShapes>::size_type i = 0; i < all_shapes.size(); ++i) {
        if (all_shapes[i].prb == (btRigidBody*)this_one) {
            dbprintf("ktDEL:%d size=%lu (%p)\n",all_shapes[i].shape,all_shapes.size(),all_shapes[i].prb);
            all_shapes.erase(all_shapes.begin()+i);
            getBullet().removeRigidBody((btRigidBody*)this_one);
            return;
        }
    }
}

//--------------------------
// get information about a shape
ktShapeInfo ktCurrentInformation(ktShapePtr shape)
{
    if (!shape)
        assert(0);
    
    ktShapeInfo ret;
    btRigidBody* body = (btRigidBody*)shape;
    assert(body);
    
    // get userindex
    ktObjectInfo* lcl = (ktObjectInfo*)body->getUserPointer();
    assert(lcl);
    ret.type = lcl->shape_type;
    
    switch (lcl->shape_type) {
        case KT_SPHERE:
        {
            btCollisionShape* shape = body->getCollisionShape();
            if (shape->getShapeType() == SPHERE_SHAPE_PROXYTYPE)
            {
                btScalar size = shape->getMargin();
                
                btTransform trans;
                body->getMotionState()->getWorldTransform(trans);
                
                ret.position = ktVector(trans.getOrigin().getX(),trans.getOrigin().getY(),trans.getOrigin().getZ());
                ret.orientation = ktQuaternion(0,0,0,0);
                ret.size = ktVector(size,size,size);
                ret.mass = 1.0f;    // tbd
                ret.inertia = ktVector(0,0,0);  // tbd
            }
            else
                assert(0);
        }
            break;
            
        case KT_CUBE:
        case KT_BOUNDING_WALL:
        {
            btCollisionShape* shape = body->getCollisionShape();
            if (shape->getShapeType() == BOX_SHAPE_PROXYTYPE)
            {
                const btBoxShape* boxShape = static_cast<const btBoxShape*>(shape);
                btVector3 halfExtent = boxShape->getHalfExtentsWithMargin();
                btVector3 origin = body->getCenterOfMassPosition();
                
                btTransform tr;
                body->getMotionState()->getWorldTransform(tr);
                btQuaternion quat = tr.getRotation();
                
                ret.position = ktVector(origin.x(), origin.y(), origin.z());
                ret.orientation = ktQuaternion(quat.x(),quat.y(),quat.z(),quat.w());
                ret.size = ktVector(halfExtent.x()*2.0f, halfExtent.y()*2.0f, halfExtent.z()*2.0f);
                ret.mass = 1.0f;    // tbd
                ret.inertia = ktVector(0,0,0);  // tbd
            }
            else
                assert(0);
        }
            break;
            
        case KT_CYLINDER:
        {
            btCollisionShape* shape = body->getCollisionShape();
            if (shape->getShapeType() == CYLINDER_SHAPE_PROXYTYPE)
            {
                const btCylinderShape* cylShape = static_cast<const btCylinderShape*>(shape);
                btVector3 halfExtent = cylShape->getHalfExtentsWithMargin();
                btVector3 origin = body->getCenterOfMassPosition();
                
                btTransform tr;
                body->getMotionState()->getWorldTransform(tr);
                btQuaternion quat = tr.getRotation();
                
                ret.position = ktVector(origin.x(), origin.y(), origin.z());
                ret.orientation = ktQuaternion(quat.x(),quat.y(),quat.z(),quat.w());
                ret.size = ktVector(halfExtent.x()*2.0f, halfExtent.y()*2.0f, halfExtent.z()*2.0f);
                ret.mass = 1.0f;    // tbd
                ret.inertia = ktVector(0,0,0);  // tbd
            }
            else
                assert(0);
        }
            break;
            
        default:
            assert(0);
            break;
    }
    
    return ret;
}

// find information about all shapes we know of this type and return their latest information
//--------------------------
std::vector<ktShapeInfo> ktCurrentInformationOfType(ktShapeType type)
{
    vector<ktShapeInfo> ret;
    
    for(std::vector<makeShapeShapes>::size_type i = 0; i < all_shapes.size(); ++i) {
        if (all_shapes[i].shape == type)
            ret.push_back( ktCurrentInformation((ktShapePtr)all_shapes[i].prb) );
    }
    
    return ret;
}

// from jj
// mom stingks abounch!!!!!!!!!!
//--------------------------
void ktRestart()
{
    dbprintf("ktRESTART\n");
    while ( all_shapes.size() > 0) {
        for(std::vector<makeShapeShapes>::size_type i = 0; i < all_shapes.size(); ++i) {
            ktDeleteShape((ktShapePtr)all_shapes[i].prb);
            break;
        }
    }
    all_shapes.clear();
    
    // restart physics engine and wait for it to be done restarting
    getTW().restart_physics();
    while ( ! getTW().done_restarting())
        ;
}

//--------------------------
void ktLibInit()
{
    dbprintf("ktLibInit\n");
    getTW().start();
}

//--------------------------
HandVector::hand_vecs ktGet_left_hand_vector()
{
    return getLeapDrv().lhand;
}

//--------------------------
HandVector::hand_vecs ktGet_right_hand_vector()
{
    return getLeapDrv().rhand;
}

//--------------------------
void ktMakeBoundingWalls(int dis, int sz, int szh, int wd)
{
    ktShapeInfo shapeInfo;
    shapeInfo.type = KT_BOUNDING_WALL;
    shapeInfo.orientation = ktQuaternion(0,0,0,0);
    shapeInfo.mass = 0.0f;
    shapeInfo.inertia = ktVector(0,0,0);
    
    shapeInfo.position = ktVector(0,0,0);   // floor
    shapeInfo.size = ktVector(sz,wd,sz);
    (void)ktMakeShape(shapeInfo, NULL);
    
    shapeInfo.position = ktVector(0,dis*2,0);   // ceiling
    shapeInfo.size = ktVector(sz,wd,sz);
    (void)ktMakeShape(shapeInfo, NULL);

    shapeInfo.position = ktVector(dis,0,0); // right wall
    shapeInfo.size = ktVector(wd,szh,sz);
    (void)ktMakeShape(shapeInfo, NULL);
    
    shapeInfo.position = ktVector(-dis,0,0);  // left wall
    shapeInfo.size = ktVector(wd,szh,sz);
    (void)ktMakeShape(shapeInfo, NULL);

    shapeInfo.position = ktVector(0,0,dis); // forward wall
    shapeInfo.size = ktVector(sz,szh,wd);
    (void)ktMakeShape(shapeInfo, NULL);
    
    shapeInfo.position = ktVector(0,0,-dis); // back wall
    shapeInfo.size = ktVector(sz,szh,wd);
    (void)ktMakeShape(shapeInfo, NULL);
}
