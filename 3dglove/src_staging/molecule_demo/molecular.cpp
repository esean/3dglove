#include <iostream>
#include <fstream>
#include <vector>
#include <btBulletDynamicsCommon.h>
#define MAXAA 50
using namespace std;

class Molecular {

	private:
		int i;
	public:
		
// Collision Detection Algorithms and Configuration 
btBroadphaseInterface* broadphase;
btDefaultCollisionConfiguration* collisionConfiguration;
btCollisionDispatcher* dispatcher;

// Physics Solver
btSequentialImpulseConstraintSolver* solver;

// The world.
btDiscreteDynamicsWorld* dynamicsWorld;


};



class Gromacs {
    
private:
    int i;
    
public:
    
    float x1,y1,z1; /*Centroid*/
    float cumsum;
    float VEC1[MAXAA],VEC2[MAXAA],VEC3[MAXAA];
    
    
    // Calculate Weighted Average
    void Weighted_Ave(int Natoms,float O1X[MAXAA]) {
        for(int i=0;i<Natoms;i++){
            cumsum+=O1X[i];
        }
        cumsum/=Natoms;
    }
    
    // Calculate Center of Mass
    void COM(int Natoms, float O1X[MAXAA],float O1Y[MAXAA],float O1Z[MAXAA]){
        x1=y1=z1=0.0;
        for(i=0;i<Natoms;i++){
            x1+=O1X[i];
            y1+=O1Y[i];
            z1+=O1Z[i];
        }
        
        x1/=Natoms;
        y1/=Natoms;
        z1/=Natoms;
        for(i=0;i<Natoms;i++){
            VEC1[i] = O1X[i]-x1;
            VEC2[i] = O1Y[i]-y1;
            VEC3[i] = O1Z[i]-z1;
        }
        
    }
    
};


int main() {
    
    //Physics Engine
    Molecular dynamics;
    dynamics.broadphase = new btDbvtBroadphase();
    dynamics.collisionConfiguration = new btDefaultCollisionConfiguration();
    dynamics.dispatcher = new btCollisionDispatcher(dynamics.collisionConfiguration);
    dynamics.solver = new btSequentialImpulseConstraintSolver;
    dynamics.dynamicsWorld = new btDiscreteDynamicsWorld(dynamics.dispatcher,dynamics.broadphase,dynamics.solver, dynamics.collisionConfiguration);
    dynamics.dynamicsWorld->setGravity(btVector3(0, -10, 0));
    
    
    //Ground
    btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
    btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
    btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
    dynamics.dynamicsWorld->addRigidBody(groundRigidBody);
    
    
    //Atom Import
    string line;
    float O1X[MAXAA],O1Y[MAXAA],O1Z[MAXAA]; /*Atom Cartesian Coordinates*/
    float x1,y1,z1; /*Centroid*/
    char Atom_type[2],ResID[8];
    int Natoms=26;
    int AtomID;
    ifstream myfile("trp.gro");
    if (myfile.is_open())
    {
        
        int i = 0;
        getline (myfile,line);
        getline (myfile,line);
        while ( i<Natoms)
        {
            myfile >> ResID >> Atom_type >> AtomID >> O1X[i] >> O1Y[i] >> O1Z[i];
            i++;
        }
        myfile.close();
    }
    
    //Draw Atoms
    btScalar mass = 1.0;
    btVector3 fallInertia(0, 0, 0);
    Gromacs mac;
    mac.COM(Natoms, O1X, O1Y, O1Z);
    btTransform t;  //position and rotation
    t.setIdentity();
    
    btCompoundShape * molecule = new btCompoundShape(); // Container for Child Shapes
    //Sets the Number of Spheres in btCompoundShape
    for (int i=0;i<Natoms;i++){
         t.setOrigin(btVector3(O1X[i],O1Y[i],O1Z[i]));  //put it to x,y,z coordinates
         btCollisionShape* atom = new btSphereShape(1); // Sphere
         molecule->addChildShape(t,atom);
    }
    
    //Draws all the atoms in initial Bullet Scene
    for(int i =0;i<Natoms;i++){
        btCollisionShape* fallShape = molecule->getChildShape(i);
        fallShape->calculateLocalInertia(mass, fallInertia);
        btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 50, 0)));
        btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, fallShape, fallInertia);
        btRigidBody* fallRigidBody = new btRigidBody(fallRigidBodyCI);
        dynamics.dynamicsWorld->addRigidBody(fallRigidBody);
    }
    
    /*
    // Now setup the constraints
    btVector3 pivotInA(0,50,0);
    btTypedConstraint* p2p = new btPoint2PointConstraint(*fallRigidBody1,pivotInA);
    dynamicsWorld->addConstraint(p2p);
    */
    
    
    

    // Simulate In Bullet World
    // Physics World Simulation
    for (int i = 0; i < 300; i++) {
        dynamics.dynamicsWorld->stepSimulation(1 / 60.f, 10);
        for(int j=0; j<Natoms;j++){
                btTransform trans = molecule->getChildTransform(j);
                std::cout << "sphere height: " << trans.getOrigin().getY() << std::endl;
        }
        
         printf("next iteration\n");
    }
    
    
    
}
