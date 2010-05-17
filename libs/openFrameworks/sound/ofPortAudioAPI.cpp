#include "ofPortAudioAPI.h"
#include <iostream>
#include "ofUtils.h"
#include "ofBaseApp.h"
#include "ofMath.h"

//------------------------------------------------------------------------------
// rtAudio callback
int ofPAAudioCallback(const void *inputBuffer,
        void *outputBuffer,
        unsigned long bufferSize,
        const PaStreamCallbackTimeInfo* streamTime,
        PaStreamCallbackFlags status,
        void *data );


//------------------------------------------------------------------------------
int ofPAAudioCallback(const void *inputBuffer,
        void *outputBuffer,
        unsigned long bufferSize,
        const PaStreamCallbackTimeInfo* streamTime,
        PaStreamCallbackFlags status,
        void *data){
	
	
	if ( status ) std::cout << "Stream over/underflow detected." << std::endl;
	
	ofPortAudioAPI * api = (ofPortAudioAPI*)data;
	return api->receiveAudioBuffer(outputBuffer, inputBuffer, bufferSize);
}

//------------------------------------------------------------------------------
ofPortAudioAPI::ofPortAudioAPI(){
	OFSAptr 		= NULL;
	stream 			= NULL;
	nInputChannels	= 0;
	nOutputChannels = 0;
	deviceID		= 0;
}

//------------------------------------------------------------------------------
ofPortAudioAPI::~ofPortAudioAPI(){
	close();
}

//------------------------------------------------------------------------------
void ofPortAudioAPI::setDeviceId(int _deviceID){
	if(stream && deviceID!=_deviceID){
		//ofLog(OF_ERROR,"cannot change device with stream already setup");
		cout << "ERROR: cannot change device with stream already setup" << endl;
	}
	else{
		cout << "Device ID set to : " << _deviceID << endl;
	}
	deviceID=_deviceID;
}

//------------------------------------------------------------------------------
/*void ofRTAudioAPI::setDeviceIdByName(string deviceName){
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
}*/

//---------------------------------------------------------
void ofPortAudioAPI::setup(int nOutputs, int nInputs, ofBaseApp * OFSA){
	setup(nOutputs, nInputs, OFSA, 44100, 256, 4);
}

//---------------------------------------------------------
void ofPortAudioAPI::setup(int nOutputs, int nInputs, unsigned int sampleRate, unsigned int bufferSize, unsigned int nBuffers){
	setup(nOutputs, nInputs, NULL, sampleRate, bufferSize, nBuffers);
}

//---------------------------------------------------------
void ofPortAudioAPI::setup(int nOutputs, int nInputs, ofBaseApp * OFSA, unsigned int sampleRate, unsigned int bufferSize, unsigned int nBuffers){
	nInputChannels 		=  nInputs;
	nOutputChannels 	=  nOutputs;
	OFSAptr 			=  OFSA;
	bufferSize = ofNextPow2(bufferSize);	// must be pow2


	PaError err;
	err = Pa_Initialize();
	if( err != paNoError ){
		ofLog(OF_LOG_ERROR,"PortAudio error initializing: %s\n",Pa_GetErrorText( err ));
		return;
	}

	/*PaStreamParameters outStreamParameters, inStreamParameters;
	PaSampleFormat sampleFormat = paFloat32;
	outStreamParameters.channelCount = nOutputs;
	outStreamParameters.device = 0;
	outStreamParameters.sampleFormat = sampleFormat;
	outStreamParameters.suggestedLatency = 0;
	outStreamParameters.hostApiSpecificStreamInfo = NULL;

	inStreamParameters.channelCount = nInputs;
	inStreamParameters.device = 0;
	inStreamParameters.sampleFormat = sampleFormat;
	inStreamParameters.suggestedLatency = 0;
	inStreamParameters.hostApiSpecificStreamInfo = NULL;

	PaStreamFlags flags = paNoFlag;

	err = Pa_OpenStream(&stream,NULL,&outStreamParameters,sampleRate,bufferSize,flags,&receiveAudioBufferAndCallSimpleApp,NULL);*/
	err = Pa_OpenDefaultStream( &stream,
									nInputChannels,          // no input channels
									nOutputChannels,          // stereo output
									paFloat32,  // 64 bit floating point output
									sampleRate,
									bufferSize,        // frames per buffer, i.e. the number
													   // of sample frames that PortAudio will
													   // request from the callback. Many apps
													   // may want to use
													   // paFramesPerBufferUnspecified, which
													   // tells PortAudio to pick the best,
													   // possibly changing, buffer size.
									&ofPAAudioCallback, // this is your callback function
									this ); ///This is a pointer that will be passed to
											//your callback
	if( err != paNoError ){
		ofLog(OF_LOG_ERROR,"PortAudio error creating stream: %s\n",Pa_GetErrorText( err ));
		return;
	}

	err = Pa_StartStream( stream );
	if( err != paNoError ){
		ofLog(OF_LOG_ERROR,"PortAudio error starting stream: %s\n",Pa_GetErrorText( err ));
	}

}

//---------------------------------------------------------
void ofPortAudioAPI::stop(){
	int err = Pa_StopStream(stream);
    if( err != paNoError )
    	ofLog(OF_LOG_ERROR,"PortAudio error: %s\n",Pa_GetErrorText( err ));
}



//---------------------------------------------------------
void ofPortAudioAPI::start(){
	int err = Pa_StartStream( stream );
    if( err != paNoError )
    	ofLog(OF_LOG_ERROR,"PortAudio error: %s\n",Pa_GetErrorText( err ));
}


//---------------------------------------------------------
void ofPortAudioAPI::close(){
	int err = Pa_Terminate();
	if( err != paNoError )
	   	ofLog(OF_LOG_ERROR,"PortAudio error: %s\n",Pa_GetErrorText( err ));
}


//---------------------------------------------------------
void ofPortAudioAPI::listDevices(){
	int numDevices;

	numDevices = Pa_GetDeviceCount();
	if( numDevices < 0 )
	{
		ofLog(OF_LOG_ERROR,"PortAudio error: %s\n",Pa_GetErrorText( numDevices ));
		return;
	}
	const   PaDeviceInfo *deviceInfo;

	for( int i=0; i<numDevices; i++ )
	{
		deviceInfo = Pa_GetDeviceInfo( i );
		cout << deviceInfo->name << endl;
		cout << "api: " << deviceInfo->hostApi << endl;
		cout << "max in channels" << deviceInfo->maxInputChannels << endl;
		cout << "max out channels" << deviceInfo->maxOutputChannels << endl;
		cout << "default sample rate:" << deviceInfo->defaultSampleRate << endl;
	}
}

int ofPortAudioAPI::receiveAudioBuffer(void *outputBuffer, const void *inputBuffer, int bufferSize){
	float * fPtrOut = (float *)outputBuffer;
	float * fPtrIn = (float *)inputBuffer;


	if (nInputChannels > 0){
		if(OFSAptr)OFSAptr->audioReceived(fPtrIn, bufferSize, nInputChannels);
		memset(fPtrIn, 0, bufferSize * nInputChannels * sizeof(float));
		#ifdef OF_USING_POCO
			audioEventArgs.buffer = fPtrIn;
			audioEventArgs.bufferSize = bufferSize;
			audioEventArgs.nChannels = nInputChannels;
			ofNotifyEvent( ofEvents.audioReceived, audioEventArgs );
		#endif
	}


	if (nOutputChannels > 0) {
		if(OFSAptr)OFSAptr->audioRequested(fPtrOut, bufferSize, nOutputChannels);
		#ifdef OF_USING_POCO
			audioEventArgs.buffer = fPtrOut;
			audioEventArgs.bufferSize = bufferSize;
			audioEventArgs.nChannels = nOutputChannels;
			ofNotifyEvent( ofEvents.audioRequested, audioEventArgs );
		#endif
	}

	return 0;
}

//------------------------------------------------------------------------------
string ofPortAudioAPI::getDeviceName(){
	return string();
}
