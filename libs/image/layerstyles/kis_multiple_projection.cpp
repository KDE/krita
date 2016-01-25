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


struct ProjectionStruct {
    KisPaintDeviceSP device;
    QString compositeOpId;
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

KisMultipleProjection::~KisMultipleProjection()
{
}

QString KisMultipleProjection::defaultProjectionId()
{
    return "00_default";
}

KisPaintDeviceSP KisMultipleProjection::getProjection(const QString &id, const QString &compositeOpId, KisPaintDeviceSP prototype)
{
    QReadLocker readLocker(&m_d->lock);

    PlanesMap::const_iterator constIt = m_d->planes.constFind(id);

    if (constIt == m_d->planes.constEnd() ||
        constIt->compositeOpId != compositeOpId ||
        !(*constIt->device->colorSpace() == *prototype->colorSpace())) {

        readLocker.unlock();

        {
            QWriteLocker writeLocker(&m_d->lock);

            PlanesMap::iterator writeIt = m_d->planes.find(id);
            if (writeIt == m_d->planes.end()) {
                ProjectionStruct plane;
                plane.device = new KisPaintDevice(*prototype);
                plane.compositeOpId = compositeOpId;
                writeIt = m_d->planes.insert(id, plane);
            } else if (writeIt->compositeOpId != compositeOpId ||
                       !(*writeIt->device->colorSpace() == *prototype->colorSpace())) {

                writeIt->device->makeCloneFromRough(prototype, prototype->extent());
                writeIt->compositeOpId = compositeOpId;
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

void KisMultipleProjection::apply(KisPaintDeviceSP dstDevice, const QRect &rect)
{
    QReadLocker readLocker(&m_d->lock);

    PlanesMap::const_iterator it = m_d->planes.constBegin();
    PlanesMap::const_iterator end = m_d->planes.constEnd();

    for (; it != end; ++it) {
        KisPainter gc(dstDevice);
        gc.setCompositeOp(it->compositeOpId);
        gc.bitBlt(rect.topLeft(), it->device, rect);
    }
}

void KisMultipleProjection::syncLodCache()
{
    QReadLocker readLocker(&m_d->lock);

    PlanesMap::const_iterator it = m_d->planes.constBegin();
    PlanesMap::const_iterator end = m_d->planes.constEnd();

    for (; it != end; ++it) {
        KisPaintDeviceSP device = it->device;
        QRegion dirtyRegion = device->syncLodCache(device->defaultBounds()->currentLevelOfDetail());
        Q_UNUSED(dirtyRegion);
    }
}
