

/*
 * $Copyright$
 * Copyright (c) 2015 All Rights Reserved, Sean Harre
 *
 * Sean Harre is the copyright holder of all code below.
 * Do not re-use without permission.
 */

#include "ofApp.h"
#include <string>

// collection of collision points reported from API on objects
// we have created since last ofApp draw frame
static vector<ktCollisionInfo> collided_objs;

// TODO:
//    - make sphere/object an ofNode()
//    - rm collision_force&depth once no contact - filter/fade out?
//
// gContactBreakingThreshold ?
// btGjkEpaPenetrationDepthSolver ?
//
// - can ofxUI do text updates?
//
//--------------------------------------------------------------

void ofApp::setup(){
    
    printf("Starting 3D.G(kt%s_of%s)\n",KINATOUCH_VERSION,APP_VERSION);
    
    // oF
    //ofSetEscapeQuitsApp(false);
    ofSetDataPathRoot("../Resources/data/");
    ofSetFrameRate(OF_MAX_FRAME_RATE);
    ofSetVerticalSync(true);
    ofBackground(25,25,25);
    //ofBackgroundGradient(ofColor::white, ofColor::gray);
    ofEnableDepthTest();
    glEnable(GL_DEPTH_TEST);
    
    // oF camera
    camtarget = CAM_TARGET;
    cam.setTarget(camtarget);
    campos = CAM_POSITION;
    cam.setPosition(campos);
    orig_view = cam.getTarget();
    cam_pos_track_hand = false;  // position tracks hand
    
    // box
    ofDisableArbTex();
    box_texture.loadImage("mc_tnt.png");
    wall_texture.loadImage("mc_sandstone.png");
    
    // lighting
    // turn on smooth lighting //
    ofSetSmoothLighting(true);
    // lets make a sphere with more resolution than the default //
    // default is 20 //
    ofSetSphereResolution(32);
    
    // Point lights emit light in all directions //
    pointLight.setPointLight();
    // set the diffuse color, color reflected from the light source //
    pointLight.setDiffuseColor( ofColor(250.f, 155.f, 155.f)); //ofColor(50.f, 255.f, 255.f));
    // specular color, the highlight/shininess color //
	pointLight.setSpecularColor( ofColor(255.f, 255.f, 55.f));
    pointLight.setPosition(ofPoint(100.0f,100.0f,200.0f));    //ofPoint(100.0f,100.0f,0.0f));

    // shininess is a value between 0 - 128, 128 being the most shiny //
	material.setShininess( 127 );
    
    // profiling
    offrdiv = DEFAULT_STAT_UPDATE_TIMES_PER_SEC;
    
    // gui
    setGUI();
    gui->setVisible(false);
    setGUIL();
    guiL->setVisible(false);
    stats_active = true;
    
    // bullet update rate
    bUpHz = STEP_WORLD_UPDATE_HZ;
    
    // start KinaTouch library and init objects in that world
    ktLibInit();
    dis=DIS;
    sz=SZ;
    szh=SZH;
    wd=WD;
    draw_initial_physics_scene();
}

//--------------------------------------------------------------
void ofApp::draw_initial_physics_scene()
{
    #if CFG_DRAW_BOUNDING_WALLS
    ktMakeBoundingWalls(dis,sz,szh,wd);
    #endif
}

//--------------------------------------------------------------
void ofApp::setGUI(){
    
    gui = new ofxUISuperCanvas("MainThread");
    
    vector<float> buffer;
    for(int i = 0; i < MOVGRAPH_DEPTH; i++)
        buffer.push_back(0.0);
    
    stringstream x;
    x.str(std::string()); x << "OF_API FPS [" << DEFAULT_MOVWINDOW_MAXOF_UPDATE_HZ << "Hz]";
    gui->addSpacer();
    gui->addLabel(x.str(), GUI_TEXT_SIZE);
    offps = gui->addMovingGraph("OF_FPS", buffer, MOVGRAPH_DEPTH, 0.0, DEFAULT_MOVWINDOW_MAXOF_UPDATE_HZ);
    
    x.str(std::string()); x << "MainFps [" << DEFAULT_MOVWINDOW_MAX_UPDATE_HZ << "Hz]";
    gui->addSpacer();
    gui->addLabel(x.str(), GUI_TEXT_SIZE);
    thread_update_hz = gui->addMovingGraph("MAINT_FPS", buffer, MOVGRAPH_DEPTH, 0.0, DEFAULT_MOVWINDOW_MAX_UPDATE_HZ);
    
    x.str(std::string()); x << "Thread:avg & recent [" << DEFAULT_MOVWINDOW_MAX_TIME_US << "us]";
    gui->addSpacer();
    gui->addLabel(x.str(), GUI_TEXT_SIZE);
    thread_avg = gui->addMovingGraph("Thread_AVG", buffer, MOVGRAPH_DEPTH, 0.0, DEFAULT_MOVWINDOW_MAX_TIME_US);
    //thread_recent = gui->addMovingGraph("Thread_RECENT", buffer, MOVGRAPH_DEPTH, 0.0, DEFAULT_MOVWINDOW_MAX_TIME_US);
    
    x.str(std::string()); x << "FTDI:avg & recent [" << DEFAULT_MOVWINDOW_MAX_TIME_US << "us]";
    gui->addSpacer();
    gui->addLabel(x.str(), GUI_TEXT_SIZE);
    ftdi_avg = gui->addMovingGraph("FTDI_AVG", buffer, MOVGRAPH_DEPTH, 0.0, DEFAULT_MOVWINDOW_MAX_TIME_US);
    //ftdi_recent = gui->addMovingGraph("FTDI_RECENT", buffer, MOVGRAPH_DEPTH, 0.0, DEFAULT_MOVWINDOW_MAX_TIME_US);

    x.str(std::string()); x << "Sound:avg & recent [" << DEFAULT_MOVWINDOW_MAX_TIME_US << "us]";
    gui->addSpacer();
    gui->addLabel(x.str(), GUI_TEXT_SIZE);
    sound_avg = gui->addMovingGraph("Sound_AVG", buffer, MOVGRAPH_DEPTH, 0.0, DEFAULT_MOVWINDOW_MAX_TIME_US);
    //sound_recent = gui->addMovingGraph("Sound_RECENT", buffer, MOVGRAPH_DEPTH, 0.0, DEFAULT_MOVWINDOW_MAX_TIME_US);
    
    x.str(std::string()); x << "Network:avg & recent [" << DEFAULT_MOVWINDOW_MAX_TIME_US << "us]";
    gui->addSpacer();
    gui->addLabel(x.str(), GUI_TEXT_SIZE);
    networker_recent = gui->addMovingGraph("Network_AVG", buffer, MOVGRAPH_DEPTH, 0.0, DEFAULT_MOVWINDOW_MAX_TIME_US);
    //nw_recent = gui->addMovingGraph("Network_RECENT", buffer, MOVGRAPH_DEPTH, 0.0, DEFAULT_MOVWINDOW_MAX_TIME_US);
    
    gui->setPosition(ofGetWidth()-GUI_WIDTH, 1);
    gui->autoSizeToFitWidgets();
}

//--------------------------------------------------------------
void ofApp::setGUIL(){
    
    guiL = new ofxUISuperCanvas("LeapThread");
    
    vector<float> buffer;
    for(int i = 0; i < MOVGRAPH_DEPTH; i++)
        buffer.push_back(0.0);
    
    stringstream x;
    x.str(std::string()); x << "Leap_API FPS [" << DEFAULT_MOVWINDOW_MAX_UPDATE_HZ << "Hz]";
    guiL->addSpacer();
    guiL->addLabel(x.str(), GUI_TEXT_SIZE);
    leapfps = guiL->addMovingGraph("LEAP_FPS", buffer, MOVGRAPH_DEPTH, 0.0, DEFAULT_MOVWINDOW_MAX_UPDATE_HZ);
  
    x.str(std::string()); x << "Leap FPS [" << DEFAULT_MOVWINDOW_MAX_UPDATE_HZ << "Hz]";
    guiL->addSpacer();
    guiL->addLabel(x.str(), GUI_TEXT_SIZE);
    leapT_update_hz = guiL->addMovingGraph("LEAPT_FPS", buffer, MOVGRAPH_DEPTH, 0.0, DEFAULT_MOVWINDOW_MAX_UPDATE_HZ);
    
    x.str(std::string()); x << "Thread:avg & recent [" << DEFAULT_MOVWINDOW_MAX_TIME_US << "us]";
    guiL->addSpacer();
    guiL->addLabel(x.str(), GUI_TEXT_SIZE);
    leapT_avg = guiL->addMovingGraph("LeapT_AVG", buffer, MOVGRAPH_DEPTH, 0.0, DEFAULT_MOVWINDOW_MAX_TIME_US);
    //leapT_recent = guiL->addMovingGraph("LeapT_RECENT", buffer, MOVGRAPH_DEPTH, 0.0, DEFAULT_MOVWINDOW_MAX_TIME_US);
    
    x.str(std::string()); x << "Physics:avg & recent [" << DEFAULT_MOVWINDOW_MAX_TIME_US << "us]";
    guiL->addSpacer();
    guiL->addLabel(x.str(), GUI_TEXT_SIZE);
    physics_avg = guiL->addMovingGraph("Bullet_AVG", buffer, MOVGRAPH_DEPTH, 0.0, DEFAULT_MOVWINDOW_MAX_TIME_US);
    //physics_recent = guiL->addMovingGraph("Bullet_RECENT", buffer, MOVGRAPH_DEPTH, 0.0, DEFAULT_MOVWINDOW_MAX_TIME_US);
    
    guiL->setPosition(ofGetWidth()-GUI_WIDTH*2, 1);
    guiL->autoSizeToFitWidgets();
}

//--------------------------------------------------------------
void ofApp::update(){
    
    if (bInfoText && stats_active)
    {
        update_stats();
    
        // main thread
        offps->addPoint(ofGetFrameRate());
        thread_update_hz->addPoint(1.0f/(ps_thread.update_avg/1000000.0f));
        thread_avg->addPoint(ps_thread.avg);
        //thread_recent->addPoint(ps_thread.recent);
        ftdi_avg->addPoint(ps_ftdi.avg);
        //ftdi_recent->addPoint(ps_ftdi.recent);
        //networker_recent->addPoint(ps.nwprof.recent);
        networker_recent->addPoint(ps_networker.avg);
        
        // leap thread
        leapfps->addPoint(getTW().leap_frame_rate);
        leapT_update_hz->addPoint(1.0f/(ps_leapT.update_avg/1000000.0f));
        leapT_avg->addPoint(ps_leapT.avg);
        //leapT_recent->addPoint(ps_leapT.recent);
        physics_avg->addPoint(ps_physics.avg);
        //physics_recent->addPoint(ps_physics.recent);
        sound_avg->addPoint(ps_snd.avg);
        //sound_recent->addPoint(ps_snd.recent);
    }
}

//--------------------------------------------------------------
void ofApp::update_stats(){
    
    static int update_cnt = 1000;   // force an update on first pass
    
    if (update_cnt >= (ofGetFrameRate()/offrdiv))
    {
        update_cnt = 0;
        
        // update all stats
        ps_thread = getTW().twprof.get_stats();
        ps_physics = getTW().btProf.get_stats();
        ps_ftdi = getTW().ftProf.get_stats();
        ps_snd = getTW().sndProf.get_stats();
        ps_leapT = getTW().leapTProf.get_stats();
        ps_physics_cb = getTW().btCbProf.get_stats();
        ps_networker = getTW().nwprof.get_stats();
    }
    else
    {
        ++update_cnt;
    }
}

//--------------------------
// filter:
//  y = x0*m + x1*(1-m)
//
ofVec3f ofApp::flt_avg(const ofVec3f newv, const ofVec3f oldv, const ofVec3f coeff)
{
    return (oldv * coeff) + (newv * (1.0f-coeff));
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    stringstream ss;
    ss << "3D.G(kt" << KINATOUCH_VERSION << "_of" << APP_VERSION << ") FR: " << ofToString(ofGetFrameRate(),0) << "hz, Leap_API FR: "
        << ofToString(getTW().leap_frame_rate,0) << "hz, ('h' for help)" << endl;
    
    // enable lighting
    ofEnableLighting();
    material.begin();
    
    // orient camera towards our hands
    HandVector::hand_vecs righthand = ktGet_right_hand_vector();
    HandVector::hand_vecs lefthand = ktGet_left_hand_vector();
    //        HandVector::joint_vecs t = lefthand.fingers[0].joints[0];
    //        printf("D:POS(%f,%f,%f) W=%f L=%f QUAT=(%f,%f,%f,%f)\n",t.pos.x, t.pos.y, t.pos.z, t.width, t.length,
    //               t.orientation.x, t.orientation.y, t.orientation.z, t.orientation.w );
    
    ofVec3f newv;
    if (righthand.is_valid || lefthand.is_valid)
    {
        if (righthand.is_valid && lefthand.is_valid)
        {
            ofVec3f newvr = ofVec3f(righthand.fingers[0].joints[0].pos.x,
                                righthand.fingers[0].joints[0].pos.y,
                                righthand.fingers[0].joints[0].pos.z)* CAM_SCALE;
            ofVec3f newvl = ofVec3f(lefthand.fingers[0].joints[0].pos.x,
                                    lefthand.fingers[0].joints[0].pos.y,
                                    lefthand.fingers[0].joints[0].pos.z)* CAM_SCALE;
            newv = (newvr + newvl) / 2.0f;
        }
        else if (righthand.is_valid)
        {
            newv = ofVec3f(righthand.fingers[0].joints[0].pos.x,
                           righthand.fingers[0].joints[0].pos.y,
                           righthand.fingers[0].joints[0].pos.z)* CAM_SCALE;
        }
        else if (lefthand.is_valid)
        {
            newv = ofVec3f(lefthand.fingers[0].joints[0].pos.x,
                           lefthand.fingers[0].joints[0].pos.y,
                           lefthand.fingers[0].joints[0].pos.z)* CAM_SCALE;
        }
        camtarget = flt_avg(newv, camtarget, TRACK_HAND_COEFF_VALID);
    }
    else
    {
        newv = ofVec3f(-100,1500,100);
        camtarget = flt_avg(newv, camtarget, TRACK_HAND_COEFF_NOTVALID);
    }
    cam.setTarget(camtarget);
    
    if (cam_pos_track_hand)
    {
        campos = flt_avg(newv, campos, TRACK_HAND_COEFF_NOTVALID);
    }
    cam.setPosition(campos);
    
    cam.begin();
    {
        pointLight.enable();
        ofScale(CAM_SCALE,CAM_SCALE,CAM_SCALE);
        
        // debug
        if (bInfoText) {
            
            ss << "LeapThread("<< ofToString((1.0f/(ps_leapT.update_avg/1000000.0f)),0) << "hz): avg = "
                << ofToString(ps_leapT.avg,0) << "us" << endl;
            ss << "+Physics(cb=" << ofToString((1.0f/(ps_physics_cb.update_avg/1000000.0f)),0) << "hz): avg = "
                << ofToString(ps_physics.avg,0) << "us" << endl;
            
            ss << "MainThread(" << ofToString((1.0f/(ps_thread.update_avg/1000000.0f)),0) << "hz): avg = "
                << ofToString(ps_thread.avg,0) << "us" << endl;
            ss << "+FTDI: avg = " << ofToString(ps_ftdi.avg,0) << "us" << endl;
            ss << "+Sound: avg = " << ofToString(ps_snd.avg,0) << "us" << endl;
            ss << "+Networker: avg = " << ofToString(ps_networker.avg,0) << "us" << endl;
            ss << endl;
            
            ss << "(f): Toggle Fullscreen" << endl;
            ss << "(s): Reset physics and networker thread" << endl;
            ss << "(v): Reset view" << endl;
            ss << "(g): camera position tracks hand: " << (cam_pos_track_hand?"TRUE":"FALSE") << endl;
            ss << (getTW().get_leap_filtering()?"DISABLE ":"ENABLE ") << "leap fil(t)ering" << endl;
            ss << "(r)/(l): Test right/left glove" << endl;
            ss << "(a): step thru joints" << endl;
            ss << "(n)faster/(m)slower: stat update rate = " << ofToString(offrdiv,0) << endl;
            ss << "(p)ause stats: " << (stats_active? "ENABLED" : "DISABLED") << endl;
            ss << "(b) dec OR (B) inc bullet update rate: " << ofToString(bUpHz,0) << "hz" << endl;
            ss << "(0) to draw cylinder on screen" << endl;
            ss << "(-) to draw cube on screen" << endl;
            ss << "(=) to draw sphere on screen" << endl;
            ss << "(d) delete last created shape" << endl;
            ss << "(1/2)dis=" << ofToString(dis) << ", (3/4)sz=" << ofToString(sz) << ", (5/6)szh=" << ofToString(szh) << ", (7/8)wd=" << ofToString(wd) << endl;
        
            ss << "    campos(" << campos.x << "," << campos.y << "," << campos.z << ")" << endl;
            ss << "    camtarget(" << camtarget.x << "," << camtarget.y << "," << camtarget.z << ")" << endl;
            
            //ofDrawGrid(1000);
            getTW().debug_hands();
        }
        
        // draw physics objects
        wall_texture.getTextureReference().bind();
        dh.draw_bounding_walls(ktCurrentInformationOfType(KT_BOUNDING_WALL));
        boxie.mapTexCoordsFromTexture( wall_texture.getTextureReference() );
        wall_texture.getTextureReference().unbind();
        
        {
            ofPushStyle();
            ofSetColor(100, 50, 10);
            dh.draw_spheres(ktCurrentInformationOfType(KT_SPHERE));
            ofSetColor(200, 50, 100);
            dh.draw_cylinders(ktCurrentInformationOfType(KT_CYLINDER),&cylinder);
            ofPopStyle();
        }
        
        dh.draw_hands(ktGet_left_hand_vector(),ktGet_right_hand_vector(),&cylinder);
        dh.draw_soft_body(getBullet().get_soft_body());

        // cube
        box_texture.getTextureReference().bind();
        dh.draw_cube(ktCurrentInformationOfType(KT_CUBE),&boxie);
        boxie.mapTexCoordsFromTexture( box_texture.getTextureReference() );
        box_texture.getTextureReference().unbind();
        
        // draw collision points since last update
        if (collided_objs.size() > 0)
        {
            #if CFG_DRAW_COLLISIONS
            for(std::vector<ktCollisionInfo>::size_type i = 0; i != collided_objs.size(); i++)
            {
#if 0   // this would just draw a sphere on the shape instead of the actual collision point
                // get info about the shape that collided
                ktShapeInfo curr = ktCurrentInformation(collided_objs[i].whichShape);
                float size = curr.size.x;
                ktVector pos = curr.position;
                ofDrawSphere(pos.x,pos.y,pos.z,size);
#endif

                ktCollisionInfo ci = collided_objs[i];
                ktVector impact_loc = ktVector(ci.handCollisionPoint.position.x,ci.handCollisionPoint.position.y,ci.handCollisionPoint.position.z);
//                printf("COLL shape:%d @ (%f,%f,%f) imp=(%f,%f,%f)\n",ci.shape_type,
//                       impact_loc.x,impact_loc.y,impact_loc.z,
//                       ci.impactImpulse.x,ci.impactImpulse.y,ci.impactImpulse.z);
                ofDrawSphere(impact_loc.x,impact_loc.y,impact_loc.z,30);
            }
            #endif
            
            collided_objs.clear();
        }
    }
    // end cam
    cam.end();

    // turn off lighting
    material.end();
    ofDisableLighting();
    
    // gui
    ofDrawBitmapString(ss.str().c_str(), 20, 20);
}

// this static method is called from bullet collision detection routine
// at a fast rate, so keep any work in here very fast
//--------------------------------------------------------------
// created object had a collision
void ofApp::created_shape_collision_cb(ktCollisionInfo info)
{
    collided_objs.push_back(info);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    // keeps record of which shapes we have added, we use when wanting to remove them
    //
    // tbd - add our own vector of structs here so we can maintain things we want
    //      to be drawn about that shape, color, texture, etc
    static vector<ktShapePtr>shapes;
    
    switch (key)
    {
        case '0':           // add a cylinder
        {
            ktShapeInfo shapeInfo;
            shapeInfo.type = KT_CYLINDER;
            shapeInfo.position = ktVector(20,120,-70);
            shapeInfo.orientation = ktQuaternion(0,0,0,0);
            shapeInfo.size = ktVector(10,80,10);
            shapeInfo.mass = 1.0f;
            shapeInfo.inertia = ktVector(10,10,10);
            ktShapePtr this_shape = ktMakeShape(shapeInfo, &ofApp::created_shape_collision_cb);
            shapes.push_back(this_shape);
        }
            break;
        case '-':           // add a cube
        {
            ktShapeInfo shapeInfo;
            shapeInfo.type = KT_CUBE;
            shapeInfo.position = ktVector(0,90,-120);
            shapeInfo.orientation = ktQuaternion(0,0,0,0);
            shapeInfo.size = ktVector(35,35,35);
            shapeInfo.mass = 1.0f;
            shapeInfo.inertia = ktVector(100,0,20);
            ktShapePtr this_shape = ktMakeShape(shapeInfo, &ofApp::created_shape_collision_cb);
            shapes.push_back(this_shape);
        }
            break;
        case '=':           // add a sphere
        {
            ktShapeInfo shapeInfo;
            shapeInfo.type = KT_SPHERE;
            shapeInfo.position = ktVector(20,270,20);
            shapeInfo.orientation = ktQuaternion(0,0,0,0);
            shapeInfo.size = ktVector(20,20,20);
            shapeInfo.mass = 1.0f;
            shapeInfo.inertia = ktVector(0,0,120);
            ktShapePtr this_shape = ktMakeShape(shapeInfo, &ofApp::created_shape_collision_cb);
            shapes.push_back(this_shape);
        }
            break;
        case 'd':
            if (shapes.size())
            {
                ktDeleteShape(shapes[0]);
                shapes.erase(shapes.begin());
            }
            break;
        case 'g':
            cam_pos_track_hand = !cam_pos_track_hand;
            break;
        case 'f':
            ofToggleFullscreen();
            break;
        case 's':
            ktRestart();
            draw_initial_physics_scene();
            break;
        case 'v':
            cam.setTarget(orig_view);
            campos = CAM_POSITION;
            cam.setPosition(campos);
            break;
        case 'h':
            bInfoText = !bInfoText;
            gui->setVisible(bInfoText);
            guiL->setVisible(bInfoText);
            getTW().set_debug_mode(bInfoText);
            dh.set_debug_mode(bInfoText);
            getTW().twprof.reset();
            getTW().btProf.reset();
            getTW().ftProf.reset();
            getTW().sndProf.reset();
            getTW().leapTProf.reset();
            break;
        case 't':
            getTW().set_leap_filtering( ! getTW().get_leap_filtering());
            break;
            
        // dev
        case 'a':
            if (bInfoText)
                getTW().debug_step_thru_hand_joints();
            break;
        case 'r':
            #if CFG_HAS_RIGHT_HAND
            getTW().debug_light_right_hand();
            #endif
            break;
        case 'l':
            #if CFG_HAS_LEFT_HAND
            getTW().debug_light_left_hand();
            #endif
            break;
            
        // gui
        case 'p':
            stats_active = !stats_active;
            break;
        case 'm':
            if (offrdiv>1)
                offrdiv--;
            break;
        case 'n':
            offrdiv++;
            break;
        
        case '1':
            dis -= 2;
            break;
        case '2':
            dis += 2;
            break;
        case '3':
            sz -= 2;
            break;
        case '4':
            sz += 2;
            break;
        case '5':
            szh -= 2;
            break;
        case '6':
            szh += 2;
            break;
        case '7':
            wd -= 2;
            break;
        case '8':
            wd += 2;
            break;
            
        case 'b':
            bUpHz -= 10;
            getTW().debug_set_bullet_update_hz(bUpHz);
            break;
        case 'B':
            bUpHz += 10;
            getTW().debug_set_bullet_update_hz(bUpHz);
            break;

        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::exit(){
    
    getTW().stop();
    delete gui;
    delete guiL;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

    gui->setPosition(ofGetWidth()-GUI_WIDTH, 1);
    guiL->setPosition(ofGetWidth()-GUI_WIDTH*2, 1);
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
