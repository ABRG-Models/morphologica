#ifndef VIDEO_H
#define VIDEO_H

#include <iostream>

#include "view.h"

#include "opencv/cv.h"
#include "opencv2/opencv.hpp"

namespace morph{ namespace softmats{

class VideoRecorder{

protected:
	cv::VideoWriter outputVideo;
	int width;
	int height;
public:
	VideoRecorder( int width, int height );
	void setup();
	void notify();
	void notifyEnd();
};

}}

#endif