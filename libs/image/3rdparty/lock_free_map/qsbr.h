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
    QVector<Action> m_actions;

public:

    template <class T>
    void enqueue(void (T::*pmf)(), T* target)
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
        QMutexLocker guard(&m_mutex);
        m_actions.append(Action(Closure::thunk, &closure, sizeof(closure)));
    }

    void update()
    {
        QVector<Action> actions;
        {
            QMutexLocker guard(&m_mutex);
            actions.swap(m_actions);
        }

        for (auto &action : actions) {
            action();
        }
    }

    void flush()
    {
        for (auto &action : m_actions) {
            action();
        }
    }
};

#endif // QSBR_H
