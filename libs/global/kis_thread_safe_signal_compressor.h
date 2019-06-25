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

#ifndef __KIS_THREAD_SAFE_SIGNAL_COMPRESSOR_H
#define __KIS_THREAD_SAFE_SIGNAL_COMPRESSOR_H

#include <QObject>
#include "kritaglobal_export.h"

#include "kis_signal_compressor.h"

/**
 * A special class which works exactly like KisSignalCompressor, but
 * supports calling \p start() method from within the context of
 * another thread. If it happens, it posts a message to Qt's event
 * loop and the \p start() signal is delivered when event loop gets
 * executes again.
 *
 * WARNING: After creation this object moves itself into the main
 *          thread, so one must *not* delete it explicitly. Use
 *          deleteLater() instead. Moving into another thread is
 *          another reason why it cannot have parent QObject.
 */
class KRITAGLOBAL_EXPORT KisThreadSafeSignalCompressor : public QObject
{
    Q_OBJECT
public:
    KisThreadSafeSignalCompressor(int delay, KisSignalCompressor::Mode mode);

    bool isActive() const;

public Q_SLOTS:
    void setDelay(int delay);
    void start();
    void stop();

Q_SIGNALS:
    void timeout();
    void internalRequestSignal();
    void internalStopSignal();
    void internalSetDelay(int delay);

private:
    KisSignalCompressor *m_compressor;
};

#endif /* __KIS_THREAD_SAFE_SIGNAL_COMPRESSOR_H */
