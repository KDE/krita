/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KISSAFEBLOCKINGQUEUECONNECTIONPROXY_H
#define KISSAFEBLOCKINGQUEUECONNECTIONPROXY_H

#include <QObject>
#include <QQueue>
#include <functional>
#include "kis_signal_compressor_with_param.h"
#include "kis_assert.h"
#include "kritaimage_export.h"

namespace KisSafeBlockingQueueConnectionProxyPrivate {
void KRITAIMAGE_EXPORT passBlockingSignalSafely(FunctionToSignalProxy &source, SignalToFunctionProxy &destination);
void KRITAIMAGE_EXPORT initProxyObject(QObject *object);
}


/**
 * A special class for safe forwarding of blocking-queued signal to the GUI thread.
 *
 * The class automatically resolves deadlocks when GUI thread blocks on the image.
 * This tie-breaking algorithm is implemented via KisBusyWaitBroker.
 *
 * Usage:
 *
 *        \code{.cpp}
 *
 *        // create the proxy
 *        KisSafeBlockingQueueConnectionProxy<QTransform> proxy(
 *            std::bind(&KisShapeLayer::slotTransformShapes, shapeLayer));
 *
 *        // emit synchronous signal with deadlock-avoidance
 *        proxy.start(QTransform::fromScale(0.5, 0.5));
 *
 *        \endcode
 */
template <typename T>
class KisSafeBlockingQueueConnectionProxy
{
    using CallbackFunction = std::function<void (T)>;
public:
    KisSafeBlockingQueueConnectionProxy(CallbackFunction function)
        : m_function(function),
          m_destination(std::bind(&KisSafeBlockingQueueConnectionProxy::fakeSlotTimeout, this))
    {
        KisSafeBlockingQueueConnectionProxyPrivate::initProxyObject(&m_source);
        KisSafeBlockingQueueConnectionProxyPrivate::initProxyObject(&m_destination);

        QObject::connect(&m_source, SIGNAL(timeout()), &m_destination, SLOT(start()), Qt::BlockingQueuedConnection);
    }

    void start(T value) {
        const int sanityQueueSize = m_value.size();

        m_value.enqueue(value);
        KisSafeBlockingQueueConnectionProxyPrivate::passBlockingSignalSafely(m_source, m_destination);

        KIS_SAFE_ASSERT_RECOVER_NOOP(m_value.size() == sanityQueueSize);
    }

private:
    void fakeSlotTimeout() {
        KIS_SAFE_ASSERT_RECOVER_RETURN(!m_value.isEmpty());
        m_function(m_value.dequeue());
    }

private:
    CallbackFunction m_function;
    FunctionToSignalProxy m_source;
    SignalToFunctionProxy m_destination;
    QQueue<T> m_value;
};

/**
 * An override of KisSafeBlockingQueueConnectionProxy for forwarding signals
 * without any parameters.
 */
template <>
class KisSafeBlockingQueueConnectionProxy<void>
{
    using CallbackFunction = std::function<void ()>;
public:
    KisSafeBlockingQueueConnectionProxy(CallbackFunction function)
        : m_function(function),
          m_destination(std::bind(&KisSafeBlockingQueueConnectionProxy::fakeSlotTimeout, this))
    {
        KisSafeBlockingQueueConnectionProxyPrivate::initProxyObject(&m_source);
        KisSafeBlockingQueueConnectionProxyPrivate::initProxyObject(&m_destination);

        QObject::connect(&m_source, SIGNAL(timeout()), &m_destination, SLOT(start()), Qt::BlockingQueuedConnection);
    }

    void start() {
        KisSafeBlockingQueueConnectionProxyPrivate::passBlockingSignalSafely(m_source, m_destination);
    }

private:
    void fakeSlotTimeout() {
        m_function();
    }

private:
    CallbackFunction m_function;
    FunctionToSignalProxy m_source;
    SignalToFunctionProxy m_destination;
};

#endif // KISSAFEBLOCKINGQUEUECONNECTIONPROXY_H
