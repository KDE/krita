/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_thread_safe_signal_compressor.h"

#include <QApplication>


KisThreadSafeSignalCompressor::KisThreadSafeSignalCompressor(int delay, KisSignalCompressor::Mode mode)
    : m_compressor(new KisSignalCompressor(delay, mode, this))
{
    connect(this, SIGNAL(internalRequestSignal()), m_compressor, SLOT(start()), Qt::AutoConnection);
    connect(this, SIGNAL(internalStopSignal()), m_compressor, SLOT(stop()), Qt::AutoConnection);
    connect(this, SIGNAL(internalSetDelay(int)), m_compressor, SLOT(setDelay(int)), Qt::AutoConnection);
    connect(m_compressor, SIGNAL(timeout()), SIGNAL(timeout()));

    // due to this line the object *must not* be deleted explicitly!
    this->setObjectName("KisThreadSafeSignalCompressor");
    this->moveToThread(QApplication::instance()->thread());
}

bool KisThreadSafeSignalCompressor::isActive() const
{
    return m_compressor->isActive();
}

void KisThreadSafeSignalCompressor::setDelay(int delay)
{
    emit internalSetDelay(delay);
}

void KisThreadSafeSignalCompressor::start()
{
    emit internalRequestSignal();
}

void KisThreadSafeSignalCompressor::stop()
{
    emit internalStopSignal();
}
