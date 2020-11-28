/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
