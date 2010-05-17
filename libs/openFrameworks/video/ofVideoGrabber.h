#ifndef _OF_VIDEO_GRABBER
#define _OF_VIDEO_GRABBER

#include "ofConstants.h"
#include "ofTexture.h"
#include "ofGraphics.h"
#include "ofTypes.h"
#include "ofVideoAPIS.h"


// todo:
// 		QT - callback, via SGSetDataProc - couldn't get this to work yet
// 		image decompress options ala mung...


class ofVideoGrabber : public ofBaseVideo{

	public :

		ofVideoGrabber();
		virtual ~ofVideoGrabber();

		void 			setGrabberAPI(ofVideoGrabberAPI * api);

		void 			listDevices();
		void			setDesiredFrameRate(int framerate);
		bool			initGrabber(int w, int h, bool bTexture = true);

		bool 			isFrameNew();
		void			update();
		void			grabFrame();
		void			close();
		void			videoSettings();
		void			setDeviceID(int _deviceID);

		void 			setVerbose(bool bTalkToMe);

		unsigned char 	* getPixels();

		ofTexture &		getTextureReference();
		void 			setUseTexture(bool bUse);
		void 			draw(float x, float y, float w, float h);
		void 			draw(float x, float y);

		//the anchor is the point the image is drawn around.
		//this can be useful if you want to rotate an image around a particular point.
        void			setAnchorPercent(float xPct, float yPct);	//set the anchor as a percentage of the image width/height ( 0.0-1.0 range )
        void			setAnchorPoint(int x, int y);				//set the anchor point in pixels
        void			resetAnchor();								//resets the anchor to (0, 0)

		float 			getHeight();
		float 			getWidth();

		ofVideoGrabberAPI * getAPI();

		int			height;
		int			width;

	protected:

		bool					bChooseDevice;
		int						deviceID;
		bool					bUseTexture;
		ofTexture 				tex;
		bool 					bVerbose;
		bool 					bGrabberInited;
		int						attemptFramerate;

		ofVideoGrabberAPI * 	api;

};




#endif

