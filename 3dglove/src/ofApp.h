
/*
 * $Copyright$
 * Copyright (c) 2015 All Rights Reserved, Sean Harre
 *
 * Sean Harre is the copyright holder of all code below.
 * Do not re-use without permission.
 */

#ifndef _OF_APP_H_
#define _OF_APP_H_

#include "3dtarget.h"       // target
#include "threadWorker.h"   // thread
#include "draw_helpers.h"
#include "ofxUI.h"

class ofApp : public ofBaseApp{
    
	public:
		void setup();
		void update();
		void draw();
        void exit();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    private:
    
        static void created_shape_collision_cb(ktCollisionInfo info);
        ofVec3f flt_avg(const ofVec3f newv, const ofVec3f oldv, const ofVec3f coeff);
    
        void draw_initial_physics_scene();
        int dis,sz,szh,wd;
    
        // camera
        ofEasyCam cam;
        ofPoint campos;   // debug
        ofPoint camtarget;  // debug
        ofNode orig_view;
        bool cam_pos_track_hand;
    
        // light
        ofLight pointLight;
        ofMaterial material;
        
        // draw helpers
        draw_helpers dh;
        
        // debug
        bool bInfoText;
      
        // bullet update rate (hz)
        btScalar bUpHz;
    
        // drawing primatives
        ofBoxPrimitive boxie;
        ofImage box_texture;
        ofImage wall_texture;
        ofCylinderPrimitive cylinder;
        ofSpherePrimitive sph;
    
        // profiling
        bool stats_active;
        void update_stats();
        int offrdiv;
        // local cache
        // these are used for gui update and printing stat text
        prof::prof_stat ps_thread;
        prof::prof_stat ps_physics;
        prof::prof_stat ps_ftdi;
        prof::prof_stat ps_snd;
        prof::prof_stat ps_leapT;
        prof::prof_stat ps_physics_cb;
        prof::prof_stat ps_networker;
    
        // gui - main thread
        void setGUI();
        ofxUISuperCanvas *gui;
        ofxUIMovingGraph *thread_update_hz;
        ofxUIMovingGraph *offps;
        ofxUIMovingGraph *thread_avg;
        ofxUIMovingGraph *thread_recent;
        ofxUIMovingGraph *ftdi_avg;
        ofxUIMovingGraph *ftdi_recent;
        ofxUIMovingGraph *networker_recent;
    
        // gui - leap thread
        void setGUIL();
        ofxUISuperCanvas *guiL;
        ofxUIMovingGraph *leapT_update_hz;
        ofxUIMovingGraph *leapfps;
        ofxUIMovingGraph *physics_avg;
        ofxUIMovingGraph *physics_recent;
        ofxUIMovingGraph *sound_avg;
        ofxUIMovingGraph *sound_recent;
        ofxUIMovingGraph *leapT_avg;
        ofxUIMovingGraph *leapT_recent;
    
};

#endif  // _OF_APP_H_
