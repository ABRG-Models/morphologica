#pragma once

namespace morph{ namespace softmats{

class Config{
private:
    
protected:
    double timeStep;
    unsigned int numIterations;
    Config(){}
    static Config *instance_;
public:
    static Config *getConfig();
    double getTimeStep();
    int getNumIterations();

};

}}