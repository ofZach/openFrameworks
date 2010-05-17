#ifndef _QT_UTILS
#define _QT_UTILS


#include "ofConstants.h"

#ifndef TARGET_LINUX

#include "ofGraphics.h"


#ifdef TARGET_OSX
	#include <QuickTime/QuickTime.h>
	#include <CoreServices/CoreServices.h>
	#include <ApplicationServices/ApplicationServices.h>
#else
	#include <QTML.h>
	#include <FixMath.h>
	#include <QuickTimeComponents.h>
	#include <TextUtils.h>
	#include <MediaHandlers.h>
	//#include <MoviesFormat.h>
#endif

//p2cstr depreciation fix - thanks pickard!
#ifdef TARGET_OSX
	#define p2cstr(aStr) CFStringGetCStringPtr(CFStringCreateWithPascalString(NULL, aStr, kCFStringEncodingMacRoman),kCFStringEncodingMacRoman)
#endif

//-------------------------- helpful for rgba->rgb conversion
typedef struct{
	unsigned char r;
	unsigned char g;
	unsigned char b;
} pix24;



//----------------------------------------

void 		initializeQuicktime();
void 		closeQuicktime();
void 		convertPixels(unsigned char * gWorldPixels, unsigned char * rgbPixels, int w, int h);
Boolean 	SeqGrabberModalFilterUPP(DialogPtr theDialog, const EventRecord *theEvent, short *itemHit, long refCon);
OSErr           IsMPEGMediaHandler(MediaHandler inMediaHandler, Boolean *outIsMPEG);
ComponentResult MPEGMediaGetStaticFrameRate(MediaHandler inMPEGMediaHandler, Fixed *outStaticFrameRate);
OSErr           MediaGetStaticFrameRate(Media inMovieMedia, double *outFPS);
void            MovieGetVideoMediaAndMediaHandler(Movie inMovie, Media *outMedia,
				MediaHandler *outMediaHandler);
void            MovieGetStaticFrameRate(Movie inMovie, double *outStaticFrameRate);

#ifdef TARGET_OSX
	OSErr	GetSettingsPreference(CFStringRef inKey, UserData *outUserData);
	OSErr	SaveSettingsPreference(CFStringRef inKey, UserData inUserData);
#endif



class ofQTGrabberAPI: public ofVideoGrabberAPI{

	ofQTGrabberAPI();
	virtual ~ofQTGrabberAPI();

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

	//--------------------------------------------------------------------
	#ifdef TARGET_OSX
	//--------------------------------------------------------------------

	bool					saveSettings();
	bool					loadSettings();

	//--------------------------------------------------------------------
	#endif
	//--------------------------------------------------------------------

protected:
	unsigned char *		pixels;
	unsigned char * 	offscreenGWorldPixels;	// 32 bit: argb (qt k32ARGBPixelFormat)
	int 				w,h;
	int 				width, height;
	int					deviceID;
	bool 				bHavePixelsChanged;
	GWorldPtr 			videogworld;
	SeqGrabComponent	gSeqGrabber;
	SGChannel 			gVideoChannel;
	Rect				videoRect;
	bool 				bSgInited;
	string				deviceName;
	SGGrabCompleteBottleUPP	myGrabCompleteProc;

	bool				qtInitSeqGrabber();
	bool				qtCloseSeqGrabber();
	bool				qtSelectDevice(int deviceNumber, bool didWeChooseADevice);
};



class ofQTPlayerAPI{
public:
	ofQTPlayerAPI(){};
	virtual ~ofQTPlayerAPI(){};

	bool 			isFrameNew()=0;
	void			close()=0;
	bool			loadMovie(string name)=0;

	void			play();
	void			stop();

	void			update()=0;

	float			getPosition()=0;
	float			getSpeed()=0;
	float			getDuration()=0;
	bool			getIsMovieDone()=0;
	int				getCurrentFrame()=0;
	int				getTotalNumFrames()=0;

	void			setPosition(float pct)=0;
	void			setVolume(int volume)=0;
	void			setLoopState(int state)=0;
	void			setSpeed(float speed)=0;
	void			setFrame(int frame)=0;  // frame 0 = first frame...
	void			setPaused(bool bPause)=0;

	bool			isPaused()=0;
	bool			isLoaded()=0;
	bool			isPlaying()=0;

	void			firstFrame()=0;
	void			nextFrame()=0;
	void			previousFrame()=0;

	unsigned char * getPixels()=0;

	int 			getHeight()=0;
	int 			getWidth()=0;




	MovieDrawingCompleteUPP myDrawCompleteProc;
	MovieController  	thePlayer;
	GWorldPtr 			offscreenGWorld;
	Movie 			 	moviePtr;
	unsigned char * 	offscreenGWorldPixels;	// 32 bit: argb (qt k32ARGBPixelFormat)
	void				qtGetFrameCount(Movie & movForcount);

protected:
	bool 				bStarted;
	bool 				bPlaying;
	bool 				bPaused;
	bool 				bIsFrameNew;			// if we are new


	int					nFrames;				// number of frames
	unsigned char * 	pixels;					// 24 bit: rgb
	bool 				bHavePixelsChanged;
	bool				allocated;				// so we know to free pixels or not
	float  				speed;
	bool 				bLoaded;

	void 				start();
	void 				createImgMemAndGWorld();
};

#endif

#endif
