
/*
 * $Copyright$
 * Copyright (c) 2015 All Rights Reserved, Sean Harre
 *
 * Sean Harre is the copyright holder of all code below.
 * Do not re-use without permission.
 */

#include "draw_helpers.h"

draw_helpers::draw_helpers() :
    debug_mode(false)
{
}

//--------------------------
bool draw_helpers::set_debug_mode(bool newv)
{
    bool was = debug_mode;
    debug_mode = newv;
    return was;
}

//--------------------------
void draw_helpers::draw_spheres(vector<ktShapeInfo> spheres)
{
    for(std::vector<ktShapeInfo*>::size_type i = 0; i != spheres.size(); i++) {
        ktVector pos = spheres[i].position;
        ofDrawSphere(pos.x,pos.y,pos.z,spheres[i].size.x);
    }
}

//--------------------------
void draw_helpers::draw_cube(vector<ktShapeInfo> cubes, ofBoxPrimitive* rbox)
{
    for(std::vector<ktShapeInfo*>::size_type i = 0; i != cubes.size(); i++) {
        ktVector pos = cubes[i].position;
        ktVector halfExtent = cubes[i].size;
        ktQuaternion quat = cubes[i].orientation;
        
        rbox->set(halfExtent.x, halfExtent.y, halfExtent.z);
        rbox->setPosition(pos.x, pos.y, pos.z);
        rbox->setOrientation(ofQuaternion(quat.x,quat.y,quat.z,quat.w));
        rbox->draw();
    }
}

//--------------------------
void draw_helpers::draw_cylinders(vector<ktShapeInfo> cylinders, ofCylinderPrimitive* rcyl)
{
    for(std::vector<ktShapeInfo*>::size_type i = 0; i != cylinders.size(); i++) {
        ktVector pos = cylinders[i].position;
        ktVector halfExtent = cylinders[i].size;
        ktQuaternion quat = cylinders[i].orientation;
        
        rcyl->set(halfExtent.x, halfExtent.y, halfExtent.z);
        rcyl->setPosition(pos.x, pos.y, pos.z);
        rcyl->setOrientation(ofQuaternion(quat.x,quat.y,quat.z,quat.w));
        rcyl->draw();
    }
}

//--------------------------
void draw_helpers::draw_bounding_walls(vector<ktShapeInfo> walls)
{
    for(std::vector<ktShapeInfo*>::size_type i = 0; i != walls.size(); i++) {
        ktVector pos = walls[i].position;
        ktVector halfExtent = walls[i].size;

        ofDrawBox(pos.x, pos.y, pos.z, halfExtent.x, halfExtent.y, halfExtent.z);
    }
}

//--------------------------
void draw_helpers::draw_hands(const HandVector::hand_vecs left, const HandVector::hand_vecs right, ofCylinderPrimitive* rcyl)
{    
    glPushMatrix();
    
    if (left.is_valid)
    {
        // draw palm location
        //drawJoint(left.palm_location.pos);
        
        for (int f = 0; f < MAX_FINGERS; ++f) {
            
            // Draw first joint inside hand.
            drawJoint(left.fingers[f].joints[0], rcyl);
            
            for (int j = 1; j < MAX_JOINTS; ++j) {
                
                // is_this_special_thumb_joint(f,j)
                if ( ! ((f==0) && (j==1)) ) {
                    
                    // Draw first joint inside hand.
                    drawJoint(left.fingers[f].joints[j], rcyl);
                    #if CFG_DRAW_HAND_BONES == 0    // tbd - bug if drawing cylinders, drawBone makes thumb no cylinders...
                    drawBone(left.fingers[f].joints[j].pos,left.fingers[f].joints[j-1].pos);
                    #endif
                }
            }
        }
    }
    
    if (right.is_valid)
    {
        // draw palm location
        //drawJoint(right.palm_location.pos);
        
        for (int f = 0; f < MAX_FINGERS; ++f) {
            
            // Draw first joint inside hand.
            drawJoint(right.fingers[f].joints[0], rcyl);
            
            for (int j = 1; j < MAX_JOINTS; ++j) {
                
                // is_this_special_thumb_joint(f,j)
                if ( ! ((f==0) && (j==1)) ) {
                    
                    // Draw first joint inside hand.
                    drawJoint(right.fingers[f].joints[j], rcyl);
                    #if CFG_DRAW_HAND_BONES == 0    // tbd - bug if drawing cylinders, drawBone makes thumb no cylinders...
                    drawBone(right.fingers[f].joints[j].pos,right.fingers[f].joints[j-1].pos);
                    #endif
                }
            }
        }
    }
    
    glPopMatrix();
}

//--------------------------
void draw_helpers::drawJoint(const HandVector::joint_vecs& jv, ofCylinderPrimitive* rcyl, float joint_force, float sphere_size)
{
    ktVector pos = ktVector(jv.pos.x, jv.pos.y, jv.pos.z);
    
#if CFG_DRAW_HAND_BONES
    
    ktQuaternion quat = jv.orientation;
    
    dbprintf("BONE[%d-%d:%d] length(%f) width(%f) pos(%f,%f,%f) quat(%f,%f,%f,%f)\n",
             jv.idx.is_left,jv.idx.hand_finger,jv.idx.hand_joint,
             jv.length, jv.width,
             pos.x, pos.y, pos.z,
             quat.x,quat.y,quat.z,quat.w);
    // cylinder(thickness,height,n/a)
    rcyl->set(jv.width/3, jv.length, jv.length);
    rcyl->setPosition(pos.x, pos.y, pos.z);
    rcyl->setOrientation(ofQuaternion(quat.x,quat.y,quat.z,quat.w));
    rcyl->draw();
    
#else
    
    if ( (joint_force > 0) && debug_mode )
    {
        ofPushStyle();
        ofSetColor(255,0,0);
        ofFill();
        ofDrawSphere(pos.x, pos.y, pos.z, sphere_size);
        ofPopStyle();
    }
    else
        ofDrawSphere(pos.x, pos.y, pos.z, sphere_size);
    
#endif
}

//--------------------------
void draw_helpers::drawBone(const Leap::Vector& nx,const Leap::Vector& px) {
    ofPushStyle();
    ofSetLineWidth(HAND_BONE_WIDTH);
    ofLine(ofVec3f(nx.x,nx.y,nx.z),ofVec3f(px.x,px.y,px.z));
    ofPopStyle();
}

//--------------------------
void draw_helpers::draw_soft_body(vector<btSoftBody*> sBodyxx)
{
    for(std::vector<btSoftBody*>::size_type i=0; i != sBodyxx.size(); i++)
    {
        btSoftBody* sBody = sBodyxx[i];
        
        if(sBody == NULL)
            continue;
        
        int numFaces = sBody->m_faces.size();

        for (int i=0; i< numFaces; i++)
        {
            btSoftBody::Node*   node_0=sBody->m_faces[i].m_n[0];
            btSoftBody::Node*    node_1=sBody->m_faces[i].m_n[1];
            btSoftBody::Node*   node_2=sBody->m_faces[i].m_n[2];
            
            
            btVector3 p0;
            p0 = node_0->m_x;
            btVector3 p1;
            p1 = node_1->m_x;
            btVector3 p2;
            p2 = node_2->m_x;

            
            ofBeginShape();
            ofVertex(p0.x(),p0.y(),p0.z());
            ofVertex(p1.x(),p1.y(),p1.z());
            ofVertex(p2.x(),p2.y(),p2.z());
            ofEndShape();
            
        }
        
        int numTetras = sBody->m_tetras.size();
        
        for (int i=0; i< numTetras; i++)
        {
            btSoftBody::Node*   node_0=sBody->m_tetras[i].m_n[0];
            btSoftBody::Node*    node_1=sBody->m_tetras[i].m_n[1];
            btSoftBody::Node*   node_2=sBody->m_tetras[i].m_n[2];
            btSoftBody::Node*   node_3=sBody->m_tetras[i].m_n[3];
            
            btVector3 p0;
            p0 = node_0->m_x;
            btVector3 p1;
            p1 = node_1->m_x;
            btVector3 p2;
            p2 = node_2->m_x;
            btVector3 p3;
            p3 = node_3->m_x;
            
            //glNormal3fv(normal);
            ofBeginShape();
            ofVertex(p0.x(),p0.y(),p0.z());
            ofVertex(p1.x(),p1.y(),p1.z());
            ofVertex(p2.x(),p2.y(),p2.z());
            ofVertex(p3.x(),p3.y(),p3.z());
            ofEndShape();
            
        }

        
        //Just draw the nodes
    //    for (int i=0; i< sBody->m_nodes.size(); i++){
    //        btVector3 p0;
    //        p0 = sBody->m_nodes[i].m_x;
    //        ofSphere(p0.x(),p0.y(),p0.z(),5);
    //
    //    }
        
    }
}
