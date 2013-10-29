#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	
//////////////////////////////////////////////////////
    MODE = FRAME_AVE_MODE;
//////////////////////////////////////////////////////
    
	camWidth 		= 320;	// try to grab at this size. 
	camHeight 		= 240;
    bDebug = false;
    lastGestureComplete = true;
    leftGestureDetected = false;
    rightGestureDetected = false;
    gestureStartTime = 0;
    curFlow = ofPoint(0,0);
    
    cout << "Listing available video devices:" << endl;
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
    flow.setup(camWidth,camHeight);
    ofSetVerticalSync(true);
    
    setupGUI();
    loadSettings();
    
    gradBar.loadImage("gradBar.png");
    handIcon.loadImage("openhand.png");
    
    serial.listDevices();
    serial.setup(0, BAUD_RATE); //open the first device
}

//--------------------------------------------------------------
void testApp::setupGUI() {
    
    int maxGridLen = camHeight/MIN_GRID_LEN;
    gui.setup();
    gui.add(optiFlowSize.set("Opti Flow Size", 10, 1, 50));
    gui.add(optiFlowBlur.set("Opti Flow Blur", 10, 1, 50));
    gui.add(lineScale.set("Line Scale", 5, 1, 50));
    gui.add(drawRes.set("Draw Res", 10, 1, 50));
    gui.add(numAveFrames.set("Number Average Frames", 3, 1, 60));
    gui.add(flowGridRes.set("Grid Size", 10, MIN_GRID_LEN, maxGridLen));
    gui.add(gestureThresh.set("Gesture Threshold", 1.5, GESTURE_THRESH_MIN, GESTURE_THRESH_MAX));
    gui.add(gestureTimeOut.set("Gesture Time Out", 0.5, 0, 2));
    gui.add(buttonLoad.setup("Load"));
    gui.add(buttonSave.setup("Save"));
    buttonSave.addListener(this, &testApp::buttonSaveChanged);
    buttonLoad.addListener(this, &testApp::buttonLoadChanged);
    optiFlowSize.addListener(this, &testApp::setFlowSize);
    optiFlowBlur.addListener(this, &testApp::setFlowBlur);
    optiFlowBlur.addListener(this, &testApp::setFlowBlur);
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

void testApp::loadSettings() {
    settings.loadFile("settings.xml");
    optiFlowBlur.set(settings.getValue("settings:optiFlowBlur", 10));
    optiFlowSize.set(settings.getValue("settings:optiFlowSize", 10));
    lineScale.set(settings.getValue("settings:lineScale", 5));
    drawRes.set(settings.getValue("settings:drawRes", 10));
    numAveFrames.set(settings.getValue("settings:numAveFrames", 3));
    flowGridRes.set(settings.getValue("settings:flowGridRes", 10));
    gestureThresh.set(settings.getValue("settings:gestureThresh", 1.5));
    gestureTimeOut.set(settings.getValue("settings:gestureTimeOut", 0.5));
}

//--------------------------------------------------------------

void testApp::buttonLoadChanged() {
    loadSettings();
}

//--------------------------------------------------------------
void testApp::buttonSaveChanged() {
    settings.setValue("settings:optiFlowSize", optiFlowSize.get());
    settings.setValue("settings:optiFlowBlur", optiFlowBlur.get());
    settings.setValue("settings:lineScale", lineScale.get());
    settings.setValue("settings:drawRes", drawRes.get());
    settings.setValue("settings:numAveFrames", numAveFrames.get());
    settings.setValue("settings:flowGridRes", flowGridRes.get());
    settings.setValue("settings:gestureThresh", gestureThresh.get());
    settings.setValue("settings:gestureTimeOut", gestureTimeOut.get());
    settings.saveFile("settings.xml");
    
}
//--------------------------------------------------------------
void testApp::getGesture() {
    float now = ofGetElapsedTimef();
    lastGestureComplete = (now - gestureStartTime) >= gestureTimeOut.get();
    
    if(lastGestureComplete) {
        curGesture = NO_GESTURE;
    }
    
    if(aveFlow.length() > gestureThresh.get() && lastGestureComplete) { //only actuate a gesture when flow vector is long enough and the last gesture has finished
        gestureStartTime = now;
        if(aveFlow.x < 0) {
            ofDrawBitmapString("Gesture Right Detected", ofGetWidth()*0.7,ofGetHeight()*0.1);
            curGesture = GESTURE_RIGHT;
            boxCol = ofColor(255,0,0);
            serial.writeByte('1');
        } else if(aveFlow.x > 0) {
            ofDrawBitmapString("Gesture Left Detected", ofGetWidth()*0.7,ofGetHeight()*0.1);
            curGesture = GESTURE_LEFT;
            boxCol = ofColor(0,0,255);
            serial.writeByte('2');
        }
    }
    
}


//--------------------------------------------------------------

void testApp::calcAveFlow() {
    int gridEdgeLen = flowGridRes.get();
    int numX = camWidth/gridEdgeLen;
    int numY = camHeight/gridEdgeLen;
    ofPoint ave;
    int numFlows = 0;
    int curFrame = 0;
    
    for(int i=0; i<numX; i++) {
        for(int j=0; j<numY; j++) {
            ofPoint flowVec = flow.getVelAtPixel(i*gridEdgeLen, j*gridEdgeLen);
            ave+=flowVec;
            numFlows++;
        }
    }
    
    ave/=numFlows;

    if(MODE == FRAME_AVE_MODE) {
        curFlow += ave;
        if(curFrame % numAveFrames.get() == 0) {
            aveFlow = curFlow/numAveFrames.get();
            curFlow = ofPoint(0);
        }
    } else if(MODE == EASING_MODE) {
        aveFlow = ave;
        aveFlowEased += (aveFlow - aveFlowEased) * 0.1;
    }
    
    
}

//--------------------------------------------------------------
void testApp::update(){
	
	ofBackground(100,100,100);
	vidGrabber.update();
	if (vidGrabber.isFrameNew()) {
        flow.update(vidGrabber);
        calcAveFlow();
        getGesture();
    }

}

//--------------------------------------------------------------
void testApp::draw(){

    int boxW=300;
    int boxH=300;
    int rightBoxX = (ofGetWidth()/2);
    int rightBoxY = ofGetHeight() * 0.5;
    int leftBoxX = (ofGetWidth()/2) - boxW;
    int leftBoxY = ofGetHeight() * 0.5;
    int barX = (ofGetWidth() - gradBar.width)/2;
    int barY = ofGetHeight()*0.9;
    int barH = gradBar.height;
    int gestureX = ofMap(aveFlow.x, MIN_FLOW_VEC_LEN, MAX_FLOW_VEC_LEN, barX, barX+gradBar.width) - handIcon.width/2; //map from flow vector to screen coords of bar
    int leftThreshX = ofMap(gestureThresh.get(),GESTURE_THRESH_MIN,GESTURE_THRESH_MAX,barX+(gradBar.width/2),barX);
    int rightThreshX = ofMap(gestureThresh.get(),GESTURE_THRESH_MIN,GESTURE_THRESH_MAX,barX+(gradBar.width/2),barX+gradBar.width);
    int colorInt = (255-75)/gestureTimeOut.get(); //75 is the default gray

    
    
    
	ofSetHexColor(0xffffff);
	vidGrabber.draw(0,0);
	flow.draw(camWidth,camHeight,lineScale.get(),drawRes.get());
    

    gradBar.draw(barX, barY);
    handIcon.draw(gestureX, barY);
    ofLine(rightThreshX, barY, rightThreshX, barY+barH);
    ofLine(leftThreshX, barY, leftThreshX, barY+barH);
    
    
    if(curGesture == GESTURE_LEFT ) {
        ofSetColor(boxCol);
//        boxCol.b-=colorInt;
        ofRect(leftBoxX, leftBoxY, boxW, boxH);
    } else if(curGesture == GESTURE_RIGHT ) {
        ofSetColor(boxCol);
        ofRect(rightBoxX, rightBoxY, boxW, boxH);
//        boxCol.r-=colorInt;
    } else {
        ofSetColor(100, 100, 100);
        ofRect(rightBoxX, rightBoxY, boxW, boxH);
        ofRect(leftBoxX, leftBoxY, boxW, boxH);
    }
    
    ofSetHexColor(0xffffff);

    
    if(bDebug) {
        gui.draw();
        ofDrawBitmapString("Ave Flow Eased:" + ofToString(aveFlowEased), ofGetWidth()*0.7,ofGetHeight()*0.05);
        ofDrawBitmapString("Ave Flow Len:" + ofToString(aveFlowEased.length()), ofGetWidth()*0.7,ofGetHeight()*0.1);
        ofDrawBitmapString("Ave Flow:" + ofToString(aveFlow), ofGetWidth()*0.7,ofGetHeight()*0.15);
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
    
    if(key == '1' ) {
        serial.writeByte('1');
    }
    
    if(key == '2' ) {
        serial.writeByte('2');
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
