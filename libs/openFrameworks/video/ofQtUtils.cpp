#include "ofQtUtils.h"
#include "ofUtils.h"

#ifndef TARGET_LINUX
static bool bQuicktimeInitialized = false;

//----------------------------------------
void initializeQuicktime(){
	if (bQuicktimeInitialized == false){

		//----------------------------------
		// do we have quicktime installed at all?
		// http://www.apple.com/quicktime/download/win.html
		// can gestalt help with versions, or is that only after init?

		OSErr myErr 	= noErr;
		#ifdef TARGET_WIN32
			myErr = InitializeQTML(0);
			if (myErr != noErr){
				ofLog(OF_LOG_ERROR, "-----------------------------------------------------");
				ofLog(OF_LOG_ERROR, "sorry, there is a problem with quicktime starting up... please check!");
                OF_EXIT_APP(0);
			}
		#endif
		myErr = EnterMovies ();
		if (myErr != noErr){
			ofLog(OF_LOG_ERROR, "-----------------------------------------------------");
			ofLog(OF_LOG_ERROR, "sorry, there is a problem with quicktime starting up... please check!");
			OF_EXIT_APP(0);
		}

		bQuicktimeInitialized = true;
	}
}

//----------------------------------------
void closeQuicktime(){
	if (bQuicktimeInitialized == true){
		ExitMovies();
		#ifdef TARGET_WIN32
			TerminateQTML();
		#endif
		bQuicktimeInitialized = false;
	}
}


//----------------------------------------
void convertPixels(unsigned char * gWorldPixels, unsigned char * rgbPixels, int w, int h){

	// ok for macs?
	// ok for intel macs?

	int * rgbaPtr 			= (int *) gWorldPixels;
	pix24 * rgbPtr 			= (pix24 *) rgbPixels;
		unsigned char * rgbaStart;

	//	putting in the boolean, so we can work on
	//	0,0 in top right...
	//	bool bFlipVertically 	= true;

	bool bFlipVertically 	= false;
	
	// -------------------------------------------
	// we flip vertically because the 0,0 position in OF
	// is the bottom left (not top left, like processing)
	// since the 0,0 of a picture is top left
	// if we upload and drawf the data as is
	// it will be upside-down....
	// -------------------------------------------

	if (!bFlipVertically){
		//----- argb->rgb
		for (int i = 0; i < h; i++){
			pix24 * rgbPtr 			= (pix24 *) rgbPixels + ((i) * w);
			for (int j = 0; j < w; j++){
				rgbaStart = (unsigned char *)rgbaPtr;
				memcpy (rgbPtr, rgbaStart+1, sizeof(pix24));
				rgbPtr++;
				rgbaPtr++;
			}
		}
	} else {
		//----- flip while argb->rgb
		for (int i = 0; i < h; i++){
			pix24 * rgbPtr 			= (pix24 *) rgbPixels + ((h-i-1) * w);
			for (int j = 0; j < w; j++){
				rgbaStart = (unsigned char *)rgbaPtr;
				memcpy (rgbPtr, rgbaStart+1, sizeof(pix24));
				rgbPtr++;
				rgbaPtr++;
			}
		}
	}
}


//----------------------------------------
// osx needs this for modal dialogs.
Boolean SeqGrabberModalFilterUPP (DialogPtr theDialog, const EventRecord *theEvent, short *itemHit, long refCon){
	#pragma unused(theDialog, itemHit)
  	Boolean  handled = false;
  	if ((theEvent->what == updateEvt) &&
    ((WindowPtr) theEvent->message == (WindowPtr) refCon))
  	{
    	BeginUpdate ((WindowPtr) refCon);
    	EndUpdate ((WindowPtr) refCon);
    	handled = true;
  	}
  	return (handled);
}


#define   kCharacteristicHasVideoFrameRate		FOUR_CHAR_CODE('vfrr')
#define   kCharacteristicIsAnMpegTrack			FOUR_CHAR_CODE('mpeg')

/*

Calculate the static frame rate for a given movie.

*/
void MovieGetStaticFrameRate(Movie inMovie, double *outStaticFrameRate)
{

  *outStaticFrameRate = 0;

  Media movieMedia;
  MediaHandler movieMediaHandler;
  /* get the media identifier for the media that contains the first
    video track's sample data, and also get the media handler for
    this media. */
  MovieGetVideoMediaAndMediaHandler(inMovie, &movieMedia, &movieMediaHandler);
  if (movieMedia && movieMediaHandler)
  {
    Boolean isMPEG = false;
    /* is this the MPEG-1/MPEG-2 media handler? */
    OSErr err = IsMPEGMediaHandler(movieMediaHandler, &isMPEG);
    if (err == noErr)
    {
      if (isMPEG) /* working with MPEG-1/MPEG-2 media */
      {
        Fixed staticFrameRate;
        ComponentResult err = MPEGMediaGetStaticFrameRate(movieMediaHandler, &staticFrameRate);
        if (err == noErr)
        {
          /* convert Fixed data result to type double */
          *outStaticFrameRate = Fix2X(staticFrameRate);
        }
      }
      else  /* working with non-MPEG-1/MPEG-2 media */
      {
        OSErr err = MediaGetStaticFrameRate(movieMedia, outStaticFrameRate);
        if (err != noErr) ofLog(OF_LOG_ERROR, "error in MediaGetStaticFrameRate, ofQtUtils");
        //assert(err == noErr);
      }
    }
  }
}

/*

Get the media identifier for the media that contains the first
video track's sample data, and also get the media handler for
this media.

*/
void MovieGetVideoMediaAndMediaHandler(Movie inMovie, Media *outMedia, MediaHandler *outMediaHandler)
{

  *outMedia = NULL;
  *outMediaHandler = NULL;

  /* get first video track */
  Track videoTrack = GetMovieIndTrackType(inMovie, 1, kCharacteristicHasVideoFrameRate,
              movieTrackCharacteristic | movieTrackEnabledOnly);
  if (videoTrack != NULL)
  {
    /* get media ref. for track's sample data */
    *outMedia = GetTrackMedia(videoTrack);
    if (*outMedia)
    {
      /* get a reference to the media handler component */
      *outMediaHandler = GetMediaHandler(*outMedia);
    }
  }
}

/*

Return true if media handler reference is from the MPEG-1/MPEG-2 media handler.
Return false otherwise.

*/
OSErr IsMPEGMediaHandler(MediaHandler inMediaHandler, Boolean *outIsMPEG)
{

  	/* is this the MPEG-1/MPEG-2 media handler? */
	return((OSErr) MediaHasCharacteristic(inMediaHandler,
					kCharacteristicIsAnMpegTrack,
					outIsMPEG));
}

/*

Given a reference to the media handler used for media in a MPEG-1/MPEG-2
track, return the static frame rate.

*/
ComponentResult MPEGMediaGetStaticFrameRate(MediaHandler inMPEGMediaHandler, Fixed *outStaticFrameRate)
{
  *outStaticFrameRate = 0;

  MHInfoEncodedFrameRateRecord encodedFrameRate;
  Size encodedFrameRateSize = sizeof(encodedFrameRate);

    /* get the static frame rate */
  ComponentResult err = MediaGetPublicInfo(inMPEGMediaHandler,
                       kMHInfoEncodedFrameRate,
                       &encodedFrameRate,
                       &encodedFrameRateSize);
  if (err == noErr)
  {
    /* return frame rate at which the track was encoded */
    *outStaticFrameRate = encodedFrameRate.encodedFrameRate;
  }

  return err;
}

/*

Given a reference to the media that contains the sample data for a track,
calculate the static frame rate.

*/
OSErr MediaGetStaticFrameRate(Media inMovieMedia, double *outFPS)
{
  *outFPS = 0;

    /* get the number of samples in the media */
  long sampleCount = GetMediaSampleCount(inMovieMedia);
  OSErr err = GetMoviesError();

  if (sampleCount && err == noErr)
  {
      /* find the media duration */

    //Quicktime 7.0 code
    //TimeValue64 duration = GetMediaDisplayDuration(inMovieMedia);
    TimeValue64 duration = GetMediaDuration(inMovieMedia);


    err = GetMoviesError();
    if (err == noErr)
    {
        /* get the media time scale */
      TimeValue64 timeScale = GetMediaTimeScale(inMovieMedia);
      err = GetMoviesError();
      if (err == noErr)
      {
        /* calculate the frame rate:
          frame rate = (sample count * media time scale) / media duration
          */
        *outFPS = (double)sampleCount * (double)timeScale / (double)duration;
      }
    }
  }

  return err;
}


//----------------------------------------

#ifdef TARGET_OSX
// GetSettingsPreference
//    Returns a preference for a specified key as QuickTime UserData
// It is your responsibility to dispose of the returned UserData
OSErr GetSettingsPreference(CFStringRef inKey, UserData *outUserData)
{
  CFPropertyListRef theCFSettings;
  Handle            theHandle = NULL;
  UserData          theUserData = NULL;
  OSErr             err = paramErr;

  // read the new setttings from our preferences
  theCFSettings = CFPreferencesCopyAppValue(inKey,
                                         kCFPreferencesCurrentApplication);
  if (theCFSettings) {
    err = PtrToHand(CFDataGetBytePtr((CFDataRef)theCFSettings), &theHandle,
                    CFDataGetLength((CFDataRef)theCFSettings));

    CFRelease(theCFSettings);
    if (theHandle) {
      err = NewUserDataFromHandle(theHandle, &theUserData);
      if (theUserData) {
        *outUserData = theUserData;
      }
      DisposeHandle(theHandle);
    }
  }

  return err;
}

//----------------------------------------
// SaveSettingsPreference
//    Saves a preference for a specified key from QuickTime UserData
OSErr SaveSettingsPreference(CFStringRef inKey, UserData inUserData)
{
  CFDataRef theCFSettings;
  Handle    hSettings;
  OSErr     err;

  if (NULL == inUserData) return paramErr;

  hSettings = NewHandle(0);
  err = MemError();

  if (noErr == err) {
    err = PutUserDataIntoHandle(inUserData, hSettings);

    if (noErr == err) {
      HLock(hSettings);

      theCFSettings = CFDataCreate(kCFAllocatorDefault,
                                   (UInt8 *)*hSettings,
                                   GetHandleSize(hSettings));
      if (theCFSettings) {
        CFPreferencesSetAppValue(inKey, theCFSettings,
                                 kCFPreferencesCurrentApplication);
        CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
        CFRelease(theCFSettings);
      }
    }

    DisposeHandle(hSettings);
  }

  return err;
}

static ComponentResult 	frameIsGrabbedProc(SGChannel sgChan, short nBufferNum, Boolean *pbDone, long lRefCon);
static ComponentResult  frameIsGrabbedProc(SGChannel sgChan, short nBufferNum, Boolean *pbDone, long lRefCon){

 	ComponentResult err = SGGrabFrameComplete( sgChan, nBufferNum, pbDone );

	bool * havePixChanged = (bool *)lRefCon;
	*havePixChanged = true;

	return err;
}

void ofQTGrabberAPI::ofQTGrabberAPI(){
	initializeQuicktime();
	bSgInited				= false;
	pixels 					= NULL;
	width 					= 320;	// default setting
	height 					= 240;	// default setting
	deviceID				= 0;
	gSeqGrabber				= NULL;
	offscreenGWorldPixels	= NULL;
}

void ofQTGrabberAPI:~ofQTGrabberAPI(){
	if (offscreenGWorldPixels != NULL){
		delete[] offscreenGWorldPixels;
	}
	close();
}

void ofQTGrabberAPI::listDevices(){
	bool bNeedToInitGrabberFirst = false;

	if (!bSgInited) bNeedToInitGrabberFirst = true;

	//if we need to initialize the grabbing component then do it
	if( bNeedToInitGrabberFirst ){
		if( !qtInitSeqGrabber() ){
			return;
		}
	}

	printf("-------------------------------------\n");

	/*
		//input selection stuff (ie multiple webcams)
		//from http://developer.apple.com/samplecode/SGDevices/listing13.html
		//and originally http://lists.apple.com/archives/QuickTime-API/2008/Jan/msg00178.html
	*/

	SGDeviceList deviceList;
	SGGetChannelDeviceList (gVideoChannel, sgDeviceListIncludeInputs, &deviceList);
	unsigned char pascalName[256];
	unsigned char pascalNameInput[256];

	//this is our new way of enumerating devices
	//quicktime can have multiple capture 'inputs' on the same capture 'device'
	//ie the USB Video Class Video 'device' - can have multiple usb webcams attached on what QT calls 'inputs'
	//The isight for example will show up as:
	//USB Video Class Video - Built-in iSight ('input' 1 of the USB Video Class Video 'device')
	//Where as another webcam also plugged whill show up as
	//USB Video Class Video - Philips SPC 1000NC Webcam ('input' 2 of the USB Video Class Video 'device')

	//this means our the device ID we use for selection has to count both capture 'devices' and their 'inputs'
	//this needs to be the same in our init grabber method so that we select the device we ask for
	int deviceCount = 0;

	ofLog(OF_LOG_NOTICE, "listing available capture devices");
	for(int i = 0 ; i < (*deviceList)->count ; ++i)
	{
		SGDeviceName nameRec;
		nameRec = (*deviceList)->entry[i];
		SGDeviceInputList deviceInputList = nameRec.inputs;

		int numInputs = 0;
		if( deviceInputList ) numInputs = ((*deviceInputList)->count);

		memcpy(pascalName, (*deviceList)->entry[i].name, sizeof(char) * 256);

		//this means we can use the capture method
		if(nameRec.flags != sgDeviceNameFlagDeviceUnavailable){

			//if we have a capture 'device' (qt's word not mine - I would prefer 'system' ) that is ready to be used
			//we go through its inputs to list all physical devices - as there could be more than one!
			for(int j = 0; j < numInputs; j++){


				//if our 'device' has inputs we get their names here
				if( deviceInputList ){
					SGDeviceInputName inputNameRec  = (*deviceInputList)->entry[j];
					memcpy(pascalNameInput, inputNameRec.name, sizeof(char) * 256);
				}

				printf( "device[%i] %s - %s\n",  deviceCount, p2cstr(pascalName), p2cstr(pascalNameInput) );

				//we count this way as we need to be able to distinguish multiple inputs as devices
				deviceCount++;
			}

		}else{
			printf( "(unavailable) device[%i] %s\n",  deviceCount, p2cstr(pascalName) );
			deviceCount++;
		}
	}
	printf( "-------------------------------------\n");

	//if we initialized the grabbing component then close it
	if( bNeedToInitGrabberFirst ){
		qtCloseSeqGrabber();
	}
}

bool ofQTGrabberAPI::isFrameNew(){

}

void ofQTGrabberAPI::close(){

	qtCloseSeqGrabber();
	DisposeSGGrabCompleteBottleUPP(myGrabCompleteProc);

	if (pixels != NULL){
		delete[] pixels;
		pixels = NULL;
	}
}

bool ofQTGrabberAPI::initGrabber(int w, int h, int framerate=-1){
	//---------------------------------- 1 - open the sequence grabber
	if( !qtInitSeqGrabber() ){
		ofLog(OF_LOG_ERROR, "error: unable to initialize the seq grabber");
		return false;
	}

	//---------------------------------- 2 - set the dimensions
	width 		= w;
	height 		= h;

	MacSetRect(&videoRect, 0, 0, width, height);

	//---------------------------------- 3 - buffer allocation
	// Create a buffer big enough to hold the video data,
	// make sure the pointer is 32-byte aligned.
	// also the rgb image that people will grab

	offscreenGWorldPixels 	= (unsigned char*)malloc(4 * width * height + 32);
	pixels					= new unsigned char[width*height*3];

	#if defined(TARGET_OSX) && defined(__BIG_ENDIAN__)
		QTNewGWorldFromPtr (&(videogworld), k32ARGBPixelFormat, &(videoRect), NULL, NULL, 0, (offscreenGWorldPixels), 4 * width);
	#else
		QTNewGWorldFromPtr (&(videogworld), k24RGBPixelFormat, &(videoRect), NULL, NULL, 0, (pixels), 3 * width);
	#endif

	LockPixels(GetGWorldPixMap(videogworld));
	SetGWorld (videogworld, NULL);
	SGSetGWorld(gSeqGrabber, videogworld, nil);


	//---------------------------------- 4 - device selection
	bool didWeChooseADevice = bChooseDevice;
	bool deviceIsSelected	=  false;

	//if we have a device selected then try first to setup
	//that device
	if(didWeChooseADevice){
		deviceIsSelected = qtSelectDevice(deviceID, true);
		if(!deviceIsSelected && bVerbose) ofLog(OF_LOG_WARNING, "unable to open device[%i] - will attempt other devices", deviceID);
	}

	//if we couldn't select our required device
	//or we aren't specifiying a device to setup
	//then lets try to setup ANY device!
	if(deviceIsSelected == false){
		//lets list available devices
		listDevices();

		setDeviceID(0);
		deviceIsSelected = qtSelectDevice(deviceID, false);
	}

	//if we still haven't been able to setup a device
	//we should error and stop!
	if( deviceIsSelected == false){
		goto bail;
	}

	//---------------------------------- 5 - final initialization steps
	OSStatus err;

	err = SGSetChannelUsage(gVideoChannel,seqGrabPreview);
	if ( err != noErr ) goto bail;


	//----------------- callback method for notifying new frame
	err = SGSetChannelRefCon(gVideoChannel, (long)&bHavePixelsChanged );
	if(!err) {

		VideoBottles vb;
		/* get the current bottlenecks */
		vb.procCount = 9;
		err = SGGetVideoBottlenecks(gVideoChannel, &vb);
		if (!err) {
			myGrabCompleteProc = NewSGGrabCompleteBottleUPP(frameIsGrabbedProc);
			vb.grabCompleteProc = myGrabCompleteProc;

			/* add our GrabFrameComplete function */
			err = SGSetVideoBottlenecks(gVideoChannel, &vb);
		}

	}

	err = SGSetChannelBounds(gVideoChannel, &videoRect);
	if ( err != noErr ) goto bail;

	err = SGPrepare(gSeqGrabber,  true, false); //theo swapped so preview is true and capture is false
	if ( err != noErr ) goto bail;

	err = SGStartPreview(gSeqGrabber);
	if ( err != noErr ) goto bail;

	bGrabberInited = true;
	loadSettings();

	if( framerate >= 0 ){
		err = SGSetFrameRate(gVideoChannel, IntToFixed(framerate) );
		if ( err != noErr ){
			ofLog(OF_LOG_ERROR,"initGrabber error setting framerate to %i", framerate);
		}
	}

	ofLog(OF_LOG_NOTICE,"end setup ofVideoGrabber");
	ofLog(OF_LOG_NOTICE,"-------------------------------------\n");

	return true;

	//--------------------- (bail) something's wrong -----
	bail:

		ofLog(OF_LOG_ERROR, "***** ofVideoGrabber error *****");
		ofLog(OF_LOG_ERROR, "-------------------------------------\n");

		//if we don't close this - it messes up the next device!
		if(bSgInited) qtCloseSeqGrabber();

		bGrabberInited = false;
		return false;

}

void ofQTGrabberAPI::videoSettings(){
	Rect curBounds, curVideoRect;
	ComponentResult	err;

	// Get our current state
	err = SGGetChannelBounds (gVideoChannel, &curBounds);
	if (err != noErr){
		ofLog(OF_LOG_ERROR, "Error in SGGetChannelBounds");
		return;
	}
	err = SGGetVideoRect (gVideoChannel, &curVideoRect);
	if (err != noErr){
		ofLog(OF_LOG_ERROR, "Error in SGGetVideoRect");
		return;
	}

	// Pause
	err = SGPause (gSeqGrabber, true);
	if (err != noErr){
		ofLog(OF_LOG_ERROR, "Error in SGPause");
		return;
	}

	#ifdef TARGET_OSX
		//load any saved camera settings from file
		loadSettings();

		static SGModalFilterUPP gSeqGrabberModalFilterUPP = NewSGModalFilterUPP(SeqGrabberModalFilterUPP);
		ComponentResult result = SGSettingsDialog(gSeqGrabber,  gVideoChannel, 0, nil, 0, gSeqGrabberModalFilterUPP, nil);
		if (result != noErr){
			ofLog(OF_LOG_ERROR, "error in  dialogue");
			return;
		}

		//save any changed settings to file
		saveSettings();
	#else
		SGSettingsDialog(gSeqGrabber, gVideoChannel, 0, nil, seqGrabSettingsPreviewOnly, NULL, 0);
	#endif

	SGSetChannelBounds(gVideoChannel, &videoRect);
	SGPause (gSeqGrabber, false);
}

unsigned char * ofQTGrabberAPI::getPixels(){

}

void ofQTGrabberAPI::setDeviceID(int _deviceID){
	if(didWeChooseADevice)	deviceID = _deviceID;
}

//--------------------------------------------------------
void ofQTGrabberAPI::update(){
	if (bGrabberInited == true){
		SGIdle(gSeqGrabber);
		// set the top pixel alpha = 0, so we can know if it
		// was a new frame or not..
		// or else we will process way more than necessary
		// (ie opengl is running at 60fps +, capture at 30fps)
		if (bHavePixelsChanged){

			#if defined(TARGET_OSX) && defined(__BIG_ENDIAN__)
				convertPixels(offscreenGWorldPixels, pixels, width, height);
			#endif

			if (bUseTexture){
				tex.loadData(pixels, width, height, GL_RGB);
			}
		}
	}

	// newness test for quicktime:
	if (bGrabberInited == true){
		bIsFrameNew = false;
		if (bHavePixelsChanged == true){
			bIsFrameNew = true;
			bHavePixelsChanged = false;
		}
	}
}

//--------------------------------------------------------
int ofQTGrabberAPI::getHeight(){
	return height;
}

//--------------------------------------------------------
int ofQTGrabberAPI::getWidth(){
	return width;
}


//--------------------------------------------------------------------
#ifdef TARGET_OSX
//--------------------------------------------------------------------


//---------------------------------------------------------------------
bool ofQTGrabberAPI::saveSettings(){

		if (bGrabberInited != true) return false;
		ComponentResult	err;

		UserData mySGVideoSettings = NULL;
		// get the SGChannel settings cofigured by the user
		err = SGGetChannelSettings(gSeqGrabber, gVideoChannel, &mySGVideoSettings, 0);
		if ( err != noErr ){
			ofLog(OF_LOG_ERROR, "Error getting camera settings %i",err);
			return false;
		}
		string pref = "ofVideoSettings-"+deviceName;
		CFStringRef cameraString = CFStringCreateWithCString(kCFAllocatorDefault,pref.c_str(),kCFStringEncodingMacRoman);

		//get the settings using the key "ofVideoSettings-the name of the device"
		SaveSettingsPreference( cameraString, mySGVideoSettings);
		DisposeUserData(mySGVideoSettings);
		return true;

}

//---------------------------------------------------------------------
bool ofQTGrabberAPI::loadSettings(){

   if (bGrabberInited != true || deviceName.length() == 0) return false;

   ComponentResult   err;
   UserData mySGVideoSettings = NULL;

   // get the settings using the key "ofVideoSettings-the name of the device"
   string pref = "ofVideoSettings-"+deviceName;
   CFStringRef cameraString = CFStringCreateWithCString(kCFAllocatorDefault,pref.c_str(),kCFStringEncodingMacRoman);

   GetSettingsPreference(cameraString, &mySGVideoSettings);
   if (mySGVideoSettings){

      Rect   curBounds, curVideoRect;

      //we need to make sure the dimensions don't get effected
      //by our preferences

      // Get our current state
      err = SGGetChannelBounds (gVideoChannel, &curBounds);
      if (err != noErr){
         ofLog(OF_LOG_ERROR, "Error in SGGetChannelBounds");
      }
      err = SGGetVideoRect (gVideoChannel, &curVideoRect);
      if (err != noErr){
         ofLog(OF_LOG_ERROR, "Error in SGGetVideoRect");
      }

      // use the saved settings preference to configure the SGChannel
      err = SGSetChannelSettings(gSeqGrabber, gVideoChannel, mySGVideoSettings, 0);
      if ( err != noErr ) {
         ofLog(OF_LOG_ERROR, "Error applying stored settings %i", err);
         return false;
      }
      DisposeUserData(mySGVideoSettings);

      // Pause
      err = SGPause (gSeqGrabber, true);
      if (err != noErr){
         ofLog(OF_LOG_ERROR, "Error in SGPause");
      }
      SGSetChannelBounds(gVideoChannel, &videoRect);
      SGPause (gSeqGrabber, false);

   }else{
      ofLog(OF_LOG_WARNING, "No camera settings to load");
      return false;
   }
   return true;
}


//------------------------------------------------------
bool ofQTGrabberAPI::qtInitSeqGrabber(){

		if (bSgInited != true){

			OSErr err = noErr;

			ComponentDescription	theDesc;
			Component				sgCompID;

			// this crashes when we get to
			// SGNewChannel
			// we get -9405 as error code for the channel
			// -----------------------------------------
			// gSeqGrabber = OpenDefaultComponent(SeqGrabComponentType, 0);

			// this seems to work instead (got it from hackTV)
			// -----------------------------------------
			theDesc.componentType = SeqGrabComponentType;
			theDesc.componentSubType = NULL;
			theDesc.componentManufacturer = 'appl';
			theDesc.componentFlags = NULL;
			theDesc.componentFlagsMask = NULL;
			sgCompID = FindNextComponent (NULL, &theDesc);
			// -----------------------------------------

			if (sgCompID == NULL){
				ofLog(OF_LOG_ERROR, "error:FindNextComponent did not return a valid component");
				return false;
			}

			gSeqGrabber = OpenComponent(sgCompID);

			err = GetMoviesError();
			if (gSeqGrabber == NULL || err) {
				ofLog(OF_LOG_ERROR, "error: can't get default sequence grabber component");
				return false;
			}

			err = SGInitialize(gSeqGrabber);
			if (err != noErr) {
				ofLog(OF_LOG_ERROR, "error: can't initialize sequence grabber component");
				return false;
			}

			err = SGSetDataRef(gSeqGrabber, 0, 0, seqGrabDontMakeMovie);
			if (err != noErr) {
				ofLog(OF_LOG_ERROR, "error: can't set the destination data reference");
				return false;
			}

			// windows crashes w/ out gworld, make a dummy for now...
			// this took a long time to figure out.
			err = SGSetGWorld(gSeqGrabber, 0, 0);
			if (err != noErr) {
				ofLog(OF_LOG_ERROR, "error: setting up the gworld");
				return false;
			}

			err = SGNewChannel(gSeqGrabber, VideoMediaType, &(gVideoChannel));
			if (err != noErr) {
				ofLog(OF_LOG_ERROR, "error: creating a channel.  Check if you have any qt capable cameras attached");
				return false;
			}

			bSgInited = true;
			return true;
		}

		return false;
}

//--------------------------------------------------------------------
bool ofQTGrabberAPI::qtCloseSeqGrabber(){

	if (gSeqGrabber != NULL){
		SGStop (gSeqGrabber);
		CloseComponent (gSeqGrabber);
		gSeqGrabber = NULL;
		bSgInited = false;
		return true;
	}

	return false;

}

//--------------------------------------------------------------------
bool ofQTGrabberAPI::qtSelectDevice(int deviceNumber, bool didWeChooseADevice){

	//note - check for memory freeing possibly needed for the all SGGetChannelDeviceList mac stuff
	// also see notes in listDevices() regarding new enunemeration method.

	//Generate a device list and enumerate
	//all devices availble to the channel
	SGDeviceList deviceList;
	SGGetChannelDeviceList(gVideoChannel, sgDeviceListIncludeInputs, &deviceList);

	unsigned char pascalName[256];
	unsigned char pascalNameInput[256];

	int numDevices = (*deviceList)->count;
	if(numDevices == 0){
		ofLog(OF_LOG_ERROR, "error: No catpure devices found");
		return false;
	}

	int deviceCount = 0;
	for(int i = 0 ; i < numDevices; ++i)
	{
		SGDeviceName nameRec;
		nameRec = (*deviceList)->entry[i];
		SGDeviceInputList deviceInputList = nameRec.inputs;

		int numInputs = 0;
		if( deviceInputList ) numInputs = ((*deviceInputList)->count);

		memcpy(pascalName, (*deviceList)->entry[i].name, sizeof(char) * 256);
		memset(pascalNameInput, 0, sizeof(char)*256);

		//this means we can use the capture method
		if(nameRec.flags != sgDeviceNameFlagDeviceUnavailable){

			//if we have a capture 'device' (qt's word not mine - I would prefer 'system' ) that is ready to be used
			//we go through its inputs to list all physical devices - as there could be more than one!
			for(int j = 0; j < numInputs; j++){

				//if our 'device' has inputs we get their names here
				if( deviceInputList ){
					SGDeviceInputName inputNameRec  = (*deviceInputList)->entry[j];
					memcpy(pascalNameInput, inputNameRec.name, sizeof(char) * 256);
				}

				//if the device number matches we try and setup the device
				//if we didn't specifiy a device then we will try all devices till one works!
				if( deviceCount == deviceNumber || !didWeChooseADevice ){
					ofLog(OF_LOG_NOTICE, "attempting to open device[%i] %s   -   %s",  deviceCount, p2cstr(pascalName), p2cstr(pascalNameInput) );

					OSErr err1 = SGSetChannelDevice(gVideoChannel, pascalName);
					OSErr err2 = SGSetChannelDeviceInput(gVideoChannel, j);

					int successLevel = 0;

					//if there were no errors then we have opened the device without issue
					if ( err1 == noErr && err2 == noErr){
						successLevel = 2;
					}
						//parameter errors are not fatal so we will try and open but will caution the user
					else if ( (err1 == paramErr || err1 == noErr) && (err2 == noErr || err2 == paramErr) ){
						successLevel = 1;
					}

					//the device is opened!
					if ( successLevel > 0 ){

						deviceName = (char *)p2cstr(pascalName);
						deviceName  += "-";
						deviceName +=  (char *)p2cstr(pascalNameInput);

						if(successLevel == 2)ofLog(OF_LOG_NOTICE, "device opened successfully");
						else ofLog(OF_LOG_WARNING, "device opened with some paramater errors - should be fine though!");

						//no need to keep searching - return that we have opened a device!
						return true;

					}else{
						//if we selected a device in particular but failed we want to go through the whole list again - starting from 0 and try any device.
						//so we return false - and try one more time without a preference
						if( didWeChooseADevice ){
							ofLog(OF_LOG_WARNING, "problems setting device[%i] %s - %s *****", deviceNumber, p2cstr(pascalName), p2cstr(pascalNameInput));
							return false;
						}else{
							ofLog(OF_LOG_WARNING, "unable to open device, trying next device");
						}
					}

				}

				//we count this way as we need to be able to distinguish multiple inputs as devices
				deviceCount++;
			}
		}else{
			//ofLog(OF_LOG_ERROR, "(unavailable) device[%i] %s",  deviceCount, p2cstr(pascalName) );
			deviceCount++;
		}
	}

	return false;
}

//--------------------------------------------------------------------
#endif
//--------------------------------------------------------------------
















// video player


//--------------------------------------------------------
bool  	createMovieFromPath(char * path, Movie &movie);
bool 	createMovieFromPath(char * path, Movie &movie){



	Boolean 	isdir			= false;
	OSErr 		result 			= 0;
	FSSpec 		theFSSpec;

	short 		actualResId 	= DoTheRightThing;

	#ifdef TARGET_WIN32
		result = NativePathNameToFSSpec (path, &theFSSpec, 0);
		if (result != noErr) {
			ofLog(OF_LOG_ERROR,"NativePathNameToFSSpec failed %d", result);
			ofLog(OF_LOG_ERROR,"Error loading movie");
			return false;
		}

	#endif



	#ifdef TARGET_OSX
		FSRef 		fsref;
		result = FSPathMakeRef((const UInt8*)path, &fsref, &isdir);
		if (result) {
			ofLog(OF_LOG_ERROR,"FSPathMakeRef failed %d", result);
			ofLog(OF_LOG_ERROR,"Error loading movie");
			return false;
		}
		result = FSGetCatalogInfo(&fsref, kFSCatInfoNone, NULL, NULL, &theFSSpec, NULL);
		if (result) {
			ofLog(OF_LOG_ERROR,"FSGetCatalogInfo failed %d", result);
			ofLog(OF_LOG_ERROR,"Error loading movie");
			return false;
		}
	#endif


	short movieResFile;
	result = OpenMovieFile (&theFSSpec, &movieResFile, fsRdPerm);
	if (result == noErr) {

		short   movieResID = 0;
		result = NewMovieFromFile(&movie, movieResFile, &movieResID, (unsigned char *) 0, newMovieActive, (Boolean *) 0);
		if (result == noErr){
			CloseMovieFile (movieResFile);
		} else {
			ofLog(OF_LOG_ERROR,"NewMovieFromFile failed %d", result);
			return false;
		}
	} else {
		ofLog(OF_LOG_ERROR,"OpenMovieFile failed %d", result);
		return false;
	}

	return true;
}

//--------------------------------------------------------------
bool createMovieFromURL(string urlIn,  Movie &movie){
	char * url = (char *)urlIn.c_str();
	Handle urlDataRef;

	OSErr err;

	urlDataRef = NewHandle(strlen(url) + 1);
	if ( ( err = MemError()) != noErr){ ofLog(OF_LOG_ERROR,"createMovieFromURL: error creating url handle"); return false;}

	BlockMoveData(url, *urlDataRef, strlen(url) + 1);

	err = NewMovieFromDataRef(&movie, newMovieActive,nil, urlDataRef, URLDataHandlerSubType);
	DisposeHandle(urlDataRef);

	if(err != noErr){
		ofLog(OF_LOG_ERROR,"createMovieFromURL: error loading url");
		return false;
	}else{
		return true;
	}

	return false;
}



//--------------------------------------------------------------
OSErr 	DrawCompleteProc(Movie theMovie, long refCon);
OSErr 	DrawCompleteProc(Movie theMovie, long refCon){

	ofVideoPlayer * ofvp = (ofVideoPlayer *)refCon;

	#if defined(TARGET_OSX) && defined(__BIG_ENDIAN__)
		convertPixels(ofvp->offscreenGWorldPixels, ofvp->pixels, ofvp->width, ofvp->height);
	#endif

	ofvp->bHavePixelsChanged = true;
	if (ofvp->bUseTexture == true){
		ofvp->tex.loadData(ofvp->pixels, ofvp->width, ofvp->height, GL_RGB);
	}

	return noErr;
}

//--------------------------------------------------------
ofQTPlayerAPI::ofQTPlayerAPI{
	bLoaded 					= false;
	speed 						= 1;
	bStarted					= false;
	pixels						= NULL;
	nFrames						= 0;
	bPaused						= true;
	bIsFrameNew					= false;

	moviePtr	 				= NULL;
	allocated 					= false;
    offscreenGWorld				= NULL;
}

//--------------------------------------------------------
ofQTPlayerAPI::~ofQTPlayerAPI{
	close();
	if(allocated)	delete[] pixels;
	if(allocated)	delete[] offscreenGWorldPixels;
	if ((offscreenGWorld)) DisposeGWorld((offscreenGWorld));
}

//--------------------------------------------------------
bool ofQTPlayerAPI::isFrameNew(){
	return bIsFrameNew;
}

//--------------------------------------------------------
void ofQTPlayerAPI::close(){

	if (bLoaded == true){

	    DisposeMovie (moviePtr);
		DisposeMovieDrawingCompleteUPP(myDrawCompleteProc);

		moviePtr = NULL;

		if(pixels){
			delete[] pixels;
			pixels = NULL;
		}

	    bLoaded = false;
    }
}

//--------------------------------------------------------
bool ofQTPlayerAPI::loadMovie(string name){
	initializeQuicktime();			// init quicktime
	closeMovie();					// if we have a movie open, close it
	bLoaded 				= false;	// try to load now

	if( name.substr(0, 7) == "http://" || name.substr(0,7) == "rtsp://" ){
		if(! createMovieFromURL(name, moviePtr) ) return false;
	}else{
		name 					= ofToDataPath(name);
		if( !createMovieFromPath((char *)name.c_str(), moviePtr) ) return false;
	}

	bool bDoWeAlreadyHaveAGworld = false;
	if (width != 0 && height != 0){
		bDoWeAlreadyHaveAGworld = true;
	}
	Rect 				movieRect;
	GetMovieBox(moviePtr, &(movieRect));
	if (bDoWeAlreadyHaveAGworld){
		// is the gworld the same size, then lets *not* de-allocate and reallocate:
		if (width == movieRect.right &&
			height == movieRect.bottom){
			SetMovieGWorld (moviePtr, offscreenGWorld, nil);
		} else {
			width 	= movieRect.right;
			height 	= movieRect.bottom;
			delete(pixels);
			delete(offscreenGWorldPixels);
			if ((offscreenGWorld)) DisposeGWorld((offscreenGWorld));
			createImgMemAndGWorld();
		}
	} else {
		width	= movieRect.right;
		height 	= movieRect.bottom;
		createImgMemAndGWorld();
	}

	if (moviePtr == NULL){
		return false;
	}

	//----------------- callback method
	myDrawCompleteProc = NewMovieDrawingCompleteUPP (DrawCompleteProc);
	SetMovieDrawingCompleteProc (moviePtr, movieDrawingCallWhenChanged,  myDrawCompleteProc, (long)this);

	// ------------- get the total # of frames:
	nFrames				= 0;
	TimeValue			curMovieTime;
	curMovieTime		= 0;
	TimeValue			duration;

	//OSType whichMediaType	= VIDEO_TYPE; // mingw chokes on this
	OSType whichMediaType	= FOUR_CHAR_CODE('vide');

	short flags				= nextTimeMediaSample + nextTimeEdgeOK;

	while( curMovieTime >= 0 ) {
		nFrames++;
		GetMovieNextInterestingTime(moviePtr,flags,1,&whichMediaType,curMovieTime,0,&curMovieTime,&duration);
		flags = nextTimeMediaSample;
	}
	nFrames--; // there's an extra time step at the end of themovie




	// ------------- get some pixels in there ------
	GoToBeginningOfMovie(moviePtr);
	SetMovieActiveSegment(moviePtr, -1,-1);
	MoviesTask(moviePtr,0);

	#if defined(TARGET_OSX) && defined(__BIG_ENDIAN__)
		convertPixels(offscreenGWorldPixels, pixels, width, height);
	#endif

	bStarted 				= false;
	bLoaded 				= true;
	bPlaying 				= false;
	bHavePixelsChanged 		= false;
	speed 					= 1;

	allocated 				= true;

	return true;
}

//--------------------------------------------------------
void ofQTPlayerAPI::start(){

	if (bLoaded == true && bStarted == false){
		SetMovieActive(moviePtr, true);

		//------------------ set the movie rate to default
		//------------------ and preroll, so the first frames come correct

		TimeValue timeNow 	= 	GetMovieTime(moviePtr, 0);
		Fixed playRate 		=	GetMoviePreferredRate(moviePtr); 		//Not being used!

		PrerollMovie(moviePtr, timeNow, X2Fix(speed));
		SetMovieRate(moviePtr,  X2Fix(speed));
		setLoopState(OF_LOOP_NORMAL);

		// get some pixels in there right away:
		MoviesTask(moviePtr,0);
		#if defined(TARGET_OSX) && defined(__BIG_ENDIAN__)
			convertPixels(offscreenGWorldPixels, pixels, width, height);
		#endif
		bHavePixelsChanged = true;
		if (bUseTexture == true){
			tex.loadData(pixels, width, height, GL_RGB);
		}

		bStarted = true;
		bPlaying = true;
	}

}

//--------------------------------------------------------
void ofQTPlayerAPI::play(){
	bPlaying = true;
	bPaused = false;
	if (!bStarted){
		start();
	}else {
		// ------------ lower level "startMovie"
		// ------------ use desired speed & time (-1,1,etc) to Preroll...
		TimeValue timeNow;
		timeNow = GetMovieTime(moviePtr, nil);
		PrerollMovie(moviePtr, timeNow, X2Fix(speed));
		SetMovieRate(moviePtr,  X2Fix(speed));
		MoviesTask(moviePtr, 0);
	}
}

//--------------------------------------------------------
void ofQTPlayerAPI::stop(){
	StopMovie (moviePtr);
	SetMovieActive (moviePtr, false);
	bStarted = false;
	bPlaying = false;
}

//--------------------------------------------------------
void ofQTPlayerAPI::update(){
	if(bLoaded){
		#if defined(TARGET_WIN32) || defined(QT_USE_MOVIETASK)
			MoviesTask(moviePtr,0);
		#endif

		// ---------------------------------------------------
		// 		on all platforms,
		// 		do "new"ness ever time we idle...
		// 		before "isFrameNew" was clearning,
		// 		people had issues with that...
		// 		and it was badly named so now, newness happens
		// 		per-idle not per isNew call
		// ---------------------------------------------------

		bIsFrameNew = bHavePixelsChanged;
		if (bHavePixelsChanged == true) {
			bHavePixelsChanged = false;
		}

	}
}

//--------------------------------------------------------
float ofQTPlayerAPI::getPosition(){
	long total 		= GetMovieDuration(moviePtr);
	long current 	= GetMovieTime(moviePtr, nil);
	float pct 		= ((float)current/(float)total);
	return pct;
}

//--------------------------------------------------------
float ofQTPlayerAPI::getSpeed(){
	return speed;
}

//--------------------------------------------------------
float ofQTPlayerAPI::getDuration(){
	return (float) (GetMovieDuration (moviePtr) / (double) GetMovieTimeScale (moviePtr));
}

//--------------------------------------------------------
bool ofQTPlayerAPI::getIsMovieDone(){
	bool bIsMovieDone = (bool)IsMovieDone(moviePtr);
	return bIsMovieDone;
}

//--------------------------------------------------------
int	ofQTPlayerAPI::getCurrentFrame(){

	int frame = 0;

	// zach I think this may fail on variable length frames...
	float pos = getPosition();


	float  framePosInFloat = ((float)getTotalNumFrames() * pos);
	int    framePosInInt = (int)framePosInFloat;
	float  floatRemainder = (framePosInFloat - framePosInInt);
	if (floatRemainder > 0.5f) framePosInInt = framePosInInt + 1;
	//frame = (int)ceil((getTotalNumFrames() * getPosition()));
	frame = framePosInInt;

	return frame;
}

//--------------------------------------------------------
int	ofQTPlayerAPI::getTotalNumFrames(){
	return nFrames;
}

//--------------------------------------------------------
void ofQTPlayerAPI::setPosition(float pct){
 	TimeRecord tr;
 	tr.base 		= GetMovieTimeBase(moviePtr);
	long total 		= GetMovieDuration(moviePtr );
	long newPos 	= (long)((float)total * pct);
	SetMovieTimeValue(moviePtr, newPos);
	MoviesTask(moviePtr,0);
}

//--------------------------------------------------------
void ofQTPlayerAPI::setVolume(int volume){
	SetMovieVolume(moviePtr, volume);
}

//--------------------------------------------------------
void ofQTPlayerAPI::setLoopState(int state){
	TimeBase myTimeBase;
	long myFlags = 0L;
	myTimeBase = GetMovieTimeBase(moviePtr);
	myFlags = GetTimeBaseFlags(myTimeBase);
	switch (state) {
		case OF_LOOP_NORMAL:
			myFlags |= loopTimeBase;
			myFlags &= ~palindromeLoopTimeBase;
			SetMoviePlayHints(moviePtr, hintsLoop, hintsLoop);
			SetMoviePlayHints(moviePtr, 0L, hintsPalindrome);
			break;
		case OF_LOOP_PALINDROME:
			myFlags |= loopTimeBase;
			myFlags |= palindromeLoopTimeBase;
			SetMoviePlayHints(moviePtr, hintsLoop, hintsLoop);
			SetMoviePlayHints(moviePtr, hintsPalindrome, hintsPalindrome);
			break;
		case OF_LOOP_NONE:
			default:
			myFlags &= ~loopTimeBase;
			myFlags &= ~palindromeLoopTimeBase;
			SetMoviePlayHints(moviePtr, 0L, hintsLoop |
			hintsPalindrome);
			break;
	}
	SetTimeBaseFlags(myTimeBase, myFlags);
}

//--------------------------------------------------------
void ofQTPlayerAPI::setSpeed(float speed){
	if (bPlaying == true){
		//setMovieRate actually plays, so let's call it only when we are playing
		SetMovieRate(moviePtr, X2Fix(speed));
	}
}

//--------------------------------------------------------
void ofQTPlayerAPI::setFrame(int frame){

	// frame 0 = first frame...

	// this is the simple way...
	//float durationPerFrame = getDuration() / getTotalNumFrames();

	// seems that freezing, doing this and unfreezing seems to work alot
	// better then just SetMovieTimeValue() ;

	if (!bPaused) SetMovieRate(moviePtr, X2Fix(0));

	// this is better with mpeg, etc:
	double frameRate = 0;
	double movieTimeScale = 0;
	MovieGetStaticFrameRate(moviePtr, &frameRate);
	movieTimeScale = GetMovieTimeScale(moviePtr);

	if (frameRate > 0){
		double frameDuration = 1 / frameRate;
		TimeValue t = (TimeValue)(frame * frameDuration * movieTimeScale);
		SetMovieTimeValue(moviePtr, t);
		MoviesTask(moviePtr, 0);
	}

   if (!bPaused) SetMovieRate(moviePtr, X2Fix(speed));

}

//--------------------------------------------------------
void ofQTPlayerAPI::setPaused(bool bPause){
	bPaused = _bPause;
	// there might be a more "quicktime-ish" way (or smarter way)
	// to do this for now, to pause, just set the movie's speed to zero,
	// on un-pause, set the movie's speed to "speed"
	// (and hope that speed != 0...)
	if (bPlaying == true){
		if (bPaused == true) 	SetMovieRate(moviePtr, X2Fix(0));
		else 					SetMovieRate(moviePtr, X2Fix(speed));
	}
}

//--------------------------------------------------------
bool ofQTPlayerAPI::isPaused(){
	return bPaused;
}

//--------------------------------------------------------
bool ofQTPlayerAPI::isLoaded(){
	return bLoaded;
}

//--------------------------------------------------------
bool ofQTPlayerAPI::isPlaying(){
	return bPlaying;
}

//--------------------------------------------------------
void ofQTPlayerAPI::firstFrame(){
	setFrame(0);
}

//--------------------------------------------------------
void ofQTPlayerAPI::nextFrame(){
	setFrame(getCurrentFrame() + 1);
}

//--------------------------------------------------------
void ofQTPlayerAPI::previousFrame(){
	setFrame(getCurrentFrame() - 1);
}

//--------------------------------------------------------
unsigned char * ofQTPlayerAPI::getPixels(){
	return pixels;
}

//--------------------------------------------------------
int ofQTPlayerAPI::getHeight(){
	return height;
}

//--------------------------------------------------------
int ofQTPlayerAPI::getWidth(){
	return width;
}

//--------------------------------------------------------
void ofQTPlayerAPI::createImgMemAndGWorld(){
	Rect movieRect;
	movieRect.top 			= 0;
	movieRect.left 			= 0;
	movieRect.bottom 		= height;
	movieRect.right 		= width;
	offscreenGWorldPixels 	= new unsigned char[4 * width * height + 32];
	pixels					= new unsigned char[width*height*3];

	#if defined(TARGET_OSX) && defined(__BIG_ENDIAN__)
		QTNewGWorldFromPtr (&(offscreenGWorld), k32ARGBPixelFormat, &(movieRect), NULL, NULL, 0, (offscreenGWorldPixels), 4 * width);
	#else
		QTNewGWorldFromPtr (&(offscreenGWorld), k24RGBPixelFormat, &(movieRect), NULL, NULL, 0, (pixels), 3 * width);
	#endif

	LockPixels(GetGWorldPixMap(offscreenGWorld));
	SetGWorld (offscreenGWorld, NULL);
	SetMovieGWorld (moviePtr, offscreenGWorld, nil);
	//------------------------------------ texture stuff:
	if (bUseTexture){
		// create the texture, set the pixels to black and
		// upload them to the texture (so at least we see nothing black the callback)
		tex.allocate(width,height,GL_RGB);
		memset(pixels, 0, width*height*3);
		tex.loadData(pixels, width, height, GL_RGB);
		allocated = true;
	}
}


//end mac specific stuff
#endif

#endif
