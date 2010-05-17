#ifndef _OF_SOUND_STREAM
#define _OF_SOUND_STREAM

#include "ofBaseApp.h"
#include "ofSoundStreamAPI.h"

void ofSoundStreamSetAPI(ofSoundStreamAPI * api);
void ofSoundStreamSetup(int nOutputChannels, int nInputChannels, ofBaseApp * OFSA = NULL);
void ofSoundStreamSetup(int nOutputChannels, int nInputChannels, int sampleRate, int bufferSize, int nBuffers);
void ofSoundStreamSetup(int nOutputChannels, int nInputChannels, ofBaseApp * OFSA, int sampleRate, int bufferSize, int nBuffers);
void ofSoundStreamStop();
void ofSoundStreamStart();
void ofSoundStreamClose();
void ofSoundStreamListDevices();

#endif
