#include "config.h"

using namespace morph::softmats;

Config* Config::instance_ = nullptr;

Config* Config::getConfig(){
    if( instance_ == nullptr ){
        instance_ = new Config();
        instance_->timeStep = 0.01; // hardcoding meanwhile
        instance_->numIterations = 1;
    }

    return instance_;
}

double Config::getTimeStep(){
    return timeStep;
}

int Config::getNumIterations(){
    return numIterations;
}