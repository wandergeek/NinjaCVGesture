#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	
	camWidth 		= 320;	// try to grab at this size. 
	camHeight 		= 240;
    bDebug = false;
    lastGestureComplete = true;
    gestureStartTime = 0;
    
    //we can now get back a list of devices. 
	vector<ofVideoDevice> devices = vidGrabber.listDevices();
	
    for(int i = 0; i < devices.size(); i++){
		cout << devices[i].id << ": " << devices[i].deviceName; 
        if( devices[i].bAvailable ){
            cout << endl;
        }else{
            cout << " - unavailable " << endl; 
        }
	}
    
	vidGrabber.setDeviceID(1);
	vidGrabber.setDesiredFrameRate(60);
	vidGrabber.initGrabber(camWidth,camHeight);

	ofSetVerticalSync(true);
    flow.setup(camWidth,camHeight);

    setupGUI();
    
    gradBar.loadImage("gradBar.png");
    handIcon.loadImage("openhand.png");
}


//--------------------------------------------------------------
void testApp::setupGUI() {
    
    int maxGridLen = camHeight/MIN_GRID_LEN;
    
    gui.setup();
    optiFlowSize.addListener(this, &testApp::setFlowSize);
    optiFlowBlur.addListener(this, &testApp::setFlowBlur);
    optiFlowBlur.addListener(this, &testApp::setFlowBlur);
    gui.add(optiFlowSize.set("Opti Flow Size", 10, 1, 50));
    gui.add(optiFlowBlur.set("Opti Flow Blur", 10, 1, 50));
    gui.add(lineScale.set("Line Scale", 5, 1, 50));
    gui.add(drawRes.set("Draw Res", 10, 1, 50));
    gui.add(numAveFrames.set("Number Average Frames", 3, 1, 60));
    gui.add(flowGridRes.set("Grid Size", 10, MIN_GRID_LEN, maxGridLen));
    gui.add(gestureThresh.set("Gesture Threshold", 1.5, GESTURE_THRESH_MIN, GESTURE_THRESH_MAX));
    gui.add(gestureTimeOut.set("Gesture Time Out", 0.5, 0, 2));
    
    
}
//--------------------------------------------------------------

void testApp::setFlowSize(int & size) {
    flow.setOpticalFlowSize(size);
}
//--------------------------------------------------------------


void testApp::setFlowBlur(int & size) {
    flow.setOpticalFlowSize(size);
}

//--------------------------------------------------------------
int testApp::getGesture() {
    int ret = NO_GESTURE;
    float now = ofGetElapsedTimef();
    lastGestureComplete = (now - gestureStartTime) >= gestureTimeOut.get();
    
    if(aveFlowEased.length() > gestureThresh.get() && lastGestureComplete) {
        cout << "hi" << endl;
        gestureStartTime = now;
        if(aveFlowEased.x > 0) {
            ofDrawBitmapString("Gesture Right Detected", ofGetWidth()*0.7,ofGetHeight()*0.1);
            ret = GESTURE_RIGHT;
        } else if(aveFlowEased.x < 0) {
            ofDrawBitmapString("Gesture Left Detected", ofGetWidth()*0.7,ofGetHeight()*0.1);
            ret = GESTURE_LEFT;
        }
    }
    
    return ret;
}


//--------------------------------------------------------------

void testApp::calcAveFlow() {
    int gridEdgeLen = flowGridRes.get();
    int numX = camWidth/gridEdgeLen;
    int numY = camHeight/gridEdgeLen;
    ofPoint ave;
    int numFlows = 0;
    
    for(int i=0; i<numX; i++) {
        for(int j=0; j<numY; j++) {
            ofPoint flowVec = flow.getVelAtPixel(i*gridEdgeLen, j*gridEdgeLen);
            ave+=flowVec;
            numFlows++;
        }
    }
    ave/=numFlows;
    
    aveFlow = ave;
    
    aveFlowEased += (aveFlow - aveFlowEased) * 0.1;
}

//--------------------------------------------------------------
void testApp::update(){
	
	ofBackground(100,100,100);
	vidGrabber.update();
	if (vidGrabber.isFrameNew()) {
        flow.update(vidGrabber);
        calcAveFlow();
    }

}

//--------------------------------------------------------------
void testApp::draw(){
    int curGesture = getGesture();
    int boxW=300;
    int boxH=300;
    int rightBoxX = (ofGetWidth()/2) - boxW;
    int rightBoxY = ofGetHeight() * 0.5;
    int leftBoxX = (ofGetWidth()/2);
    int leftBoxY = ofGetHeight() * 0.5;
    int barX = (ofGetWidth() - gradBar.width)/2;
    int barY = ofGetHeight()*0.9;
    int barH = gradBar.height;
    int gestureX = ofMap(aveFlowEased.x, MIN_FLOW_VEC_LEN, MAX_FLOW_VEC_LEN, barX, barX+gradBar.width) - handIcon.width/2; //map from flow vector to screen coords of bar
    int leftThreshX = ofMap(gestureThresh.get(),GESTURE_THRESH_MIN,GESTURE_THRESH_MAX,barX+(gradBar.width/2),barX);
    int rightThreshX = ofMap(gestureThresh.get(),GESTURE_THRESH_MIN,GESTURE_THRESH_MAX,barX+(gradBar.width/2),barX+gradBar.width);

    
    
    
	ofSetHexColor(0xffffff);
	vidGrabber.draw(0,0);
	flow.draw(camWidth,camHeight,lineScale.get(),drawRes.get());
    

    gradBar.draw(barX, barY);
    handIcon.draw(gestureX, barY);
    ofLine(rightThreshX, barY, rightThreshX, barY+barH);
    ofLine(leftThreshX, barY, leftThreshX, barY+barH);
    
    
    
//    if(curGesture == GESTURE_RIGHT) {
//        ofSetColor(255, 0, 0);
//        ofRect(rightBoxX, rightBoxY, boxW, boxH);
//    } else if(curGesture == GESTURE_LEFT) {
//        ofSetColor(0, 0, 255);
//        ofRect(leftBoxX, leftBoxY, boxW, boxH);
//    }
//    
//    ofSetHexColor(0xffffff);
    
    
    if(bDebug) {
        gui.draw();
        ofDrawBitmapString("Ave Flow:" + ofToString(aveFlowEased), ofGetWidth()*0.7,ofGetHeight()*0.05);
        ofDrawBitmapString("Ave Flow Len:" + ofToString(aveFlowEased.length()), ofGetWidth()*0.7,ofGetHeight()*0.1);
    };
}


//--------------------------------------------------------------
void testApp::keyPressed  (int key){ 
	
	// in fullscreen mode, on a pc at least, the 
	// first time video settings the come up
	// they come up *under* the fullscreen window
	// use alt-tab to navigate to the settings
	// window. we are working on a fix for this...
	
	// Video settings no longer works in 10.7
	// You'll need to compile with the 10.6 SDK for this
    // For Xcode 4.4 and greater, see this forum post on instructions on installing the SDK
    // http://forum.openframeworks.cc/index.php?topic=10343        
	if (key == 's' || key == 'S'){
		vidGrabber.videoSettings();
	}
	
    if(key == 'd') {
        bDebug = !bDebug;
    }
	
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){ 
	
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}
