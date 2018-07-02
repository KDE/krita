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
            Q_ASSERT(paramSize <= sizeof(param)); // Verify size limit.
            memcpy(&param, p, paramSize);
        }

        void operator()()
        {
            func(&param);
        }
    };

    QMutex m_mutex;
    QVector<Action> m_pendingActions;
    QVector<Action> m_deferedActions;
    std::atomic_flag m_isProcessing = ATOMIC_FLAG_INIT;

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
        while (m_isProcessing.test_and_set(std::memory_order_acquire)) {
        }

        if (migration) {
            m_deferedActions.append(Action(Closure::thunk, &closure, sizeof(closure)));
        } else {
            m_pendingActions.append(Action(Closure::thunk, &closure, sizeof(closure)));
        }

        m_isProcessing.clear(std::memory_order_release);
    }

    void update(bool migration)
    {
        if (!m_isProcessing.test_and_set(std::memory_order_acquire)) {
            QVector<Action> actions;
            actions.swap(m_pendingActions);

            if (!migration) {
                m_pendingActions.swap(m_deferedActions);
            }

            m_isProcessing.clear(std::memory_order_release);

            for (auto &action : actions) {
                action();
            }
        }
    }

    void flush()
    {
        if (!m_isProcessing.test_and_set(std::memory_order_acquire)) {
            for (auto &action : m_pendingActions) {
                action();
            }

            for (auto &action : m_deferedActions) {
                action();
            }

            m_isProcessing.clear(std::memory_order_release);
        }
    }
};

#endif // QSBR_H
