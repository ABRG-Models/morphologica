#include "timemanager.h"
#include <iostream>

using namespace morph::softmats;

TimeManager* TimeManager::instance_ = nullptr;

TimeManager* TimeManager::getInstance(){
    if( instance_ ==   nullptr )
        instance_ = new TimeManager();

    return instance_;
}

void TimeManager::tic(){
    this->start = std::chrono::steady_clock::now();
}
void TimeManager::toc(){
    this->end = std::chrono::steady_clock::now();
    int ellapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::cout << "Elapsed: " 
              << (ellapsed*1e-6) 
              << "s\n";
}