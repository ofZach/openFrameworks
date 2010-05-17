/*
 * ofVideoAPIS.h
 *
 *  Created on: 02/05/2010
 *      Author: arturo
 */

#ifndef OFVIDEOAPIS_H_
#define OFVIDEOAPIS_H_

#include "ofTypes.h"


class ofVideoGrabberAPI: public ofBaseHasPixels, public ofBaseUpdates{
public:
	ofVideoGrabberAPI(){};
	virtual ~ofVideoGrabberAPI(){};

	virtual void 			listDevices()=0;
	virtual bool 			isFrameNew()=0;
	virtual void			close()=0;
	virtual bool			initGrabber(int w, int h, int framerate)=0;
	virtual void			videoSettings()=0;
	virtual unsigned char 	* getPixels()=0;
	virtual void			setDeviceID(int _deviceID)=0;
	virtual void			update()=0;

	virtual int 			getHeight()=0;
	virtual int 			getWidth()=0;
};

class ofVideoPlayerAPI: public ofBaseHasPixels, public ofBaseUpdates{
public:
	ofVideoPlayerAPI(){};
	virtual ~ofVideoPlayerAPI(){};

	virtual bool 			isFrameNew()=0;
	virtual void			close()=0;
	virtual bool			loadMovie(string name)=0;

	virtual void			play()=0;
	virtual void			stop()=0;

	virtual void			update()=0;

	virtual float			getPosition()=0;
	virtual float			getSpeed()=0;
	virtual float			getDuration()=0;
	virtual bool			getIsMovieDone()=0;
	virtual int				getCurrentFrame()=0;
	virtual int				getTotalNumFrames()=0;

	virtual void			setPosition(float pct)=0;
	virtual void			setVolume(int volume)=0;
	virtual void			setLoopState(int state)=0;
	virtual void			setSpeed(float speed)=0;
	virtual void			setFrame(int frame)=0;  // frame 0 = first frame...
	virtual void			setPaused(bool bPause)=0;

	virtual bool			isPaused()=0;
	virtual bool			isLoaded()=0;
	virtual bool			isPlaying()=0;

	virtual void			firstFrame()=0;
	virtual void			nextFrame()=0;
	virtual void			previousFrame()=0;

	virtual unsigned char * getPixels()=0;

	virtual int 			getHeight()=0;
	virtual int 			getWidth()=0;
};


#endif /* OFVIDEOAPIS_H_ */
