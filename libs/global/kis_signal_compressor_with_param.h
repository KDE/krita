/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SIGNAL_COMPRESSOR_WITH_PARAM_H
#define __KIS_SIGNAL_COMPRESSOR_WITH_PARAM_H

#include <kis_signal_compressor.h>
#include <functional>


/**
 * A special class that converts a Qt signal into a std::function call.
 *
 * Usage:
 *
 *        \code{.cpp}
 *
 *        std::function<void ()> destinationFunctionCall(std::bind(someNiceFunc, firstParam, secondParam));
 *        SignalToFunctionProxy proxy(destinationFunctionCall);
 *        connect(srcObject, SIGNAL(sigSomethingChanged()), &proxy, SLOT(start()));
 *
 *        \endcode
 *
 * Now every time sigSomethingChanged() is emitted, someNiceFunc is
 * called. std::bind allows us to call any method of any class without
 * changing signature of the class or creating special wrappers.
 */
class KRITAGLOBAL_EXPORT SignalToFunctionProxy : public QObject
{
    Q_OBJECT
public:
    using TrivialFunction = std::function<void ()>;

public:
    SignalToFunctionProxy(TrivialFunction function)
        : m_function(function)
    {
    }

public Q_SLOTS:
    void start() {
        m_function();
    }

private:
    TrivialFunction m_function;
};

/**
 * A special class that converts a standard function call into Qt signal
 *
 * Usage:
 *
 *        \code{.cpp}
 *
 *        FunctionToSignalProxy proxy;
 *        connect(&proxy, SIGNAL(timeout()), &dstObject, SLOT(someDestinationSlot()));
 *
 *        \endcode
 *
 *        Now every time `proxy.start()` is called, the signal `timeout()` is
 *        forwarded to `someDestinationSlot()`
 */
class KRITAGLOBAL_EXPORT FunctionToSignalProxy : public QObject
{
    Q_OBJECT

public:
    void start() {
        emit timeout();
    }

Q_SIGNALS:
    void timeout();
};



/**
 * A special class for deferring and comressing events with one
 * parameter of type T. This works like KisSignalCompressor but can
 * handle events with one parameter. Due to limitation of the Qt this
 * doesn't allow signal/slots, so it uses std::function instead.
 *
 * In the end (after a timeout) the latest param value is returned to
 * the callback.
 *
 *        Usage:
 *
 *        \code{.cpp}
 *
 *        using namespace std::placeholders; // For _1 placeholder
 *
 *        // prepare the callback function
 *        std::function<void (qreal)> callback(
 *            std::bind(&LutDockerDock::setCurrentExposureImpl, this, _1));
 *
 *        // Create the compressor object
 *        KisSignalCompressorWithParam<qreal> compressor(40, callback);
 *
 *        // When event comes:
 *        compressor.start(0.123456);
 *
 *        \endcode
 */

template <typename T>
class KisSignalCompressorWithParam
{
public:
    using CallbackFunction = std::function<void (T)>;

public:
KisSignalCompressorWithParam(int delay, CallbackFunction function, KisSignalCompressor::Mode mode = KisSignalCompressor::FIRST_ACTIVE)
        : m_compressor(delay, mode),
          m_function(function)
    {
        std::function<void ()> callback(
            std::bind(&KisSignalCompressorWithParam<T>::fakeSlotTimeout, this));
        m_signalProxy.reset(new SignalToFunctionProxy(callback));

        m_compressor.connect(&m_compressor, SIGNAL(timeout()), m_signalProxy.data(), SLOT(start()));
    }

    ~KisSignalCompressorWithParam()
    {
    }

    void start(T param) {
        m_currentParamValue = param;
        m_compressor.start();
    }

    void stop() {
        m_compressor.stop();
    }

    bool isActive() const {
        return m_compressor.isActive();
    }

    void setDelay(int value) {
        m_compressor.setDelay(value);
    }

private:
    void fakeSlotTimeout() {
        m_function(m_currentParamValue);
    }

private:
    KisSignalCompressor m_compressor;
    CallbackFunction m_function;
    QScopedPointer<SignalToFunctionProxy> m_signalProxy;
    T m_currentParamValue;
};

#endif /* __KIS_SIGNAL_COMPRESSOR_WITH_PARAM_H */
