#include "ofDSUtils.h"
#ifdef TARGET_WIN

ofDSGrabberAPI::ofDSGrabberAPI(){

	bVerbose 			= false;
	bDoWeNeedToResize 	= false;
	pixels 				= NULL;
	width 				= 320;	// default setting
	height 				= 240;	// default setting
	deviceID			= 0;
};
virtual ~ofDSGrabberAPI(){};

void ofDSGrabberAPI::listDevices(){
	ofLog(OF_LOG_NOTICE, "---");
	VI.listDevices();
	ofLog(OF_LOG_NOTICE, "---");
}

bool ofDSGrabberAPI::isFrameNew(){

}

void ofDSGrabberAPI::close(){
	if (bGrabberInited == true){
		VI.stopDevice(device);
		bGrabberInited = false;
	}
	if (pixels != NULL){
		delete[] pixels;
		pixels = NULL;
	}
}

bool ofDSGrabberAPI::initGrabber(int w, int h, int framerate){
	if (bChooseDevice){
		device = deviceID;
		ofLog(OF_LOG_NOTICE, "choosing %i", deviceID);
	} else {
		device = 0;
	}

	width = w;
	height = h;
	bGrabberInited = false;

	if( framerate >= 0){
		VI.setIdealFramerate(device, framerate);
	}
	bool bOk = VI.setupDevice(device, width, height);

	int ourRequestedWidth = width;
	int ourRequestedHeight = height;

	if (bOk == true){
		bGrabberInited = true;
		width 	= VI.getWidth(device);
		height 	= VI.getHeight(device);

		if (width == ourRequestedWidth && height == ourRequestedHeight){
			bDoWeNeedToResize = false;
		} else {
			bDoWeNeedToResize = true;
			width = ourRequestedWidth;
			height = ourRequestedHeight;
		}


		pixels	= new unsigned char[width * height * 3];

		return true;
	} else {
		ofLog(OF_LOG_ERROR, "error allocating a video device");
		ofLog(OF_LOG_ERROR, "please check your camera with AMCAP or other software");
		bGrabberInited = false;
		return false;
	}
}

void ofDSGrabberAPI::videoSettings(){
	if (bGrabberInited == true) VI.showSettingsWindow(device);
}

unsigned char * ofDSGrabberAPI::getPixels(){

}

void ofDSGrabberAPI::setDeviceID(int _deviceID){

}

void ofDSGrabberAPI::update(){
	if (bGrabberInited == true){
		bIsFrameNew = false;
		if (VI.isFrameNew(device)){

			bIsFrameNew = true;


			/*
				rescale --
				currently this is nearest neighbor scaling
				not the greatest, but fast
				this can be optimized too
				with pointers, etc

				better --
				make sure that you ask for a "good" size....

			*/

			unsigned char * viPixels = VI.getPixels(device, true, true);


			if (bDoWeNeedToResize == true){

				int inputW = VI.getWidth(device);
				int inputH = VI.getHeight(device);

				float scaleW =	(float)inputW / (float)width;
				float scaleH =	(float)inputH / (float)height;

				for(int i=0;i<width;i++){
					for(int j=0;j<height;j++){

						float posx = i * scaleW;
						float posy = j * scaleH;

						/*

						// start of calculating
						// for linear interpolation

						int xbase = (int)floor(posx);
						int xhigh = (int)ceil(posx);
						float pctx = (posx - xbase);

						int ybase = (int)floor(posy);
						int yhigh = (int)ceil(posy);
						float pcty = (posy - ybase);
						*/

						int posPix = (((int)posy * inputW * 3) + ((int)posx * 3));

						pixels[(j*width*3) + i*3    ] = viPixels[posPix  ];
						pixels[(j*width*3) + i*3 + 1] = viPixels[posPix+1];
						pixels[(j*width*3) + i*3 + 2] = viPixels[posPix+2];

					}
				}

			} else {

				memcpy(pixels, viPixels, width*height*3);

			}

			if (bUseTexture){
				tex.loadData(pixels, width, height, GL_RGB);
			}
		}
	}
}

int ofDSGrabberAPI::getHeight(){

}

int ofDSGrabberAPI::getWidth(){

}

#endif

