/*------------------------------------------------------------------------
  Junction: Concurrent data structures in C++
  Copyright (c) 2016 Jeff Preshing
  Distributed under the Simplified BSD License.
  Original location: https://github.com/preshing/junction
  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the LICENSE file for more information.
------------------------------------------------------------------------*/

#ifndef QSBR_H
#define QSBR_H

#include <QVector>
#include <QMutex>
#include <QMutexLocker>
#include <tiles3/kis_lockless_stack.h>

#define CALL_MEMBER(obj, pmf) ((obj).*(pmf))

class QSBR
{
private:
    struct Action {
        void (*func)(void*);
        quint64 param[4]; // Size limit found experimentally. Verified by assert below.

        Action() = default;

        Action(void (*f)(void*), void* p, quint64 paramSize) : func(f)
        {
            KIS_ASSERT(paramSize <= sizeof(param)); // Verify size limit.
            memcpy(&param, p, paramSize);
        }

        void operator()()
        {
            func(&param);
        }
    };

    QAtomicInt m_rawPointerUsers;
    QAtomicInt m_poolSize;
    KisLocklessStack<Action> m_pendingActions;
    KisLocklessStack<Action> m_migrationReclaimActions;
    std::atomic_flag m_isProcessing = ATOMIC_FLAG_INIT;

    void releasePoolSafely(KisLocklessStack<Action> *pool) {
        Action action;

        while (pool->pop(action)) {
            action();
        }
    }

public:

    template <class T>
    void enqueue(void (T::*pmf)(), T* target, bool migration = false)
    {
        struct Closure {
            void (T::*pmf)();
            T* target;

            static void thunk(void* param)
            {
                Closure* self = (Closure*) param;
                CALL_MEMBER(*self->target, self->pmf)();
            }
        };


        Closure closure = {pmf, target};

        if (migration) {
            m_migrationReclaimActions.push(Action(Closure::thunk, &closure, sizeof(closure)));
        } else {
            m_pendingActions.push(Action(Closure::thunk, &closure, sizeof(closure)));
        }

        m_poolSize.ref();
    }

    void update(bool migration)
    {
        if (m_rawPointerUsers.testAndSetAcquire(0, 1)) {
            releasePoolSafely(&m_pendingActions);
            m_poolSize.store(0);

            if (!migration) {
                releasePoolSafely(&m_migrationReclaimActions);
            }

            m_rawPointerUsers.deref();

        } else if (m_poolSize > 4098) {
            // TODO: make pool size limit configurable!

            while (!m_rawPointerUsers.testAndSetAcquire(0, 1));

            releasePoolSafely(&m_pendingActions);
            m_poolSize.store(0);

            m_rawPointerUsers.deref();
        }
    }

    void flush()
    {
        while (!m_rawPointerUsers.testAndSetAcquire(0, 1));

        releasePoolSafely(&m_pendingActions);
        m_poolSize.store(0);

        releasePoolSafely(&m_migrationReclaimActions);

        m_rawPointerUsers.deref();
    }

    void lockRawPointerAccess()
    {
        m_rawPointerUsers.ref();
    }

    void unlockRawPointerAccess()
    {
        m_rawPointerUsers.deref();
    }
};

#endif // QSBR_H
