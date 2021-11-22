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

Q_GLOBAL_STATIC(KisSynchronizedConnectionEventTypeRegistrar, s_instance)


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
        deliverEventToReceiver();
    } else {
        qApp->postEvent(this, new KisSynchronizedConnectionEvent(this));
    }
}

