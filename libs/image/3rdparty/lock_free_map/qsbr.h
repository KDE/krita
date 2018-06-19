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

struct Property {
    Property() = default;
    virtual ~Property() = default;
};

template <class T>
class GarbageCollector
{
public:
    ~GarbageCollector()
    {
        cleanUpNodes();
    }

    void enqueue(T *data)
    {
        m_rawPointerUsers.ref();

        Node *newNode = new Node(data);
        Node *head;

        do {
            head = m_freeNodes.loadAcquire();
            newNode->next = head;
        } while (!m_freeNodes.testAndSetRelease(head, newNode));

        m_rawPointerUsers.deref();
    }

    void update()
    {
        m_rawPointerUsers.ref();

        if (m_rawPointerUsers == 1) {
            Node *head = m_freeNodes.loadAcquire();
            if (!(reinterpret_cast<std::uintptr_t>(head) & 1)) {
                if (m_freeNodes.testAndSetOrdered(head, reinterpret_cast<Node *>(reinterpret_cast<std::uintptr_t>(head) | 1))) {
                    if (m_rawPointerUsers == 1) {
                        if (m_freeNodes.testAndSetOrdered(reinterpret_cast<Node *>(reinterpret_cast<std::uintptr_t>(head) | 1), 0)) {
                            cleanUpNodes();
                        }
                    } else {
                        m_freeNodes.testAndSetOrdered(reinterpret_cast<Node *>(reinterpret_cast<std::uintptr_t>(head) | 1), head);
                    }
                }
            }
        }

        m_rawPointerUsers.deref();
    }

private:
    struct Node {
        Node(T *data) : d(data) {}

        Node *next;
        T* d;
    };

    inline void cleanUpNodes()
    {
        Node *head = m_freeNodes.fetchAndStoreOrdered(0);
        if (head) {
            freeList(head);
        }
    }

    inline void freeList(Node *first)
    {
        Node *next;
        while (first) {
            next = first->next;
            delete first;
            first = next;
        }
    }

private:
    QAtomicInt m_rawPointerUsers;
    QAtomicPointer<Node> m_freeNodes;
};

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

    struct Status {
        qint16 inUse : 1;
        qint16 wasIdle : 1;
        qint16 nextFree : 14;

        Status() : inUse(1), wasIdle(0), nextFree(0)
        {
        }
    };

    QMutex m_mutex;
    QVector<Status> m_status;
    qint64 m_freeIndex;
    qint64 m_numContexts;
    qint64 m_remaining;
    QVector<Action> m_deferredActions;
    QVector<Action> m_pendingActions;

    void onAllQuiescentStatesPassed(QVector<Action>& actions)
    {
        // m_mutex must be held
        actions.swap(m_pendingActions);
        m_pendingActions.swap(m_deferredActions);
        m_remaining = m_numContexts;

        for (qint32 i = 0; i < m_status.size(); i++) {
            m_status[i].wasIdle = 0;
        }
    }

public:
    typedef qint16 Context;

    QSBR() : m_freeIndex(-1), m_numContexts(0), m_remaining(0)
    {
    }

    Context createContext()
    {
        QMutexLocker guard(&m_mutex);
        m_numContexts++;
        m_remaining++;
        Q_ASSERT(m_numContexts < (1 << 14));
        qint64 context = m_freeIndex;

        if (context >= 0) {
            Q_ASSERT(context < (qint64) m_status.size());
            Q_ASSERT(!m_status[context].inUse);
            m_freeIndex = m_status[context].nextFree;
            m_status[context] = Status();
        } else {
            context = m_status.size();
            m_status.append(Status());
        }

        return context;
    }

    void destroyContext(QSBR::Context context)
    {
        QVector<Action> actions;
        {
            QMutexLocker guard(&m_mutex);
            Q_ASSERT(context < m_status.size());
            if (m_status[context].inUse && !m_status[context].wasIdle) {
                Q_ASSERT(m_remaining > 0);
                --m_remaining;
            }

            m_status[context].inUse = 0;
            m_status[context].nextFree = m_freeIndex;
            m_freeIndex = context;
            m_numContexts--;

            if (m_remaining == 0) {
                onAllQuiescentStatesPassed(actions);
            }
        }

        for (qint32 i = 0; i < actions.size(); i++) {
            actions[i]();
        }
    }

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
        m_deferredActions.append(Action(Closure::thunk, &closure, sizeof(closure)));
    }

    void update(QSBR::Context context)
    {
        QVector<Action> actions;
        {
            QMutexLocker guard(&m_mutex);
            Q_ASSERT(context < m_status.size());
            Status& status = m_status[context];
            Q_ASSERT(status.inUse);

            if (status.wasIdle) {
                return;
            }

            status.wasIdle = 1;
            Q_ASSERT(m_remaining > 0);

            if (--m_remaining > 0) {
                return;
            }

            onAllQuiescentStatesPassed(actions);
        }

        for (qint32 i = 0; i < actions.size(); i++) {
            actions[i]();
        }
    }

    void flush()
    {
        // This is like saying that all contexts are quiescent,
        // so we can issue all actions at once.
        // No lock is taken.
        for (qint32 i = 0; i < m_pendingActions.size(); i++) {
            m_pendingActions[i]();
        }

        m_pendingActions.clear();

        for (qint32 i = 0; i < m_deferredActions.size(); i++) {
            m_deferredActions[i]();
        }

        m_deferredActions.clear();
        m_remaining = m_numContexts;
    }
};

#endif // QSBR_H
