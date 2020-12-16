/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
