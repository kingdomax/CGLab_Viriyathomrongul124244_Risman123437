#pragma once
class Timer {
    public:
        Timer();
        double getElapsedTime();

    private:
        double _lastTime;
};