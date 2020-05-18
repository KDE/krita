/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_multiple_projection.h"

#include <QMap>
#include <QReadWriteLock>


#include <KoColorSpace.h>

#include "kis_painter.h"
#include "kis_paint_device.h"
#include "kis_layer_style_filter_environment.h"


struct ProjectionStruct {
    KisPaintDeviceSP device;
    QString compositeOpId;
    quint8 opacity = OPACITY_OPAQUE_U8;
    QBitArray channelFlags;
};

typedef QMap<QString, ProjectionStruct> PlanesMap;

struct KisMultipleProjection::Private
{
    QReadWriteLock lock;
    PlanesMap planes;
};


KisMultipleProjection::KisMultipleProjection()
    : m_d(new Private)
{
}

KisMultipleProjection::KisMultipleProjection(const KisMultipleProjection &rhs)
    : m_d(new Private)
{
    QReadLocker readLocker(&rhs.m_d->lock);

    auto it = rhs.m_d->planes.constBegin();
    for (; it != rhs.m_d->planes.constEnd(); ++it) {
        ProjectionStruct proj;
        proj.device = new KisPaintDevice(*it->device);
        proj.compositeOpId = it->compositeOpId;
        proj.opacity = it->opacity;
        proj.channelFlags = it->channelFlags;

        m_d->planes.insert(it.key(), proj);
    }
}

KisMultipleProjection::~KisMultipleProjection()
{
}

QString KisMultipleProjection::defaultProjectionId()
{
    return "00_default";
}

KisPaintDeviceSP KisMultipleProjection::getProjection(const QString &id,
                                                      const QString &compositeOpId,
                                                      quint8 opacity,
                                                      const QBitArray &channelFlags,
                                                      KisPaintDeviceSP prototype)
{
    QReadLocker readLocker(&m_d->lock);

    PlanesMap::const_iterator constIt = m_d->planes.constFind(id);

    if (constIt == m_d->planes.constEnd() ||
        constIt->compositeOpId != compositeOpId ||
        constIt->opacity != opacity ||
        constIt->channelFlags != channelFlags ||
        *constIt->device->colorSpace() != *prototype->colorSpace()) {

        readLocker.unlock();

        {
            QWriteLocker writeLocker(&m_d->lock);

            PlanesMap::iterator writeIt = m_d->planes.find(id);
            if (writeIt == m_d->planes.end()) {
                ProjectionStruct plane;
                plane.device = new KisPaintDevice(prototype->colorSpace());
                plane.device->prepareClone(prototype);
                plane.compositeOpId = compositeOpId;
                plane.opacity = opacity;
                plane.channelFlags = channelFlags;
                writeIt = m_d->planes.insert(id, plane);
            } else if (writeIt->compositeOpId != compositeOpId ||
                       *writeIt->device->colorSpace() != *prototype->colorSpace()) {

                writeIt->device->prepareClone(prototype);
                writeIt->compositeOpId = compositeOpId;
                writeIt->opacity = opacity;
                writeIt->channelFlags = channelFlags;
            }

            return writeIt->device;
        }
    }

    return constIt->device;
}

void KisMultipleProjection::freeProjection(const QString &id)
{
    QWriteLocker writeLocker(&m_d->lock);
    m_d->planes.remove(id);
}

void KisMultipleProjection::freeAllProjections()
{
    QWriteLocker writeLocker(&m_d->lock);
    m_d->planes.clear();
}

void KisMultipleProjection::clear(const QRect &rc)
{
    QReadLocker readLocker(&m_d->lock);

    PlanesMap::const_iterator it = m_d->planes.constBegin();
    PlanesMap::const_iterator end = m_d->planes.constEnd();

    for (; it != end; ++it) {
        const_cast<KisPaintDevice*>(it->device.data())->clear(rc);
    }
}

void KisMultipleProjection::apply(KisPaintDeviceSP dstDevice, const QRect &rect, KisLayerStyleFilterEnvironment *env)
{
    QReadLocker readLocker(&m_d->lock);

    PlanesMap::const_iterator it = m_d->planes.constBegin();
    PlanesMap::const_iterator end = m_d->planes.constEnd();

    for (; it != end; ++it) {
        KisPainter gc(dstDevice);
        gc.setCompositeOp(it->compositeOpId);
        env->setupFinalPainter(&gc, it->opacity, it->channelFlags);
        gc.bitBlt(rect.topLeft(), it->device, rect);
    }
}

KisPaintDeviceList KisMultipleProjection::getLodCapableDevices() const
{
    QReadLocker readLocker(&m_d->lock);

    PlanesMap::const_iterator it = m_d->planes.constBegin();
    PlanesMap::const_iterator end = m_d->planes.constEnd();

    KisPaintDeviceList list;
    for (; it != end; ++it) {
        list << it->device;
    }

    return list;
}

bool KisMultipleProjection::isEmpty() const
{
    return m_d->planes.isEmpty();
}

