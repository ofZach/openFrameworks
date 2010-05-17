/*
 *      ofxSoundStream.h
 *
 *      v 0.02 March 2010
 *      Arturo Castro & Eduard Prats Molner
 */

#ifndef _OF_RT_AUDIO_API
#define _OF_RT_AUDIO_API

#include <iostream>
#include "ofConstants.h"
#include "ofUtils.h"
#include "ofBaseApp.h"
#include "ofEvents.h"
#include "ofMath.h"
#include "RtAudio.h"
#include "ofSoundStreamAPI.h"
//#include "ofxAudioEvent.h"

class ofRTAudioAPI: public ofSoundStreamAPI{
public:
	ofRTAudioAPI();
	virtual ~ofRTAudioAPI();
	void setDeviceId(int deviceID);
	void setDeviceIdByName(string _deviceName);
	void setup(int nOutputChannels, int nInputChannels, ofBaseApp * OFSA = NULL);
	void setup(int nOutputChannels, int nInputChannels, unsigned int sampleRate, unsigned int bufferSize, unsigned int nBuffers);
	void setup(int nOutputChannels, int nInputChannels, ofBaseApp * OFSA, unsigned int sampleRate, unsigned int bufferSize, unsigned int nBuffers);
	void stop();
	void start();
	void close();
	void listDevices();

	int 	receiveAudioBuffer(void *outputBuffer, void *inputBuffer, int bufferSize);

protected:
	ofBaseApp 	* 		OFSAptr;
	RtAudio		*		audio;
	int 				nInputChannels;
	int 				nOutputChannels;
	ofAudioEventArgs 	audioEventArgs;
	int					deviceID;
	
	string getDeviceName();

};

#endif
