#ifndef _OF_SOUND_STREAM_API
#define _OF_SOUND_STREAM_API

class ofSoundStreamAPI{
public:
	ofSoundStreamAPI(){};
	virtual ~ofSoundStreamAPI(){};
	virtual void setDeviceId(int deviceID)=0;
	//virtual void setDeviceIdByName(string _deviceName)=0;
	virtual void setup(int nOutputChannels, int nInputChannels, ofBaseApp * OFSA = NULL)=0;
	virtual void setup(int nOutputChannels, int nInputChannels, unsigned int sampleRate, unsigned int bufferSize, unsigned int nBuffers)=0;
	virtual void setup(int nOutputChannels, int nInputChannels, ofBaseApp * OFSA, unsigned int sampleRate, unsigned int bufferSize, unsigned int nBuffers)=0;
	virtual void stop()=0;
	virtual void start()=0;
	virtual void close()=0;
	virtual void listDevices()=0;
};

#endif
