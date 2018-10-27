#include "GyroscopicSetup.h"

static int gyroflags[5] = {
    0,//none, no gyroscopic term
    BT_ENABLE_GYROSCOPIC_FORCE_EXPLICIT,
    BT_ENABLE_GYROSCOPIC_FORCE_IMPLICIT_EWERT,
    BT_ENABLE_GYROSCOPIC_FORCE_IMPLICIT_CATTO,
    BT_ENABLE_GYROSCOPIC_FORCE_IMPLICIT_COOPER,
};

static const char* gyroNames[5] = {
    "No Coriolis",
    "Explicit",
    "Ewert",
    "Catto",
    "Cooper",
};


void GyroscopicSetup::initPhysics(GraphicsPhysicsBridge& gfxBridge)
{
	gfxBridge.setUpAxis(2);
	createEmptyDynamicsWorld();
	m_dynamicsWorld->setGravity(btVector3(0, 0, 0));
	gfxBridge.createPhysicsDebugDrawer(m_dynamicsWorld);

	
	btVector3 positions[5] = {
		btVector3( -10, 8,4),
		btVector3( -5, 8,4),
		btVector3( 0, 8,4),
		btVector3( 5, 8,4),
		btVector3( 10, 8,4),
	};


	for (int i = 0; i<5; i++)
	{
		btCylinderShapeZ* pin = new btCylinderShapeZ(btVector3(0.1,0.1, 0.2));
		btBoxShape* box = new btBoxShape(btVector3(1,0.1,0.1));
		box->setMargin(0.01);
		pin->setMargin(0.01);
		btCompoundShape* compound = new btCompoundShape();
		compound->addChildShape(btTransform::getIdentity(), pin);
		btTransform offsetBox(btMatrix3x3::getIdentity(),btVector3(0,0,0.2));
		compound->addChildShape(offsetBox, box);
		btScalar masses[2] = {0.3,0.1};
		btVector3 localInertia;
		btTransform principal;
		compound->calculatePrincipalAxisTransform(masses,principal,localInertia);
		
		btRigidBody* body = new btRigidBody(1, 0, compound, localInertia);
		btTransform tr;
		tr.setIdentity();
		tr.setOrigin(positions[i]);
		body->setCenterOfMassTransform(tr);
		body->setAngularVelocity(btVector3(0, 0.1, 10));//51));
		//body->setLinearVelocity(btVector3(3, 0, 0));
		body->setFriction(btSqrt(1));
		m_dynamicsWorld->addRigidBody(body);
		body->setFlags(gyroflags[i]);
		m_dynamicsWorld->getSolverInfo().m_maxGyroscopicForce = 10.f;
		body->setDamping(0.0000f, 0.000f);


	}

    {
        //btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(50.),btScalar(50.),btScalar(0.5)));
        btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 0, 1), 0);
        
        m_collisionShapes.push_back(groundShape);
        btTransform groundTransform;
        groundTransform.setIdentity();
        groundTransform.setOrigin(btVector3(0, 0, 0));
        btRigidBody* groundBody;
        groundBody = createRigidBody(0, groundTransform, groundShape);
        groundBody->setFriction(btSqrt(2));
    }
	gfxBridge.autogenerateGraphicsObjects(m_dynamicsWorld);
}

void GyroscopicSetup::syncPhysicsToGraphics(GraphicsPhysicsBridge& gfxBridge)
{
    CommonRigidBodySetup::syncPhysicsToGraphics(gfxBridge);
    //render method names above objects
    for (int i=0;i<m_dynamicsWorld->getNumCollisionObjects();i++)
    {
        btRigidBody* body = btRigidBody::upcast(m_dynamicsWorld->getCollisionObjectArray()[i]);
        if (body && body->getInvMass()>0)
        {
            btTransform tr = body->getWorldTransform();
            btVector3 pos = tr.getOrigin()+btVector3(0,0,2);
            btScalar size=1;
            gfxBridge.drawText3D(gyroNames[i],pos.x(),pos.y(),pos.z(),size);
        }
    }
}
