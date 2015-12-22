/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_SIGNAL_COMPRESSOR_WITH_PARAM_H
#define __KIS_SIGNAL_COMPRESSOR_WITH_PARAM_H

#include <kis_signal_compressor.h>
#include <functional>


/**
 * A special class that converts a Qt signal into a std::function call.
 *
 * Example:
 *
 * std::function<void ()> destinationFunctionCall(std::bind(someNiceFunc, firstParam, secondParam));
 * SignalToFunctionProxy proxy(destinationFunctionCall);
 * connect(srcObject, SIGNAL(sigSomethingChanged()), &proxy, SLOT(start()));
 *
 * Now every time sigSomethingChanged() is emitted, someNiceFunc is
 * called. std::bind allows us to call any method of any class without
 * changing signature of the class or creating special wrappers.
 */
class KRITAIMAGE_EXPORT SignalToFunctionProxy : public QObject
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
    KisSignalCompressorWithParam(int delay, CallbackFunction function)
        : m_compressor(delay, KisSignalCompressor::FIRST_ACTIVE),
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
