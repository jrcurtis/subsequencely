
#pragma once

#include <chrono>
#include <functional>
#include <thread>
#include <utility>

namespace lpp
{
    class Timer
    {
        static void runTimer(Timer* t)
        {
            while (t->isRunning())
            {
                t->event();
                std::this_thread::sleep_for(t->interval);
            }
        }
        
    public:
        Timer()
            : running(false),
              interval(std::chrono::seconds(1))
        { }
        
        void start(std::chrono::milliseconds i, const std::function<void()>& e)
        {
            {
                std::lock_guard<mutex> lock(runningMutex);
                if (running)
                {
                    return;
                }
                
                running = true;
            }
            
            interval = i;
            event = e;
            timerThread = std::move(std::thread(runTimer, this));
        }
        
        void stop()
        {
            {
                std::lock_guard<mutex> lock(runningMutex);
                running = false;
            }
            timerThread.join();
        }
        
        bool isRunning()
        {
            std::lock_guard<mutex> lock(runningMutex);
            return running;
        }
        
    private:
        bool running;
        std::chrono::milliseconds interval;
        std::function<void()> event;
        std::thread timerThread;
        
        std::mutex runningMutex;
    };
}