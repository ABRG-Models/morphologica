#pragma once
#include <chrono>

namespace morph{namespace softmats{
typedef std::chrono::steady_clock::time_point mtime_point;

class TimeManager{
/**
 * Manages time related aspects of the simulation 
 * 
 * So far only used for timing processes
 * 
 * @author Alejandro Jimenez Rodriguez
 */
private:
    mtime_point start;
    mtime_point end;
protected:
    TimeManager(){

    }
    static TimeManager* instance_;
public:
    static TimeManager* getInstance();
    // Starts recording time
    void tic();
    // Reports ellapsed time
    void toc();
};

}}