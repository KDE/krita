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

#include <boost/function.hpp>
#include <boost/bind.hpp>

/**
 * \class KisDeferredSignal is used for calling a specified callback
 *        function (which is a boost::function) after a specified time
 *        delay. The callback is called from the QTimer event, so the
 *        usage of the class does not block the Qt's event loop.
 *
 *        Usage:
 *
 *        \code{.cpp}
 *
 *        // prepare the callback function
 *        boost::function<void ()> callback(
 *            boost::bind(&KisCanvas2::setMonitorProfile, this,
 *                        monitorProfile, renderingIntent, conversionFlags));
 *
 *        // create the timer connected to the function
 *        KisDeferredSignal::deferSignal(1000, callback);
 *
 *        \endcode
 *
 *        TODO: 1) rename KisDeferredSignal -> KisDeferredCallback
 *              2) rename TrivialFunction -> CallbackFunction
 */
class KisDeferredSignal : public QObject
{
    Q_OBJECT
public:
    typedef boost::function<void ()> TrivialFunction;

public:
    /**
     * Creates a timer which will call \p function after \p delay
     * milliseconds
     */
    static void deferSignal(int delay, TrivialFunction function);

private Q_SLOTS:
    void timeout();

private:
    KisDeferredSignal(int delay, TrivialFunction function);

private:
    TrivialFunction m_function;
};

#endif /* __KIS_DEFERRED_SIGNAL_H */
