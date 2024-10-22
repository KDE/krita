/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ASYNC_ACTION_FEEDBACK_H
#define __KIS_ASYNC_ACTION_FEEDBACK_H

#include <QScopedPointer>
#include <functional>
#include "KisImportExportFilter.h"

class QWidget;
class QMutex;

class KisAsyncActionFeedback
{
private:
    struct DefaultWaitingMessageCallback {
        QString operator()() const;
    };

public:
    KisAsyncActionFeedback(const QString &message, QWidget *parent);
    ~KisAsyncActionFeedback();

    KisImportExportErrorCode runAction(std::function<KisImportExportErrorCode()> func);
    void runVoidAction(std::function<void()> func);

    template <typename Mutex>
    void waitForMutex(Mutex &mutex) {
        waitForMutexLikeImpl(std::make_unique<MutexLike<Mutex>>(mutex));
    }

    template<typename Mutex, typename CallbackFunc = DefaultWaitingMessageCallback>
    class MutexWrapper : public Mutex
    {
    public:
        template<typename ...Args>
        MutexWrapper(Args ...args)
            : Mutex(args...)
        {
        }

        void lock() {
            if (!Mutex::try_lock()) {
                KisAsyncActionFeedback f(CallbackFunc{}(), 0);
                f.waitForMutex(static_cast<Mutex&>(*this));
                Mutex::lock();
            }
        }
    };

private:

    /**
     * A simple base for type-erasure wrapper for mutex-like
     * objects.
     */
    struct MutexLikeBase
    {
        virtual ~MutexLikeBase() = default;
        virtual void lock() = 0;
        virtual void unlock() = 0;
        virtual bool try_lock() = 0;
    };

    /**
     * A type-erasure wrapper for mutex-like objects
     */
    template <typename T>
    struct MutexLike : MutexLikeBase
    {
        MutexLike(T& m) : mutex(m) {}

        T &mutex;

        void lock() override {
            mutex.lock();
        }
        void unlock() override {
            mutex.unlock();
        }
        bool try_lock() override {
            return mutex.try_lock();
        }
    };

    void waitForMutexLikeImpl(std::unique_ptr<MutexLikeBase> &&mutex);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_ASYNC_ACTION_FEEDBACK_H */
