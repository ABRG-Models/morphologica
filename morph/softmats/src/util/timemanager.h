#pragma once
#include <chrono>

namespace morph{namespace softmats{
typedef std::chrono::steady_clock::time_point mtime_point;

class TimeManager{
private:
    mtime_point start;
    mtime_point end;
protected:
    TimeManager(){

    }
    static TimeManager* instance_;
public:
    static TimeManager* getInstance();

    void tic();
    void toc();
};

}}