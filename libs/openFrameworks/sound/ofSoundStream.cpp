#include "ofSoundStream.h"
#include "ofRTAudioAPI.h"
#include "ofConstants.h"
#include "ofEvents.h"
#include "ofMath.h"

//----------------------------------- static variables:
static ofSoundStreamAPI 	* 		api = NULL;

//---------------------------------------------------------
void ofSoundStreamSetAPI(ofSoundStreamAPI * _api){
	api = _api;
}

//---------------------------------------------------------
void ofSoundStreamSetup(int nOutputs, int nInputs, ofBaseApp * OFSA){
	ofSoundStreamSetup(nOutputs, nInputs, OFSA, 44100, 256, 4);
}

//---------------------------------------------------------
void ofSoundStreamSetup(int nOutputs, int nInputs, int sampleRate, int bufferSize, int nBuffers){
	ofSoundStreamSetup(nOutputs, nInputs, NULL, sampleRate, bufferSize, nBuffers);
}

//---------------------------------------------------------
void ofSoundStreamSetup(int nOutputs, int nInputs, ofBaseApp * OFSA, int sampleRate, int bufferSize, int nBuffers){

	if(!api) api = new ofRTAudioAPI;
	api->setup(nOutputs, nInputs, OFSA, sampleRate, bufferSize, nBuffers);

}

//---------------------------------------------------------
void ofSoundStreamStop(){
	api->stop();
}

//---------------------------------------------------------
void ofSoundStreamStart(){
	api->start();
}

//---------------------------------------------------------
void ofSoundStreamClose(){
	api->close();
}

//---------------------------------------------------------
void ofSoundStreamListDevices(){
	api->listDevices();
}
