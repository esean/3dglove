
/*
 * $Copyright$
 * Copyright (c) 2015 All Rights Reserved, Sean Harre
 *
 * Sean Harre is the copyright holder of all code below.
 * Do not re-use without permission.
 */

#ifndef DRAW_HELPERS_H
#define DRAW_HELPERS_H

#include "3dtarget.h"

class draw_helpers {
    
public:
    
    draw_helpers();
    
    void draw_spheres(vector<ktShapeInfo> spheres);
    void draw_cube(vector<ktShapeInfo> cubes, ofBoxPrimitive* rbox);
    void draw_cylinders(vector<ktShapeInfo> cylinders, ofCylinderPrimitive* rcyl);
    void draw_bounding_walls(vector<ktShapeInfo> walls);
    
    void draw_hands(const HandVector::hand_vecs left, const HandVector::hand_vecs right, ofCylinderPrimitive* rcyl);
    void draw_soft_body(vector<btSoftBody*> sBody);

    bool set_debug_mode(bool newv);
    
private:
    
    void drawJoint(const HandVector::joint_vecs& jv, ofCylinderPrimitive* rcyl, float joint_force = 0.0f, float sphere_size = HAND_SPHERE_RADIUS);
    
    void drawBone(const Leap::Vector& joint_position1,const Leap::Vector& joint_position2);
    
    bool debug_mode;
    
};
#endif
