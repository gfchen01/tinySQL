//
// Created by luke on 22-6-11.
//

#ifndef TIMER_H
#define TIMER_H

#include <chrono>

using namespace std;
using namespace std::chrono;

class TimerClock
{
public:
    TimerClock()
    {
        update();
    }

    ~TimerClock()
    {
    }

    void update()
    {
        _start = high_resolution_clock::now();
    }
    //获取秒
    double getTimerSecond()
    {
        return getTimerMicroSec() * 0.000001;
    }
    //获取毫秒
    double getTimerMilliSec()
    {
        return getTimerMicroSec()*0.001;
    }
    //获取微妙
    long long getTimerMicroSec()
    {
        //当前时钟减去开始时钟的count
        return duration_cast<microseconds>(high_resolution_clock::now() - _start).count();
    }

private:
    time_point<high_resolution_clock>_start;
};

#endif //TIMER_H