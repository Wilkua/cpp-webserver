#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

namespace web
{
    template <typename T>
    class WorkQueue
    {
    private:
        std::queue<T> m_queue;
        std::mutex m_queueMutex;
        std::condition_variable m_queueCv;

    public:
        WorkQueue() = default;
        virtual ~WorkQueue() = default;

        void dispatchWork(T work);
        void dispatchWork(T *work, size_t count);
        T findWork();
        // T *collectWork(size_t *count);
    };
} // namespace web


#include <WorkQueue.cpp>
