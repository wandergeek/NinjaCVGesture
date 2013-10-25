#pragma once

#include "ofMain.h"
#include "ofxOpticalFlowLK.h"
#include "ofxGui.h"
#include "ninjaConstants.h"

class testApp : public ofBaseApp{
	
	public:
		
		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
        void setFlowBlur(int & size);
        void setFlowSize(int & size);
        int getGesture();
        void calcAveFlow();
        void drawGestureBar();
        void setupGUI();
    
		ofVideoGrabber 		vidGrabber;
		unsigned char * 	videoInverted;
		ofTexture			videoTexture;
		int 				camWidth;
		int 				camHeight;
    ofPoint             aveFlow;
    ofPoint             aveFlowEased;
    ofImage             gradBar;
    ofImage             handIcon;
    
    
    ofxOpticalFlowLK flow;
    ofxPanel gui;
    ofParameter<int> optiFlowSize;
    ofParameter<int> lineScale;
    ofParameter<int> drawRes;
    ofParameter<int> numAveFrames;
    ofParameter<int> optiFlowBlur;
    ofParameter<int> flowGridRes;
    ofParameter<float> gestureThresh;
    ofParameter<float> gestureTimeOut;

    float gestureStartTime;
     bool bDebug;
    bool lastGestureComplete;
    
};
