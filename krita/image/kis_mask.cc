/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *            (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_mask.h"


#include <kis_debug.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoCompositeOp.h>

#include "kis_paint_device.h"
#include "kis_selection.h"
#include "kis_pixel_selection.h"
#include "kis_painter.h"

#include "kis_image.h"
#include "kis_layer.h"

struct KisMask::Private {
    class PerThreadPaintDevice {
    public:
        KisPaintDeviceSP device(KisPaintDeviceSP projection) {
            if(!m_storage.hasLocalData()) {
                // XXX: this leaks!
                m_storage.setLocalData(new KisPaintDeviceSP(new KisPaintDevice(projection->colorSpace())));
            }
            KisPaintDeviceSP device = *m_storage.localData();
            device->prepareClone(projection);

            return device;
        }

        ~PerThreadPaintDevice() {
            // In case current thread used the storage too...
            if(m_storage.hasLocalData()) {
                m_storage.setLocalData(0);
            }
        }
    private:
        QThreadStorage<KisPaintDeviceSP *> m_storage;
    };

    mutable KisSelectionSP selection;
    PerThreadPaintDevice paintDeviceCache;
};

KisMask::KisMask(const QString & name)
        : KisNode()
        , m_d(new Private())
{
    setName(name);
}

KisMask::KisMask(const KisMask& rhs)
        : KisNode(rhs)
        , m_d(new Private())
{
    setName(rhs.name());

    if (rhs.m_d->selection)
        m_d->selection = new KisSelection(*rhs.m_d->selection.data());
}

KisMask::~KisMask()
{
    delete m_d;
}

const KoColorSpace * KisMask::colorSpace() const
{
    KisNodeSP parentNode = parent();
    return parentNode ? parentNode->colorSpace() : 0;
}

const KoCompositeOp * KisMask::compositeOp() const
{
    /**
     * FIXME: This function duplicates the same function from
     * KisLayer. We can't move it to KisBaseNode as it doesn't
     * know anything about parent() method of KisNode
     * Please think it over...
     */

    KisNodeSP parentNode = parent();
    if (!parentNode) return 0;

    if (!parentNode->colorSpace()) return 0;
    const KoCompositeOp* op = parentNode->colorSpace()->compositeOp(compositeOpId());
    return op ? op : parentNode->colorSpace()->compositeOp(COMPOSITE_OVER);
}

void KisMask::initSelection(KisSelectionSP copyFrom, KisLayerSP parentLayer)
{
    Q_ASSERT(parentLayer);

    KisPaintDeviceSP parentPaintDevice = parentLayer->paintDevice();

    if(copyFrom) {
        /**
         * We can't use setSelection as we may not have parent() yet
         */
        m_d->selection = new KisSelection(*copyFrom);
        m_d->selection->setDefaultBounds(parentPaintDevice->defaultBounds());
    }
    else {
        m_d->selection = new KisSelection(parentPaintDevice,
                                          parentPaintDevice->defaultBounds());

        quint8 newDefaultPixel = MAX_SELECTED;
        m_d->selection->getOrCreatePixelSelection()->setDefaultPixel(&newDefaultPixel);
    }
    m_d->selection->updateProjection();
}

KisSelectionSP KisMask::selection() const
{
    #warning "Please remove lazyness from KisMask::selection() after release of 2.3"

    if(!m_d->selection) {
        KisLayer *parentLayer = dynamic_cast<KisLayer*>(parent().data());
        if(parentLayer) {
            KisPaintDeviceSP parentPaintDevice = parentLayer->paintDevice();
            m_d->selection = new KisSelection(parentPaintDevice,
                                              parentPaintDevice->defaultBounds());

            quint8 newDefaultPixel = MAX_SELECTED;
            m_d->selection->getOrCreatePixelSelection()->setDefaultPixel(&newDefaultPixel);
        }
        else {
            m_d->selection = new KisSelection();
        }
        m_d->selection->updateProjection();
    }

    return m_d->selection;
}

KisPaintDeviceSP KisMask::paintDevice() const
{
    return selection()->getOrCreatePixelSelection();
}

void KisMask::setSelection(KisSelectionSP selection)
{
    m_d->selection = selection;
    if (parent()) {
        const KisLayer *parentLayer = qobject_cast<const KisLayer*>(parent());
        m_d->selection->setDefaultBounds(KisDefaultBounds(parentLayer->image()));
    }
}

void KisMask::select(const QRect & rc, quint8 selectedness)
{
    KisSelectionSP sel = selection();
    KisPixelSelectionSP psel = sel->getOrCreatePixelSelection();
    psel->select(rc, selectedness);
    sel->updateProjection(rc);
}


QRect KisMask::decorateRect(KisPaintDeviceSP &src,
                            KisPaintDeviceSP &dst,
                            const QRect & rc) const
{
    Q_UNUSED(src);
    Q_UNUSED(dst);
    Q_ASSERT_X(0, "KisMask::decorateRect", "Should be overridden by successors");
    return rc;
}

void KisMask::apply(KisPaintDeviceSP projection, const QRect & rc) const
{
    if (selection()) {

        m_d->selection->updateProjection(rc);

        if(!m_d->selection->selectedRect().intersects(rc))
            return;

        KisPaintDeviceSP cacheDevice = m_d->paintDeviceCache.device(projection);

        QRect updatedRect = decorateRect(projection, cacheDevice, rc);

        KisPainter gc(projection);
        gc.setCompositeOp(compositeOp());
        gc.setOpacity(opacity());
        gc.setSelection(m_d->selection);
        gc.bitBlt(updatedRect.topLeft(), cacheDevice, updatedRect);
    } else {
        KisPaintDeviceSP cacheDevice = m_d->paintDeviceCache.device(projection);
        cacheDevice->makeCloneFromRough(projection, rc);
        projection->clear(rc);

        // FIXME: how about opacity and compositeOp?
        decorateRect(cacheDevice, projection, rc);
    }
}

QRect KisMask::needRect(const QRect &rect,  PositionToFilthy pos) const
{
    Q_UNUSED(pos);
    QRect resultRect = rect;
    if (m_d->selection)
        resultRect &= m_d->selection->selectedRect();

    return resultRect;
}

QRect KisMask::changeRect(const QRect &rect, PositionToFilthy pos) const
{
    Q_UNUSED(pos);
    QRect resultRect = rect;
    if (m_d->selection)
        resultRect &= m_d->selection->selectedRect();

    return resultRect;
}

QRect KisMask::extent() const
{
    return m_d->selection ? m_d->selection->selectedRect() :
           parent() ? parent()->extent() : QRect();
}

QRect KisMask::exactBounds() const
{
    return m_d->selection ? m_d->selection->selectedExactRect() :
           parent() ? parent()->exactBounds() : QRect();
}

qint32 KisMask::x() const
{
    return m_d->selection ? m_d->selection->x() :
           parent() ? parent()->x() : 0;
}

qint32 KisMask::y() const
{
    return m_d->selection ? m_d->selection->y() :
           parent() ? parent()->y() : 0;
}

void KisMask::setX(qint32 x)
{
    if (m_d->selection)
        m_d->selection->setX(x);
}

void KisMask::setY(qint32 y)
{
    if (m_d->selection)
        m_d->selection->setY(y);
}

void KisMask::setDirty(const QRect & rect)
{
    Q_ASSERT(parent());

    const KisLayer *parentLayer = qobject_cast<const KisLayer*>(parent());
    KisImageWSP image = parentLayer->image();
    Q_ASSERT(image);

    image->updateProjection(this, rect);
}

QImage KisMask::createThumbnail(qint32 w, qint32 h)
{
    KisPaintDeviceSP originalDevice = paintDevice();

    return originalDevice ?
           originalDevice->createThumbnail(w, h) : QImage();
}

#include "kis_mask.moc"
