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

struct KisMask::Private {
    KisSelectionSP selection;
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

KisSelectionSP KisMask::selection() const
{
    if (!m_d->selection) {
        m_d->selection = new KisSelection();
        /**
         * FIXME: Add default pixel choice
         * e.g. "Selected by default" or "Deselected by default"
         */
        if (parent())
            m_d->selection->getOrCreatePixelSelection()->select(parent()->exactBounds(), MAX_SELECTED);
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
    Q_ASSERT_X(0, "KisMask::decorateRect", "Should be overriden by successors");
    return rc;
}

void KisMask::apply(KisPaintDeviceSP projection, const QRect & rc) const
{
    if (m_d->selection) {

        m_d->selection->updateProjection(rc);

        KisPaintDeviceSP cacheDevice =
            new KisPaintDevice(projection->colorSpace());

        QRect updatedRect = decorateRect(projection, cacheDevice, rc);

        /**
         * FIXME: ALPHA_DARKEN vs OVER
         */
        KisPainter gc(projection);
        gc.setCompositeOp(compositeOp());
        gc.setOpacity(opacity());
        gc.setSelection(m_d->selection);
        gc.bitBlt(updatedRect.topLeft(), cacheDevice, updatedRect);
    } else {
        decorateRect(projection, projection, rc);
    }
}

QRect KisMask::needRect(const QRect &rect) const
{
    QRect resultRect = rect;
    if (m_d->selection)
        resultRect &= m_d->selection->selectedRect();

    return resultRect;
}

QRect KisMask::changeRect(const QRect &rect) const
{
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

void KisMask::setDirty()
{
    if (parent() && parent()->inherits("KisLayer")) {
        parent()->setDirty();
    }
}

void KisMask::setDirty(const QRect & rect)
{
    if (parent() && parent()->inherits("KisLayer")) {
        parent()->setDirty(rect);
    }
}

void KisMask::setDirty(const QRegion & region)
{
    if (parent() && parent()->inherits("KisLayer")) {
        parent()->setDirty(region);
    }
}

QImage KisMask::createThumbnail(qint32 w, qint32 h)
{
    KisPaintDeviceSP originalDevice = paintDevice();

    return originalDevice ?
           originalDevice->createThumbnail(w, h) : QImage();
}

#include "kis_mask.moc"
