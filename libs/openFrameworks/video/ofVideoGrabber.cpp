#include "ofVideoGrabber.h"
#include "ofUtils.h"

#ifdef TARGET_OSX
	#include "ofQtUtils.h"
#endif

#ifdef TARGET_WIN
	#include "videoInput.h"
#endif

#ifdef TARGET_LINUX
	#include "ofGstUtils.h"
#endif

//--------------------------------------------------------------------
ofVideoGrabber::ofVideoGrabber(){

	bVerbose 				= false;
	bGrabberInited 			= false;
	bUseTexture				= true;
	bChooseDevice			= false;
	deviceID				= 0;
	width 					= 320;	// default setting
	height 					= 240;	// default setting
	api						= NULL;
}


//--------------------------------------------------------------------
ofVideoGrabber::~ofVideoGrabber(){
	close();
}


//--------------------------------------------------------------------
void ofVideoGrabber::listDevices(){
	api->listDevices();
}

//--------------------------------------------------------------------
void ofVideoGrabber::setVerbose(bool bTalkToMe){
	bVerbose = bTalkToMe;
}

//--------------------------------------------------------------------
void ofVideoGrabber::setDeviceID(int _deviceID){
	deviceID		= _deviceID;
	bChooseDevice	= true;
}

//--------------------------------------------------------------------
void ofVideoGrabber::setDesiredFrameRate(int framerate){
	attemptFramerate = framerate;
}

//---------------------------------------------------------------------------
unsigned char * ofVideoGrabber::getPixels(){
		return api->getPixels();
}

//------------------------------------
//for getting a reference to the texture
ofTexture & ofVideoGrabber::getTextureReference(){
	if(!tex.bAllocated() ){
		ofLog(OF_LOG_WARNING, "ofVideoGrabber - getTextureReference - texture is not allocated");
	}
	return tex;
}

//---------------------------------------------------------------------------
bool  ofVideoGrabber::isFrameNew(){
	return api->isFrameNew();
}

//--------------------------------------------------------------------
void ofVideoGrabber::update(){
	grabFrame();
}

//--------------------------------------------------------------------
void ofVideoGrabber::grabFrame(){
	if (bGrabberInited){
		api->update();
		bool bIsFrameNew = api->isFrameNew();
		if(bIsFrameNew) {
			if (bUseTexture){
				tex.loadData(api->getPixels(), api->getWidth(), api->getHeight(), GL_RGB);
			}
		}
	}
}

//--------------------------------------------------------------------
void ofVideoGrabber::close(){
	if(api)
		api->close();

	tex.clear();
}

//--------------------------------------------------------------------
void ofVideoGrabber::videoSettings(void){
	api->videoSettings();
}


//--------------------------------------------------------------------
bool ofVideoGrabber::initGrabber(int w, int h, bool setUseTexture){
	bUseTexture = setUseTexture;

	//---------------------------------
	#ifdef TARGET_OSX
	//---------------------------------
	if(!api) api = new ofQTGrabberAPI;
	//---------------------------------
	#endif
	//---------------------------------

	//---------------------------------
	#ifdef TARGET_WIN
	//---------------------------------
	if(!api) api = new ofDSGrabberAPI;
	//---------------------------------
	#endif
	//---------------------------------

	//---------------------------------
	#ifdef TARGET_LINUX
	//---------------------------------
	if(!api) api = new ofGstUtils;
	//---------------------------------
	#endif
	//---------------------------------

	if(bChooseDevice) api->setDeviceID(deviceID);

	if(api->initGrabber(w,h,attemptFramerate)){
		width 	= getWidth();
		height 	= getHeight();
		if (bUseTexture){
			// create the texture, set the pixels to black and
			// upload them to the texture (so at least we see nothing black the callback)
			tex.allocate(width,height,GL_RGB);
			tex.loadData(getPixels(), width, height, GL_RGB);
		}
		bGrabberInited = true;
		ofLog(OF_LOG_VERBOSE, "ofVideoGrabber: initied");
	}else{
		bGrabberInited = false;
		ofLog(OF_LOG_ERROR, "ofVideoGrabber: couldn't init");
	}
	return bGrabberInited;
}

//------------------------------------
void ofVideoGrabber::setUseTexture(bool bUse){
	bUseTexture = bUse;
}

//we could cap these values - but it might be more useful
//to be able to set anchor points outside the image

//----------------------------------------------------------
void ofVideoGrabber::setAnchorPercent(float xPct, float yPct){
    if (bUseTexture)tex.setAnchorPercent(xPct, yPct);
}

//----------------------------------------------------------
void ofVideoGrabber::setAnchorPoint(int x, int y){
    if (bUseTexture)tex.setAnchorPoint(x, y);
}

//----------------------------------------------------------
void ofVideoGrabber::resetAnchor(){
   	if (bUseTexture)tex.resetAnchor();
}

//------------------------------------
void ofVideoGrabber::draw(float _x, float _y, float _w, float _h){
	if (bUseTexture){
		tex.draw(_x, _y, _w, _h);
	}
}

//------------------------------------
void ofVideoGrabber::draw(float _x, float _y){
	draw(_x, _y, (float)width, (float)height);
}


//----------------------------------------------------------
float ofVideoGrabber::getHeight(){
	return api->getHeight();
}

//----------------------------------------------------------
float ofVideoGrabber::getWidth(){
	return api->getWidth();
}

//----------------------------------------------------------
void ofVideoGrabber::setGrabberAPI(ofVideoGrabberAPI * _api){
	close();
	api = _api;
}

//----------------------------------------------------------
ofVideoGrabberAPI * ofVideoGrabber::getAPI(){
	return api;
}
