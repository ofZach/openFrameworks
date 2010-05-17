#ifndef _OF_PORT_AUDIO_API
#define _OF_PORT_AUDIO_API

#include "ofConstants.h"
#include "ofEvents.h"
#include "ofSoundStreamAPI.h"
#include "portaudio.h"

class ofBaseApp;

class ofPortAudioAPI: public ofSoundStreamAPI{
public:
	ofPortAudioAPI();
	virtual ~ofPortAudioAPI();
	void setDeviceId(int deviceID);
	//void setDeviceIdByName(string _deviceName);
	void setup(int nOutputChannels, int nInputChannels, ofBaseApp * OFSA = NULL);
	void setup(int nOutputChannels, int nInputChannels, unsigned int sampleRate, unsigned int bufferSize, unsigned int nBuffers);
	void setup(int nOutputChannels, int nInputChannels, ofBaseApp * OFSA, unsigned int sampleRate, unsigned int bufferSize, unsigned int nBuffers);
	void stop();
	void start();
	void close();
	void listDevices();

	int 	receiveAudioBuffer(void *outputBuffer, const void *inputBuffer, int bufferSize);

protected:
	ofBaseApp 	* 		OFSAptr;
	PaStream	*		stream;
	int 				nInputChannels;
	int 				nOutputChannels;
	ofAudioEventArgs 	audioEventArgs;
	int					deviceID;
	
	string getDeviceName();

};

#endif
