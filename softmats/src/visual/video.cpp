#include "video.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

using namespace morph::softmats;

VideoRecorder::VideoRecorder(std::string title, int width, int height ){
	this->width = width;
    this->height = height;
    this->title = title;
}

void VideoRecorder::setup(){
    this->outputVideo.open(title + ".avi", cv::VideoWriter::fourcc('M','J','P','G'), 20.0f, cv::Size( this->width, this->height ), true);
}

void VideoRecorder::notify(){
    int height = this->height;
    int width = this->width;
    cv::Mat pixels( height, width, CV_8UC3 );
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data );

    cv::Mat cv_pixels( height, width, CV_8UC3 );

    for( int y = 0; y < height; y++ )
        for( int x=0; x < width; x++ ){
        cv_pixels.at<cv::Vec3b>(y,x)[2] = pixels.at<cv::Vec3b>(height-y-1,x)[0];
        cv_pixels.at<cv::Vec3b>(y,x)[1] = pixels.at<cv::Vec3b>(height-y-1,x)[1];
        cv_pixels.at<cv::Vec3b>(y,x)[0] = pixels.at<cv::Vec3b>(height-y-1,x)[2];
    }
    this->outputVideo << cv_pixels;
}

void VideoRecorder::notifyEnd(){
    this->outputVideo.release();
}
