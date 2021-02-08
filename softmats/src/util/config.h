#pragma once
#include <string>
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
    // Location of the mesh
    std::string meshLocation;
    // Location of the shader
    std::string shaderLocation;
    Config();
    ~Config();
    static Config *instance_;
public:
    static std::string configPath;
    static Config *getConfig();
    double getTimeStep();
    int getNumIterations();
    std::string getMeshLocation();
    std::string getShaderLocation();

};

}}