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

#include "kis_thread_safe_signal_compressor.h"

#include <QApplication>


KisThreadSafeSignalCompressor::KisThreadSafeSignalCompressor(int delay, KisSignalCompressor::Mode mode)
    : m_compressor(new KisSignalCompressor(delay, mode, this))
{
    connect(this, SIGNAL(internalRequestSignal()), m_compressor, SLOT(start()), Qt::AutoConnection);
    connect(this, SIGNAL(internalStopSignal()), m_compressor, SLOT(stop()), Qt::AutoConnection);
    connect(m_compressor, SIGNAL(timeout()), SIGNAL(timeout()));

    // due to this line the object *must not* be deleted explicitly!
    this->setObjectName("KisThreadSafeSignalCompressor");
    this->moveToThread(QApplication::instance()->thread());
}

bool KisThreadSafeSignalCompressor::isActive() const
{
    return m_compressor->isActive();
}

void KisThreadSafeSignalCompressor::start()
{
    emit internalRequestSignal();
}

void KisThreadSafeSignalCompressor::stop()
{
    emit internalStopSignal();
}
