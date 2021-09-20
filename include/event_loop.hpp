#ifndef _KLIB_KEVENT_LOOP_
#define _KLIB_KEVENT_LOOP_

#include <memory>
#include <any>
#include <queue>
#include <functional>
#include <mutex>
#include <string>

namespace klib
{

    struct KEvent
    {
        std::string eventType;
        std::any    eventData;
    };    

    class KEventLoop
    {    
    public:
        KEventLoop(std::function<void(KEvent)> eventHandler) 
        {
            if(eventHandler == nullptr)
                throw "error: eventHandler == null";
            
            m_eventHandler = eventHandler;
        }

        void pushEvent(KEvent event) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_eventQueue.push(event);
        }

        void step() {
            std::queue<KEvent> tempQueue;
            std::unique_lock<std::mutex> lock(m_mutex);
            tempQueue = m_eventQueue;
            m_eventQueue = {};
            lock.unlock();

            while (!tempQueue.empty())
            {
                KEvent event{tempQueue.front()};
                tempQueue.pop();
                m_eventHandler(event);
            }            
        }

        ~KEventLoop(){}

    private:
        std::mutex m_mutex;
        std::queue<KEvent> m_eventQueue;
        std::function<void(KEvent)> m_eventHandler;
    };
    
} // namespace klib

#endif



