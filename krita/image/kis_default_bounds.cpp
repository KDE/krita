/*
 *  Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_default_bounds.h"
#include "kis_paint_device.h"

const QRect KisDefaultBounds::infiniteRect =
    QRect(qint32_MIN/2, qint32_MIN/2, qint32_MAX, qint32_MAX);

struct KisDefaultBounds::Private
{
    KisImageWSP image;
    KisPaintDeviceSP parentDevice;
};


KisDefaultBounds::KisDefaultBounds()
    : QObject(0)
    , m_d(new Private())
{
    m_d->image = 0;
    m_d->parentDevice = 0;
}

KisDefaultBounds::KisDefaultBounds(KisImageWSP image)
    : QObject(0)
    , m_d(new Private())
{
    m_d->parentDevice = 0;
    m_d->image = image;
}

KisDefaultBounds::KisDefaultBounds(KisPaintDeviceSP parentDevice)
    : QObject(0)
    , m_d(new Private())
{
    m_d->parentDevice = parentDevice;
    m_d->image = 0;
}

KisDefaultBounds::KisDefaultBounds(KisImageWSP image, KisPaintDeviceSP parentDevice)
    : QObject(0)
    , m_d(new Private())
{
    m_d->image = image;
    m_d->parentDevice = parentDevice;
}


KisDefaultBounds::~KisDefaultBounds()
{
    delete m_d;
}

QRect KisDefaultBounds::bounds() const
{


    QRect additionalRect = m_d->parentDevice ? m_d->parentDevice->exactBounds() : QRect();
    additionalRect |= (m_d->image ? m_d->image->bounds() : infiniteRect);

    if (!m_d->image && !m_d->parentDevice) {
        additionalRect = QRect();
    }
    return additionalRect;
}
