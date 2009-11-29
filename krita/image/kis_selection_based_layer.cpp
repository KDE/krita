/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *  Copyright (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_selection_based_layer.h"

#include <klocale.h>
#include "kis_debug.h"

#include <KoCompositeOp.h>

#include "kis_image.h"
#include "kis_painter.h"

#include "kis_selection.h"
#include "kis_pixel_selection.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "filter/kis_filter.h"


class KisSelectionBasedLayer::Private
{
public:
    bool showSelection;
    KisSelectionSP selection;
    KisPaintDeviceSP paintDevice;
};


KisSelectionBasedLayer::KisSelectionBasedLayer(KisImageWSP image,
        const QString &name,
        KisSelectionSP selection)
        : KisLayer(image.data(), name, OPACITY_OPAQUE),
        m_d(new Private())
{
    if (!selection)
        initSelection();
    else
        setSelection(selection);

    setShowSelection(true);

    m_d->paintDevice = new KisPaintDevice(image->colorSpace());
}

KisSelectionBasedLayer::KisSelectionBasedLayer(const KisSelectionBasedLayer& rhs)
        : KisLayer(rhs)
        , KisIndirectPaintingSupport(rhs)
        , KisNodeFilterInterface(rhs)
        , m_d(new Private())
{
    setSelection(rhs.m_d->selection);
    setShowSelection(rhs.m_d->showSelection);

    m_d->paintDevice = new KisPaintDevice(*rhs.m_d->paintDevice.data());
}


KisSelectionBasedLayer::~KisSelectionBasedLayer()
{
    delete m_d;
}

void KisSelectionBasedLayer::initSelection()
{
    m_d->selection = new KisSelection();
    m_d->selection->getOrCreatePixelSelection()->select(image()->bounds());
    m_d->selection->updateProjection();
    /**
     * FIXME: Maybe we should have set up a parent
     * for being really interested in it?
     */
    m_d->selection->setInterestedInDirtyness(true);
}

bool KisSelectionBasedLayer::allowAsChild(KisNodeSP node) const
{
    return node->inherits("KisMask");
}


KisPaintDeviceSP KisSelectionBasedLayer::original() const
{
    return m_d->paintDevice;
}
KisPaintDeviceSP KisSelectionBasedLayer::paintDevice() const
{
    return m_d->selection->getOrCreatePixelSelection();
}


QRect KisSelectionBasedLayer::repaintOriginal(KisPaintDeviceSP original,
        const QRect& rect)
{
    Q_UNUSED(original);
    return rect;
}

bool KisSelectionBasedLayer::needProjection() const
{
    return m_d->selection;
}

void KisSelectionBasedLayer::copyOriginalToProjection(const KisPaintDeviceSP original,
        KisPaintDeviceSP projection,
        const QRect& rect) const
{
    m_d->selection->updateProjection();
    KisSelectionSP tempSelection = m_d->selection;
    KisPainter gc(projection);

    if (m_d->selection) {
        if (hasTemporaryTarget()) {
            /**
             * Cloning a selection with COW
             * FIXME: check whether it's faster than usual bitBlt'ing
             */
            tempSelection = new KisSelection(*tempSelection);

            KisPainter gc2(tempSelection);
            gc2.setOpacity(temporaryOpacity());
            gc2.setCompositeOp(temporaryCompositeOp());
            gc2.bitBlt(rect.topLeft(), temporaryTarget(), rect);
        }

        projection->clear(rect);
        gc.setCompositeOp(colorSpace()->compositeOp(COMPOSITE_OVER));
        gc.setSelection(tempSelection);
    } else
        gc.setCompositeOp(colorSpace()->compositeOp(COMPOSITE_COPY));


    gc.bitBlt(rect.topLeft(), original, rect);
}

QRect KisSelectionBasedLayer::changeRect(const QRect &rect) const
{
    /**
     * Warn: we won't call KisNode's copy of changeRect as it's dummy
     */
    return m_d->selection ?
           rect & m_d->selection->selectedRect() :
           rect;
}

QRect KisSelectionBasedLayer::needRect(const QRect &rect) const
{
    return rect;
}

void KisSelectionBasedLayer::resetCache(const KoColorSpace *colorSpace)
{
    if (!colorSpace)
        colorSpace = image()->colorSpace();

    if (!m_d->paintDevice ||
            !(*m_d->paintDevice->colorSpace() == *colorSpace)) {

        m_d->paintDevice = new KisPaintDevice(colorSpace);
    } else {
        m_d->paintDevice->clear();
    }
}

KisSelectionSP KisSelectionBasedLayer::selection() const
{
    return m_d->selection;
}

void KisSelectionBasedLayer::setSelection(KisSelectionSP selection)
{
    if (selection) {
        m_d->selection = new KisSelection(*selection.data());
        m_d->selection->updateProjection();
        m_d->selection->setInterestedInDirtyness(true);
    } else
        m_d->selection = 0;
}

bool KisSelectionBasedLayer::showSelection() const
{
    return m_d->showSelection;
}
void KisSelectionBasedLayer::setShowSelection(bool b)
{
    m_d->showSelection = b;
}

qint32 KisSelectionBasedLayer::x() const
{
    return m_d->selection ? m_d->selection->x() : 0;
}

qint32 KisSelectionBasedLayer::y() const
{
    return m_d->selection ? m_d->selection->y() : 0;
}

void KisSelectionBasedLayer::setX(qint32 x)
{
    if (m_d->selection) {
        m_d->selection->setX(x);
        resetCache();
    }
}

void KisSelectionBasedLayer::setY(qint32 y)
{
    if (m_d->selection) {
        m_d->selection->setY(y);
        resetCache();
    }
}

void KisSelectionBasedLayer::setDirty()
{
    Q_ASSERT(image());

    KisLayer::setDirty(image()->bounds());
}

QRect KisSelectionBasedLayer::extent() const
{
    Q_ASSERT(image());

    return m_d->selection ?
           m_d->selection->selectedRect() : image()->bounds();
}

QRect KisSelectionBasedLayer::exactBounds() const
{
    Q_ASSERT(image());

    return m_d->selection ?
           m_d->selection->selectedExactRect() : image()->bounds();
}

QImage KisSelectionBasedLayer::createThumbnail(qint32 w, qint32 h)
{
    KisSelectionSP originalSelection = selection();
    KisPaintDeviceSP originalDevice = original();

    return originalDevice && originalSelection ?
           originalDevice->createThumbnail(w, h, originalSelection) :
           QImage();
}

#include "kis_selection_based_layer.moc"
