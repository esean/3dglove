
/*
 * $Copyright$
 * Copyright (c) 2015 All Rights Reserved, Sean Harre
 *
 * Sean Harre is the copyright holder of all code below.
 * Do not re-use without permission.
 */

#include "ofMain.h"
#include "ofApp.h"
#include <iostream>
using namespace std;

//========================================================================
int main( ){

	ofSetupOpenGL(800, 600, OF_FULLSCREEN); // OF_WINDOW);			// <-------- setup the GL context

    try
    {
        // this kicks off the running of my app
        // can be OF_WINDOW or OF_FULLSCREEN
        // pass in width and height too:
        ofRunApp( new ofApp());
    }
    catch (int e)
    {
        cout << "An exception occurred. Exception Nr. " << e << '\n';
        exit(-1);
    }
    exit(0);
}
