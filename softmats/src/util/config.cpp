#include "config.h"
#include <morph/Config.h>

using namespace morph::softmats;

std::string Config::configPath = ".";
Config* Config::instance_ = nullptr;

Config* Config::getConfig(){
    if( instance_ == nullptr ){
        instance_ = new Config();
        instance_->timeStep = 0.01; // hardcoding meanwhile
        instance_->numIterations = 1;
    }

    return instance_;
}

Config::Config(){
    morph::Config conf(configPath);

    if( !conf.ready ){
        std::cerr << "Configuration file softmats.json not found\n";
        return;
    }

    this->meshLocation = conf.getString("mesh", ".");
    this->shaderLocation = conf.getString("shaderPath", ".");
    std::cout << "config location: " << configPath << "\n";
    std::cout << "Mesh location: " << meshLocation << "\n";
    std::cout << "Shader location: " << shaderLocation << "\n";
}

Config::~Config(){

}

std::string Config::getMeshLocation(){
    return this->meshLocation;
}

std::string Config::getShaderLocation(){
    return this->shaderLocation;
}

double Config::getTimeStep(){
    return timeStep;
}

int Config::getNumIterations(){
    return numIterations;
}