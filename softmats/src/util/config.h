#pragma once

namespace morph{ namespace softmats{
/**
 * Global properties of the simulation. <Singleton>
 * 
 * @author Alejandro Jimenez Rodriguez
 */
class Config{
private:
    
protected:
    // Step size of the euler integration
    double timeStep;
    // Number of iteration in the PBD solver
    unsigned int numIterations;
    Config(){}
    static Config *instance_;
public:
    static Config *getConfig();
    double getTimeStep();
    int getNumIterations();

};

}}