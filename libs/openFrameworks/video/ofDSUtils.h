#ifndef OFDSUTILS_H_
#define OFDSUTILS_H_

#include "ofConstants.h"
#ifdef TARGET_WIN
class ofDSGrabberAPI: public ofVideoGrabberAPI{

	ofDSGrabberAPI();
	virtual ~ofDSGrabberAPI();

	void 			listDevices();
	bool 			isFrameNew();
	void			close();
	bool			initGrabber(int w, int h, int framerate=-1);
	void			videoSettings();
	unsigned char 	* getPixels();
	void			setDeviceID(int _deviceID);
	void			update();

	int 			getHeight();
	int 			getWidth();

protected:
	int 					device;
	videoInput 				VI;
	unsigned char *			pixels;
	int						width, height;
	int						deviceID;

	bool 					bDoWeNeedToResize;
}

#endif
#endif
