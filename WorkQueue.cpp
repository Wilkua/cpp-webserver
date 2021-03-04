#include <WorkQueue.h>

namespace web
{
    template <typename T>
    void WorkQueue<T>::dispatchWork(T workPiece)
    {
        std::lock_guard<std::mutex> queueLock(m_queueMutex);
        m_queue.push(workPiece);

        m_queueCv.notify_one();
    }

    template <typename T>
    void WorkQueue<T>::dispatchWork(T *work, size_t count)
    {
        std::lock_guard<std::mutex> queueLock(m_queueMutex);
        T *end = work + (sizeof(T) * count);
        for (T *i = work; i < end; i += sizeof(T))
            m_queue.push(*i);

        m_queueCv.notify_all();
    }

    template <typename T>
    T WorkQueue<T>::findWork()
    {
        std::unique_lock<std::mutex> queueLock(m_queueMutex);
        m_queueCv.wait(queueLock, [=]{ return !(m_queue.empty()); });

        T workPiece = m_queue.front();
        m_queue.pop();
        queueLock.unlock(); // drop lock as quickly as possible

        return workPiece;
    }
} // namespace web

