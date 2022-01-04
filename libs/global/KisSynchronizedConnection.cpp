/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisSynchronizedConnection.h"

#include <QThread>
#include <QCoreApplication>
#include <kis_assert.h>

/**
 * @brief The KisSynchronizedConnectionEventTypeRegistrar is a simple
 * class to register QEvent type in the Qt's system
 */
struct KisSynchronizedConnectionEventTypeRegistrar
{
    KisSynchronizedConnectionEventTypeRegistrar() {
        eventType = QEvent::registerEventType(QEvent::User + 1000);
    }

    int eventType = -1;
};

struct KisBarrierCallbackContainer
{
    std::function<void()> callback;
};

Q_GLOBAL_STATIC(KisSynchronizedConnectionEventTypeRegistrar, s_instance)
Q_GLOBAL_STATIC(KisBarrierCallbackContainer, s_barrier)


/************************************************************************/
/*            KisSynchronizedConnectionEvent                            */
/************************************************************************/

KisSynchronizedConnectionEvent::KisSynchronizedConnectionEvent(QObject *_destination)
    : QEvent(QEvent::Type(s_instance->eventType)),
      destination(_destination)
{
}

KisSynchronizedConnectionEvent::KisSynchronizedConnectionEvent(const KisSynchronizedConnectionEvent &rhs)
    : QEvent(QEvent::Type(s_instance->eventType)),
      destination(rhs.destination)
{
}

KisSynchronizedConnectionEvent::~KisSynchronizedConnectionEvent()
{
}

/************************************************************************/
/*            KisSynchronizedConnectionBase                             */
/************************************************************************/

int KisSynchronizedConnectionBase::eventType()
{
    return s_instance->eventType;
}

void KisSynchronizedConnectionBase::registerSynchronizedEventBarrier(std::function<void ()> callback)
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(!s_barrier->callback);
    s_barrier->callback = callback;
}

bool KisSynchronizedConnectionBase::event(QEvent *event)
{
    if (event->type() == s_instance->eventType) {
        KisSynchronizedConnectionEvent *typedEvent =
                static_cast<KisSynchronizedConnectionEvent*>(event);

        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(typedEvent->destination == this, false);
        deliverEventToReceiver();
        return true;
    }

    return QObject::event(event);
}

void KisSynchronizedConnectionBase::postEvent()
{
    if (QThread::currentThread() == this->thread()) {
        /// TODO: This assert triggers in unittests that don't have a fully-featured
        /// KisApplication object. Perhaps we should add a fake callback to KISTEST_MAIN
        /// KIS_SAFE_ASSERT_RECOVER_NOOP(s_barrier->callback);

        if (s_barrier->callback) {
            s_barrier->callback();
        }

        deliverEventToReceiver();
    } else {
        qApp->postEvent(this, new KisSynchronizedConnectionEvent(this));
    }
}

