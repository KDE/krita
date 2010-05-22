/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_wait_update_done.h"

KisWaitUpdateDone::KisWaitUpdateDone(KisImageWSP image, qint32 numToWait)
{
    connect(image, SIGNAL(sigImageUpdated(const QRect &)),
            SLOT(slotImageUpdated(const QRect &)),
            Qt::DirectConnection);
    m_numToWait = numToWait;
    m_image = image;
}

KisWaitUpdateDone::~KisWaitUpdateDone()
{
    disconnect(m_image, SIGNAL(sigImageUpdated(const QRect &)));
}

void KisWaitUpdateDone::wait()
{
    m_semaphore.acquire(m_numToWait);
}

void KisWaitUpdateDone::slotImageUpdated(const QRect &rect) {
    // qDebug()<< "Got one updated";
    m_semaphore.release();
}

#include "kis_wait_update_done.moc"
