/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  Outline algorithm based of the limn of fontutils
 *  Copyright (c) 1992 Karl Berry <karl@cs.umb.edu>
 *  Copyright (c) 1992 Kathryn Hargreaves <letters@cs.umb.edu>
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

#include "kis_selection.h"
#include <QImage>
#include <QVector>
#include <QRect>
#include <QRegion>

#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include <KoColorProfile.h>
#include <KoIntegerMaths.h>
#include <KoCompositeOp.h>

#include "kis_debug.h"
#include "kis_types.h"
#include "kis_iterators_pixel.h"
#include "kis_image.h"
#include "kis_datamanager.h"
#include "kis_selection_component.h"
#include "kis_mask.h"
#include "kis_pixel_selection.h"
#include "kis_painter.h"
#include "kis_fill_painter.h"

struct KisSelection::Private {
    KisPaintDeviceWSP parentPaintDevice;
    bool interestedInDirtyness;
    bool hasPixelSelection;
    bool hasShapeSelection;
    bool isDeselected; // true if the selection is empty, no pixels are selected
    KisPixelSelectionSP pixelSelection;
    KisSelectionComponent* shapeSelection;
};

KisSelection::KisSelection(KisPaintDeviceSP dev)
        : KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8())
        , m_d(new Private)
{
    Q_ASSERT(dev);
    m_d->parentPaintDevice = dev;
    m_d->interestedInDirtyness = false;
    m_d->hasPixelSelection = false;
    m_d->hasShapeSelection = false;
    m_d->isDeselected = false;
    m_d->shapeSelection = 0;

    clear();
}


KisSelection::KisSelection(KisPaintDeviceSP parent, KisMaskSP mask)
        : KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8())
        , m_d(new Private)
{
    m_d->parentPaintDevice = parent;
    m_d->interestedInDirtyness = false;
    m_d->hasPixelSelection = true;
    m_d->pixelSelection = new KisPixelSelection();
    m_d->hasShapeSelection = false;
    m_d->isDeselected = false;
    m_d->shapeSelection = 0;

    clear();

    QRect extent = mask->exactBounds();
    KisPainter gc(m_d->pixelSelection);
    gc.setCompositeOp(colorSpace()->compositeOp(COMPOSITE_COPY));
    gc.bitBlt(extent.topLeft(), mask->selection(), extent);
    gc.end();

    /**
     * Share the data manager (the one inherited from KisPaintDevice)
     * with the pixel selection until we get an additional
     * selection component
     */
    m_datamanager = m_d->pixelSelection->dataManager();
}

KisSelection::KisSelection()
        : KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8())
        , m_d(new Private)
{
    m_d->parentPaintDevice = 0;
    m_d->interestedInDirtyness = false;
    m_d->hasPixelSelection = false;
    m_d->hasShapeSelection = false;
    m_d->isDeselected = false;
    m_d->shapeSelection = 0;

    clear();
}

KisSelection::KisSelection(const KisSelection& rhs)
        : KisPaintDevice(rhs)
        , m_d(new Private)
{
    m_d->parentPaintDevice = rhs.m_d->parentPaintDevice;
    m_d->interestedInDirtyness = false;
    if (rhs.m_d->hasPixelSelection) {
        m_d->pixelSelection = new KisPixelSelection(*rhs.m_d->pixelSelection.data());
        m_d->pixelSelection->dataManager()->setDefaultPixel(rhs.m_d->pixelSelection->dataManager()->defaultPixel());
    }
    m_d->hasPixelSelection = rhs.m_d->hasPixelSelection;
    m_d->hasShapeSelection = rhs.m_d->hasShapeSelection;
    m_d->isDeselected = rhs.m_d->isDeselected;

    if (rhs.m_d->hasShapeSelection) {
        Q_ASSERT(rhs.m_d->shapeSelection);
        m_d->shapeSelection = rhs.m_d->shapeSelection->clone();
        Q_ASSERT(m_d->shapeSelection != rhs.m_d->shapeSelection);
    } else {
        m_d->shapeSelection = 0;
    }

}

KisSelection::~KisSelection()
{
    delete m_d->shapeSelection;
    delete m_d;
}

quint8 KisSelection::selected(qint32 x, qint32 y) const
{
    KisHLineConstIteratorPixel iter = createHLineConstIterator(x, y, 1);

    const quint8 *pix = iter.rawData();

    return *pix;
}

void KisSelection::clear()
{
    quint8 defPixel = MIN_SELECTED;
    dataManager()->setDefaultPixel(&defPixel);
    dataManager()->clear();
}

void KisSelection::clear(const QRect& r)
{

    KisFillPainter painter(KisPaintDeviceSP(this));
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    painter.fillRect(r, KoColor(Qt::white, cs), *(dataManager()->defaultPixel()));
}


bool KisSelection::isTotallyUnselected(const QRect & r) const
{
    if (*(dataManager()->defaultPixel()) != MIN_SELECTED)
        return false;
    QRect sr = selectedExactRect();
    return ! r.intersects(sr);
}

bool KisSelection::isProbablyTotallyUnselected(const QRect & r) const
{
    if (*(dataManager()->defaultPixel()) != MIN_SELECTED)
        return false;
    QRect sr = selectedRect();
    return ! r.intersects(sr);
}

QRect KisSelection::selectedRect() const
{
    if (*(m_datamanager->defaultPixel()) == MIN_SELECTED) {
        if (m_d->parentPaintDevice) {
            // The selected exact rect is the area of this selection that overlaps
            // with the parent paint device.
            return m_d->parentPaintDevice->extent().intersected(extent());
        } else {
            return extent();
        }
    } else {
        if (m_d->parentPaintDevice) {
            // By default all pixels are selected, to the size of the parent paint device.
            return m_d->parentPaintDevice->extent();
        } else {
            // By default all pixels are selected; no matter how many pixels are
            // marked as deselected, there are always by-default-selected pixels
            // around the deselected pixels.
            return QRect(0, 0, qint32_MAX, qint32_MAX);
        }
    }
}

QRect KisSelection::selectedExactRect() const
{
    if (*(m_datamanager->defaultPixel()) == MIN_SELECTED) {
        if (m_d->parentPaintDevice) {
            // The selected exact rect is the area of this selection that overlaps
            // with the parent paint device.
            return m_d->parentPaintDevice->exactBounds().intersected(exactBounds());
        } else {
            return exactBounds();
        }
    } else {
        if (m_d->parentPaintDevice) {
            // By default all pixels are selected, to the size of the parent paint device.
            return m_d->parentPaintDevice->exactBounds();
        } else {
            // By default all pixels are selected; no matter how many pixels are
            // marked as deselected, there are always by-default-selected pixels
            // around the deselected pixels.
            return QRect(0, 0, qint32_MAX, qint32_MAX);
        }
    }
}

void KisSelection::setInterestedInDirtyness(bool b)
{
    m_d->interestedInDirtyness = b;
}

bool KisSelection::interestedInDirtyness() const
{
    return m_d->interestedInDirtyness;
}


void KisSelection::setDirty(const QRect& rc)
{
    if (m_d->interestedInDirtyness)
        KisPaintDevice::setDirty(rc); // Updates m_d->parentPaintDevice
}


void KisSelection::setDirty(const QRegion& reg)
{
    if (m_d->interestedInDirtyness)
        KisPaintDevice::setDirty(reg); // Updates m_d->parentPaintDevice
}

void KisSelection::setDirty()
{
    if (m_d->interestedInDirtyness)
        KisPaintDevice::setDirty(); // Updates m_d->parentPaintDevice
}

bool KisSelection::hasPixelSelection() const
{
    return m_d->hasPixelSelection;
}

bool KisSelection::hasShapeSelection() const
{
    return m_d->hasShapeSelection;
}

KisPixelSelectionSP KisSelection::pixelSelection() const
{
    return m_d->pixelSelection;
}

KisSelectionComponent* KisSelection::shapeSelection() const
{
    return m_d->shapeSelection;
}

KisPixelSelectionSP KisSelection::getOrCreatePixelSelection()
{
    if (!m_d->hasPixelSelection) {
        KisPixelSelectionSP pixelSelection;
        if (m_d->parentPaintDevice)
            pixelSelection = new KisPixelSelection(m_d->parentPaintDevice);
        else
            pixelSelection = new KisPixelSelection();
        setPixelSelection(pixelSelection);
    }
    // Share the datamanager unless there's a shape selection
    if (!m_d->hasShapeSelection) {
        m_datamanager = m_d->pixelSelection->dataManager();
    }
    return m_d->pixelSelection;
}

void KisSelection::setPixelSelection(KisPixelSelectionSP pixelSelection)
{
    m_d->pixelSelection = pixelSelection;
    m_d->hasPixelSelection = true;
    // Share the datamanager unless there's a shape selection
    if (!m_d->hasShapeSelection) {
        m_datamanager = pixelSelection->dataManager();
    }
}

void KisSelection::setShapeSelection(KisSelectionComponent* shapeSelection)
{
    m_d->shapeSelection = shapeSelection;
    m_d->hasShapeSelection = true;
    // Unshare the data manager of the pixel selection
    if (m_d->hasPixelSelection && m_datamanager == m_d->pixelSelection->dataManager()) {
        m_datamanager = new KisDataManager(1, m_d->pixelSelection->dataManager()->defaultPixel());
    }
}

void KisSelection::updateProjection()
{
    // if we share the datamanager with the pixel selection, no updates are necessary
    if (!m_d->hasShapeSelection) return;
    Q_ASSERT(m_d->shapeSelection);

    clear();
    if (m_d->hasPixelSelection) {
        quint8 defPixel = *(m_d->pixelSelection->dataManager()->defaultPixel());
        dataManager()->setDefaultPixel(&defPixel);
        m_d->pixelSelection->renderToProjection(this);
    }
    m_d->shapeSelection->renderToProjection(this);


}

void KisSelection::updateProjection(const QRect& r)
{
    // if we share the datamanager with the pixel selection, no updates are necessary
    if (!m_d->hasShapeSelection) return;
    Q_ASSERT(m_d->shapeSelection);

    clear(r);
    if (m_d->hasPixelSelection) {
        quint8 defPixel = *(m_d->pixelSelection->dataManager()->defaultPixel());
        dataManager()->setDefaultPixel(&defPixel);

        m_d->pixelSelection->renderToProjection(this, r);
    }
    m_d->shapeSelection->renderToProjection(this, r);
}

void KisSelection::setDeselected(bool deselected)
{
    m_d->isDeselected = deselected;
}

bool KisSelection::isDeselected()
{
    return m_d->isDeselected;
}
