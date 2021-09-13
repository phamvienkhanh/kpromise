#ifndef _KASYNC_PROMISE_HPP_
#define _KASYNC_PROMISE_HPP_

#include <any>
#include <mutex>
#include <queue>
#include <vector>
#include <thread>
#include <functional>
#include <condition_variable>

namespace kasync {

    class Resolve
    {
    public:
        Resolve() {}
        
        void operator()() {
            
        }

        void operator()(std::any val) {
            m_value = val;
        }

        std::any value() {
            return m_value;
        }
    private:
        std::any m_value;
    };

    class Reject
    {
    public:
        Reject() {}
        void operator()() {
        }
    };

    class PromiseData
    {
    public:
        std::function<void(Resolve&, Reject&)> asyncfunc;
        std::vector<std::function<std::any(std::any)>> listCallback;
        Resolve resolve;
        Reject  reject;
    };

    class PromiseHandler
    {
    public:
        static PromiseHandler* getIntance() {
            if(s_promiseHandler == nullptr) {
                s_promiseHandler = new PromiseHandler();
            }

            return s_promiseHandler;
        }

        void callNext() {
            std::unique_lock<std::mutex> lock(m_mutex);
            if(!m_queue.empty()) {
                auto promiseData = m_queue.front();
                m_queue.pop();
                lock.unlock();
                
                std::any value = promiseData.resolve.value();
                for(auto funcCallback : promiseData.listCallback) {
                    value = funcCallback(value);
                }

            }
        }

        void pushData(PromiseData promiseData) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_queue.push(promiseData);
        }

    private:
        static PromiseHandler* s_promiseHandler;
        std::queue<PromiseData> m_queue;
        std::mutex m_mutex;
    };
    PromiseHandler* PromiseHandler::s_promiseHandler = nullptr;

    class PromiseExecutor
    {
    public:
        static void init() {
            if(s_promiseExecutor == nullptr) {
                s_promiseExecutor = new PromiseExecutor();
            }
        }

        static PromiseExecutor* getIntance() {
            if(s_promiseExecutor == nullptr) {
                s_promiseExecutor = new PromiseExecutor();
            }

            return s_promiseExecutor;
        }

        static void destroy() {
            if(s_promiseExecutor)
                s_promiseExecutor->stopAll();
        }

        void pushJob(PromiseData job) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_queue.push(job);
            lock.unlock();
            m_conditionVar.notify_one();
        }
    private:

        static PromiseExecutor* s_promiseExecutor;

        int m_numWorkers = 2;
        bool m_isStop = false;
        std::mutex m_mutex;
        std::condition_variable m_conditionVar;
        std::vector<std::thread*> m_workers;
        std::queue<PromiseData> m_queue;

        PromiseExecutor() {
            for(int i = 0; i < m_numWorkers; i++)
            {
                std::thread *_thread = new std::thread([&]() {
                    std::unique_lock<std::mutex> lock(m_mutex);

                    do
                    {
                        m_conditionVar.wait(lock, [this]() {
                            return (m_queue.size() || m_isStop);
                        });

                        if (m_queue.size())
                        {
                            auto promiseData = m_queue.front();
                            m_queue.pop();

                            lock.unlock();

                            if (promiseData.asyncfunc != nullptr) {
                                promiseData.asyncfunc(promiseData.resolve, promiseData.reject);
                                PromiseHandler::getIntance()->pushData(promiseData);
                                
                            }
                            
                            lock.lock();
                        }
                    } while (!m_isStop);
                });
                m_workers.push_back(_thread);
                _thread->detach();
            }
        };

        void stopAll() {
            m_isStop = true;
            m_mutex.unlock();
            m_conditionVar.notify_all();

            for(int i = 0; i < m_numWorkers; i++)
            {
                if(m_workers[i]->joinable())
                    m_workers[i]->join();
            }
        };
    };
    PromiseExecutor* PromiseExecutor::s_promiseExecutor = nullptr;
    
    class Promise
    {
    public:
        Promise(std::function<void(Resolve&, Reject&)> asyncfunc) {
            m_promiseData.asyncfunc = asyncfunc;
        }

        Promise then(std::function<std::any(std::any)> callback) {
            m_promiseData.listCallback.push_back(callback);
            return *this;
        }

        void launch() {
            PromiseExecutor::getIntance()->pushJob(m_promiseData);
        }

    private:
        PromiseData m_promiseData;
    };
}

#endif