#ifndef _HEAD_FPSLIMIT_HPP
#define _HEAD_FPSLIMIT_HPP

#include <chrono>
#include <thread>

namespace fps_func {

class FPS_Limiter {
private:
    const std::chrono::duration< double,std::ratio<1,1> > targetFrameTime;
    std::chrono::steady_clock::time_point start,end;
public:
    FPS_Limiter(const double targetFPS):
        targetFrameTime(1.0/targetFPS),
        start(std::chrono::steady_clock::now())
    {}
    void limit(){
        end=std::chrono::steady_clock::now();
        std::chrono::duration< double,std::ratio<1,1> > eplased(end-start);
        if(eplased<targetFrameTime) std::this_thread::sleep_for(targetFrameTime-eplased);
        start=std::chrono::steady_clock::now();
        return;
    }
};

}
#endif