#ifndef VIDEO_H
#define VIDEO_H

#include <iostream>
#include "view.h"
#include "opencv2/opencv.hpp"
#include <string>

// plus #include <opencv2/videoio.hpp> ?

namespace morph{ namespace softmats{

class VideoRecorder{
/**
 * Video rendering of the simulation
 *
 * @author Alejandro Jimenez Rodriguez
 */
protected:
	cv::VideoWriter outputVideo;
	int width;
	int height;
	std::string title;
public:
	VideoRecorder( std::string title, int width, int height );
	// Creates the videoWrite object
	void setup();
	// Saves a frame
	void notify();
	// Finishes up the video
	void notifyEnd();
};

}}

#endif
