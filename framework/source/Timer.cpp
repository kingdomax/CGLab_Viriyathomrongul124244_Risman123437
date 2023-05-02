#include "Timer.hpp"
#include <GLFW/glfw3.h>

Timer::Timer() {
    _lastTime = glfwGetTime();
}

double Timer::getElapsedTime() {
    double currentTime = glfwGetTime();
    double elapsedTime = currentTime - _lastTime;
    _lastTime = currentTime;
    return elapsedTime;
}