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

#ifndef __KIS_DEFERRED_SIGNAL_H
#define __KIS_DEFERRED_SIGNAL_H

#include <QObject>

#include <functional>

/**
 * \class KisDeferredSignal
 *        \brief This class is used for calling a specified callback
 *        function (which is a std::function) after a specified time
 *        delay. The callback is called from the QTimer event, so the
 *        usage of the class does not block the Qt's event loop.
 *
 *        Usage:
 *
 *        \code{.cpp}
 *
 *        // prepare the callback function
 *        std::function<void ()> callback(
 *            std::bind(&KisCanvas2::setMonitorProfile, this,
 *                        monitorProfile, renderingIntent, conversionFlags));
 *
 *        // create the timer connected to the function
 *        KisDeferredSignal::deferSignal(1000, callback);
 *
 *        \endcode
 *
 *        TODO: rename KisDeferredSignal -> KisDeferredCallback
 */
class KisDeferredSignal : public QObject
{
    Q_OBJECT
public:
    using CallbackFunction = std::function<void ()>;

public:
    /**
     * Creates a timer which will call \p function after \p delay
     * milliseconds
     */
    static void deferSignal(int delay, CallbackFunction function);

private Q_SLOTS:
    void timeout();

private:
    KisDeferredSignal(int delay, CallbackFunction function);

private:
    CallbackFunction m_function;
};

#endif /* __KIS_DEFERRED_SIGNAL_H */
