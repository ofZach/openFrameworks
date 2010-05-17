#include "ofVideoPlayer.h"
#include "ofUtils.h"

#ifdef TARGET_LINUX
	#include "ofGstUtils.h"
#else
	#include "ofQtUtils.h"
#endif


//---------------------------------------------------------------------------
ofVideoPlayer::ofVideoPlayer (){

	width 			= 0;
	height			= 0;
	bUseTexture		= true;
	api				= NULL;

}

//---------------------------------------------------------------------------
unsigned char * ofVideoPlayer::getPixels(){
	return api->getPixels();
}

//------------------------------------
//for getting a reference to the texture
ofTexture & ofVideoPlayer::getTextureReference(){
	if(!tex.bAllocated() ){
		ofLog(OF_LOG_WARNING, "ofVideoPlayer - getTextureReference - texture is not allocated");
	}
	return tex;
}


//---------------------------------------------------------------------------
bool ofVideoPlayer::isFrameNew(){
	return api->isFrameNew();
}

//--------------------------------------------------------------------
void ofVideoPlayer::update(){
	idleMovie();
}

//---------------------------------------------------------------------------
void ofVideoPlayer::idleMovie(){
	if (api->isLoaded()){
		api->update();
		if(api->isFrameNew()){
			width = api->getWidth();
			height = api->getHeight();
			tex.loadData(api->getPixels(), width, height, GL_RGB);
		}
	}
}

//---------------------------------------------------------------------------
void ofVideoPlayer::closeMovie(){
	api->close();
	tex.clear();
}

//---------------------------------------------------------------------------
void ofVideoPlayer::close(){
	closeMovie();
}

//---------------------------------------------------------------------------
ofVideoPlayer::~ofVideoPlayer(){
	closeMovie();
	delete api;
}


//---------------------------------------------------------------------------
bool ofVideoPlayer::loadMovie(string name){
	//---------------------------------
	#ifdef TARGET_LINUX
	//---------------------------------
	if(!api) api = new ofGstUtils();
	//---------------------------------
	#else
	//---------------------------------
	if(!api) api = new ofQtPlayerAPI();
	//---------------------------------
	#endif
	//---------------------------------

	if(api->loadMovie(name)){
		if(bUseTexture){
			tex.allocate(api->getWidth(),api->getHeight(),GL_RGB,false);
			tex.loadData(api->getPixels(), api->getWidth(), api->getHeight(), GL_RGB);
		}
		height = api->getHeight();
		width  = api->getWidth();
		ofLog(OF_LOG_VERBOSE,"ofVideoPlayer: movie loaded");
		return true;
	}else{
		ofLog(OF_LOG_ERROR,"ofVideoPlayer couldn't load movie");
		return false;
	}
}

//--------------------------------------------------------
void ofVideoPlayer::play(){
	api->play();
}

//--------------------------------------------------------
void ofVideoPlayer::stop(){
	api->stop();
}

//--------------------------------------------------------
void ofVideoPlayer::setVolume(int volume){
	api->setVolume(volume);
}


//--------------------------------------------------------
void ofVideoPlayer::setLoopState(int state){
	api->setLoopState(state);
}


//---------------------------------------------------------------------------
void ofVideoPlayer::setPosition(float pct){
	api->setPosition(pct);
}

//---------------------------------------------------------------------------
void ofVideoPlayer::setFrame(int frame){
   api->setFrame(frame);
}


//---------------------------------------------------------------------------
float ofVideoPlayer::getDuration(){
	return api->getDuration();
}

//---------------------------------------------------------------------------
float ofVideoPlayer::getPosition(){
	return api->getPosition();
}

//---------------------------------------------------------------------------
int ofVideoPlayer::getCurrentFrame(){
	return api->getCurrentFrame();
}


//---------------------------------------------------------------------------
bool ofVideoPlayer::getIsMovieDone(){
	return api->getIsMovieDone();
}

//---------------------------------------------------------------------------
void ofVideoPlayer::firstFrame(){
	api->firstFrame();
}

//---------------------------------------------------------------------------
void ofVideoPlayer::nextFrame(){
	api->nextFrame();
}

//---------------------------------------------------------------------------
void ofVideoPlayer::previousFrame(){
	api->previousFrame();
}

//---------------------------------------------------------------------------
void ofVideoPlayer::setSpeed(float _speed){
	api->setSpeed(_speed);
}

//---------------------------------------------------------------------------
float ofVideoPlayer::getSpeed(){
	return api->getSpeed();
}

//---------------------------------------------------------------------------
void ofVideoPlayer::setPaused(bool _bPause){
	api->setPaused(_bPause);
}

//------------------------------------
void ofVideoPlayer::setUseTexture(bool bUse){
	bUseTexture = bUse;
}

//we could cap these values - but it might be more useful
//to be able to set anchor points outside the image

//----------------------------------------------------------
void ofVideoPlayer::setAnchorPercent(float xPct, float yPct){
    if (bUseTexture)tex.setAnchorPercent(xPct, yPct);
}

//----------------------------------------------------------
void ofVideoPlayer::setAnchorPoint(int x, int y){
    if (bUseTexture)tex.setAnchorPoint(x, y);
}

//----------------------------------------------------------
void ofVideoPlayer::resetAnchor(){
   	if (bUseTexture)tex.resetAnchor();
}

//------------------------------------
void ofVideoPlayer::draw(float _x, float _y, float _w, float _h){
	if (bUseTexture){
		tex.draw(_x, _y, _w, _h);
	}
}

//------------------------------------
void ofVideoPlayer::draw(float _x, float _y){
	draw(_x, _y, (float)width, (float)height);
}

//------------------------------------
int ofVideoPlayer::getTotalNumFrames(){
	return api->getTotalNumFrames();
}

//----------------------------------------------------------
float ofVideoPlayer::getHeight(){
	return api->getHeight();
}

//----------------------------------------------------------
float ofVideoPlayer::getWidth(){
	return api->getWidth();
}

//----------------------------------------------------------
bool ofVideoPlayer::isPaused(){
	return api->isPaused();
}

//----------------------------------------------------------
bool ofVideoPlayer::isLoaded(){
	return api->isLoaded();
}

//----------------------------------------------------------
bool ofVideoPlayer::isPlaying(){
	return api->isPlaying();
}

//----------------------------------------------------------
void ofVideoPlayer::setPlayerAPI(ofVideoPlayerAPI * _api){
	close();
	api = _api;
}

//----------------------------------------------------------
ofVideoPlayerAPI *  ofVideoPlayer::getPlayerAPI(){
	return api;
}
