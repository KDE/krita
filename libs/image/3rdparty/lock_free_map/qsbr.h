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
    KisLocklessStack<Action> m_pendingActions;
    KisLocklessStack<Action> m_migrationReclaimActions;

    void releasePoolSafely(KisLocklessStack<Action> *pool, bool force = false) {
        KisLocklessStack<Action> tmp;
        tmp.mergeFrom(*pool);
        if (tmp.isEmpty()) return;

        if (force || tmp.size() > 4096) {
            while (m_rawPointerUsers.loadAcquire());

            Action action;
            while (tmp.pop(action)) {
                action();
            }
        } else {
            if (!m_rawPointerUsers.loadAcquire()) {
                Action action;
                while (tmp.pop(action)) {
                    action();
                }
            } else {
                // push elements back to the source
                pool->mergeFrom(tmp);
            }
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
    }

    void update()
    {
        releasePoolSafely(&m_pendingActions);
        releasePoolSafely(&m_migrationReclaimActions);
    }

    void flush()
    {
        releasePoolSafely(&m_pendingActions, true);
        releasePoolSafely(&m_migrationReclaimActions, true);
    }

    void lockRawPointerAccess()
    {
        m_rawPointerUsers.ref();
    }

    void unlockRawPointerAccess()
    {
        m_rawPointerUsers.deref();
    }

    bool sanityRawPointerAccessLocked() const {
        return m_rawPointerUsers.loadAcquire();
    }
};

#endif // QSBR_H
