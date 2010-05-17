/*
 *      ofxSoundStream.cpp
 *
 *      v 0.02 March 2010
 *      Arturo Castro & Eduard Prats Molner
 */

#include "ofRTAudioAPI.h"

//------------------------------------------------------------------------------
// rtAudio callback
int ofRTAudioCallback(void *outputBuffer, void *inputBuffer, unsigned int bufferSize,
									   double streamTime, RtAudioStreamStatus status, void *data);


//------------------------------------------------------------------------------
int ofRTAudioCallback(void *outputBuffer, void *inputBuffer, unsigned int bufferSize,
									   double streamTime, RtAudioStreamStatus status, void *data){
	
	
	if ( status ) std::cout << "Stream over/underflow detected." << std::endl;
	
	ofRTAudioAPI * api = (ofRTAudioAPI*)data;
	api->receiveAudioBuffer(outputBuffer, inputBuffer, bufferSize);
	
	return 0;
}
//------------------------------------------------------------------------------
ofRTAudioAPI::ofRTAudioAPI(){
	OFSAptr 		= NULL;
	audio 			= NULL;
	nInputChannels	= 0;
	nOutputChannels = 0;
	deviceID		= 0;
}

//------------------------------------------------------------------------------
ofRTAudioAPI::~ofRTAudioAPI(){
	close();
}


//------------------------------------------------------------------------------
void ofRTAudioAPI::setDeviceId(int _deviceID){
	if(audio && deviceID!=_deviceID){
		//ofLog(OF_ERROR,"cannot change device with stream already setup");
		cout << "ERROR: cannot change device with stream already setup" << endl;
	}
	else{
		cout << "Device ID set to : " << _deviceID << endl;
	}
	deviceID=_deviceID;
}

//------------------------------------------------------------------------------
void ofRTAudioAPI::setDeviceIdByName(string deviceName){
	RtAudio *audioTemp = 0;
	bool found = false;
	try {
		audioTemp = new RtAudio();
	} catch (RtError &error) {
		error.printMessage();
	}
 	int devices = audioTemp->getDeviceCount();
	RtAudio::DeviceInfo info;
	
	for(int i=0;i<devices;i++){
		info = audioTemp->getDeviceInfo(i);
		if(info.name == deviceName){
			found = true;
			if(audio && deviceID!=i){
				//ofLog(OF_ERROR,"cannot change device with stream already setup");
				cout << "ERROR: cannot change device with stream already setup" << endl;
			}
			else{
				deviceID = i;
				cout << "Device ID set to : " << i << endl;
			}
			break;
		}
	}
	
	if(!found){
		cout << "Device Name not found!" << endl;
	}
}

//---------------------------------------------------------
void ofRTAudioAPI::setup(int nOutputs, int nInputs, ofBaseApp * OFSA){
	setup(nOutputs, nInputs, OFSA, 44100, 256, 4);
}

//---------------------------------------------------------
void ofRTAudioAPI::setup(int nOutputs, int nInputs, unsigned int sampleRate, unsigned int bufferSize, unsigned int nBuffers){
	setup(nOutputs, nInputs, NULL, sampleRate, bufferSize, nBuffers);
}

//---------------------------------------------------------
void ofRTAudioAPI::setup(int nOutputs, int nInputs, ofBaseApp * OFSA, unsigned int sampleRate, unsigned int bufferSize, unsigned int nBuffers){
	RtAudio::StreamParameters *inputParameters = NULL;
	RtAudio::StreamParameters *outputParameters = NULL;
	RtAudio::StreamOptions streamOptions;
	
	if(nOutputs > 0){
		outputParameters = new RtAudio::StreamParameters();
		outputParameters->deviceId = deviceID;
		outputParameters->nChannels = nOutputs;
		outputParameters->deviceId = deviceID;
	}
	
	if(nInputs > 0){
		inputParameters = new RtAudio::StreamParameters;
		inputParameters->deviceId = deviceID;
		inputParameters->nChannels = nInputs;
		inputParameters->deviceId = deviceID;
	}
	
	streamOptions.numberOfBuffers = nBuffers;
	streamOptions.flags = RTAUDIO_SCHEDULE_REALTIME;
	streamOptions.priority = 1;

	nInputChannels 		=  nInputs;
	nOutputChannels 	=  nOutputs;
	OFSAptr 			=  OFSA;

	bufferSize = ofNextPow2(bufferSize);	// must be pow2

	if(audio)
		close();

	audio = new RtAudio();
	try {
		
		audio->openStream( outputParameters, inputParameters, RTAUDIO_FLOAT32,
						   sampleRate, &bufferSize, &ofRTAudioCallback, this, &streamOptions);
		audio->startStream();
	} catch (RtError &error) {
		error.printMessage();
	}
}

//---------------------------------------------------------
void ofRTAudioAPI::stop(){
	if(!audio){
		//ofLog(OF_ERROR,"call setup first");
		cout << "ERROR: call setup first" << endl;
		return;
	}
	try {
    	audio->stopStream();
  	} catch (RtError &error) {
   		error.printMessage();
 	}
}



//---------------------------------------------------------
void ofRTAudioAPI::start(){
	if(!audio){
		//ofLog(OF_ERROR,"call setup first");
		cout << "ERROR: call setup first" << endl;
		return;
	}
	try{
		audio->startStream();
	} catch (RtError &error) {
		error.printMessage();
	}
}


//---------------------------------------------------------
void ofRTAudioAPI::close(){
	if(!audio)
		return;

	try {
    	audio->stopStream();
    	audio->closeStream();
  	} catch (RtError &error) {
   		error.printMessage();
 	}
	delete audio;
	audio = NULL;
}


//---------------------------------------------------------
void ofRTAudioAPI::listDevices(){
	RtAudio *audioTemp = 0;
	try {
		audioTemp = new RtAudio();
	} catch (RtError &error) {
		error.printMessage();
	}
 	int devices = audioTemp->getDeviceCount();
	RtAudio::DeviceInfo info;
	for (int i=0; i<devices; i++) {
		try {
			info = audioTemp->getDeviceInfo(i);
		} catch (RtError &error) {
			error.printMessage();
			break;
		}
		std::cout << "device = " << i << " (" << info.name << ")\n";
		if (info.isDefaultInput) std::cout << "----* default ----* \n";
		std::cout << "maximum output channels = " << info.outputChannels << "\n";
		std::cout << "maximum input channels = " << info.inputChannels << "\n";
		std::cout << "-----------------------------------------\n";

	}
	delete audioTemp;
}

int ofRTAudioAPI::receiveAudioBuffer(void *outputBuffer, void *inputBuffer, int bufferSize){
	// 	rtAudio uses a system by which the audio
	// 	can be of different formats
	// 	char, float, etc.
	// 	we choose float
	float * fPtrOut = (float *)outputBuffer;
	float * fPtrIn = (float *)inputBuffer;
	// [zach] memset output to zero before output call
	// this is because of how rtAudio works: duplex w/ one callback
	// you need to cut in the middle. if the simpleApp
	// doesn't produce audio, we pass silence instead of duplex...
	
	
	if (nInputChannels > 0){
		if(OFSAptr) OFSAptr->audioReceived(fPtrIn, bufferSize, nInputChannels);
		//memset(fPtrIn, 0, bufferSize * nInputChannels * sizeof(float));
#ifdef OF_USING_POCO
		audioEventArgs.buffer = fPtrIn;
		audioEventArgs.bufferSize = bufferSize;
		audioEventArgs.nChannels = nInputChannels;
		//audioEventArgs.deviceID = deviceID;
		//audioEventArgs.deviceName = getDeviceName();
		ofNotifyEvent( ofEvents.audioReceived, audioEventArgs, this);
#endif
		memset(fPtrIn, 0, bufferSize * nInputChannels * sizeof(float));
	}
	
	
	if (nOutputChannels > 0) {
		if(OFSAptr) OFSAptr->audioRequested(fPtrOut, bufferSize, nOutputChannels);
#ifdef OF_USING_POCO
		audioEventArgs.buffer = fPtrOut;
		audioEventArgs.bufferSize = bufferSize;
		audioEventArgs.nChannels = nOutputChannels;
		//audioEventArgs.deviceID = deviceID;
		//audioEventArgs.deviceName = getDeviceName();
		ofNotifyEvent( ofEvents.audioRequested, audioEventArgs, this);
#endif
	}
	
	return 0;
}

//------------------------------------------------------------------------------
string ofRTAudioAPI::getDeviceName(){
	RtAudio *audioTemp = 0;
	bool found = false;
	try {
		audioTemp = new RtAudio();
	} catch (RtError &error) {
		error.printMessage();
	}
 	int devices = audioTemp->getDeviceCount();
	RtAudio::DeviceInfo info;
	info = audioTemp->getDeviceInfo(deviceID);
	return info.name;
}
