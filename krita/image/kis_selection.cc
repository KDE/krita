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
#include <QVarLengthArray>

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
#include "kis_datamanager.h"

struct KisSelection::Private {
    KisPaintDeviceWSP parentPaintDevice;
    bool interestedInDirtyness;
    bool hasPixelSelection;
    bool hasShapeSelection;
    KisPixelSelectionSP pixelSelection;
    KisSelectionComponent* shapeSelection;
};

KisSelection::KisSelection(KisPaintDeviceSP dev)
    : KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8(), QString("selection for ") + dev->objectName())
    , m_d(new Private)
{
    Q_ASSERT(dev);
    m_d->parentPaintDevice = dev;
    m_d->interestedInDirtyness = false;
    m_d->hasPixelSelection = false;
    m_d->hasShapeSelection = false;
    m_d->shapeSelection = 0;
    
    clear();
}


KisSelection::KisSelection( KisPaintDeviceSP parent, KisMaskSP mask )
    : KisPaintDevice( KoColorSpaceRegistry::instance()->alpha8(), "selection from mask" )
    , m_d(new Private)
{
    m_d->parentPaintDevice = parent;
    m_d->interestedInDirtyness = false;
    m_d->hasPixelSelection = true;
    m_d->pixelSelection = new KisPixelSelection();
    m_d->hasShapeSelection = false;
    m_d->shapeSelection = 0;

    clear();
    
    QRect extent = mask->exactBounds();
    KisPainter gc( m_d->pixelSelection );
    gc.setCompositeOp( colorSpace()->compositeOp( COMPOSITE_COPY ) );
    gc.bitBlt( extent.topLeft(), mask->selection(), extent );
    gc.end();

    // Share the data manager with the pixel selection until we get an additional selection component
    m_datamanager = m_d->pixelSelection->dataManager();
}

KisSelection::KisSelection()
    : KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8(), "anonymous selection")
    , m_d( new Private )
{
    m_d->parentPaintDevice = 0;
    m_d->interestedInDirtyness = false;
    m_d->hasPixelSelection = false;
    m_d->hasShapeSelection = false;
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
    }
    m_d->hasPixelSelection = rhs.m_d->hasPixelSelection;
    m_d->hasShapeSelection = rhs.m_d->hasShapeSelection;
    if (rhs.m_d->hasShapeSelection) {
        m_d->shapeSelection = 0; // XXX: define clone method for selection components!
    }
    else {
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
    if(*(dataManager()->defaultPixel()) != MIN_SELECTED)
        return false;
    QRect sr = selectedExactRect();
    return ! r.intersects(sr);
}

bool KisSelection::isProbablyTotallyUnselected(const QRect & r) const
{
    if(*(dataManager()->defaultPixel()) != MIN_SELECTED)
        return false;
    QRect sr = selectedRect();
    return ! r.intersects(sr);
}

QRect KisSelection::selectedRect() const
{
    if (*(m_datamanager->defaultPixel()) == MIN_SELECTED )
    {
        if (m_d->parentPaintDevice) {
            // The selected exact rect is the area of this selection that overlaps
            // with the parent paint device.
            return m_d->parentPaintDevice->extent().intersected(extent());
        }
        else {
            return extent();
        }
    }
    else {
        if (m_d->parentPaintDevice) {
            // By default all pixels are selected, to the size of the parent paint device.
            return m_d->parentPaintDevice->extent();
        }
        else {
            // By default all pixels are selected; no matter how many pixels are
            // marked as deselected, there are always by-default-selected pixels
            // around the deselected pixels.
            return QRect(0, 0, qint32_MAX, qint32_MAX);
        }
    }
}

QRect KisSelection::selectedExactRect() const
{
    if (*(m_datamanager->defaultPixel()) == MIN_SELECTED )
    {
        if (m_d->parentPaintDevice) {
            // The selected exact rect is the area of this selection that overlaps
            // with the parent paint device.
            return m_d->parentPaintDevice->exactBounds().intersected(exactBounds());
        }
        else {
            return exactBounds();
        }
    }
    else {
        if (m_d->parentPaintDevice) {
            // By default all pixels are selected, to the size of the parent paint device.
            return m_d->parentPaintDevice->exactBounds();
        }
        else {
            // By default all pixels are selected; no matter how many pixels are
            // marked as deselected, there are always by-default-selected pixels
            // around the deselected pixels.
            return QRect(0, 0, qint32_MAX, qint32_MAX);
        }
    }
}

void KisSelection::paint(QImage* img, const QRect & r)
{
    if (img->isNull()) {
        return;
    }

    qint32 width = r.width();
    qint32 height = r.height();

    Q_ASSERT(img->width() == width);
    Q_ASSERT(img->height() == height);

    QVarLengthArray<quint8> buffer(width*height);
    readBytes(&buffer[0], r.x(), r.y(), width, height);

    for (qint32 y = 0; y < height; y++) {

        QRgb *imagePixel = reinterpret_cast<QRgb *>(img->scanLine(y));
        for (qint32 x = 0; x < width; x++) {

                quint8 selectedness = buffer[y*width+x];

                if (selectedness != MAX_SELECTED) {

                    // this is where we come if the pixels should be blue or bluish

                    QRgb srcPixel = *imagePixel;
                    quint8 srcGrey = (qRed(srcPixel) + qGreen(srcPixel) + qBlue(srcPixel)) / 9;
                    quint8 srcAlpha = qAlpha(srcPixel);

                    // Color influence is proportional to alphaPixel.
                    srcGrey = UINT8_MULT(srcGrey, srcAlpha);

                    QRgb dstPixel;

                    if (selectedness == MIN_SELECTED) {

                        // Stop unselected transparent areas from appearing the same
                        // as selected transparent areas.
                        quint8 dstAlpha = qMax(srcAlpha, quint8(192));
                        dstPixel = qRgba(128 + srcGrey, 128 + srcGrey, 165 + srcGrey, dstAlpha);

                    } else {
                        dstPixel = qRgba(UINT8_BLEND(qRed(srcPixel), srcGrey + 128, selectedness),
                                         UINT8_BLEND(qGreen(srcPixel), srcGrey + 128, selectedness),
                                         UINT8_BLEND(qBlue(srcPixel), srcGrey + 165, selectedness),
                                         srcAlpha);
                    }

                    *imagePixel = dstPixel;
                }

                imagePixel++;
        }
    }
}

void KisSelection::setInterestedInDirtyness(bool b) { m_d->interestedInDirtyness = b; }
    
bool KisSelection::interestedInDirtyness() const { return m_d->interestedInDirtyness; }


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
    if ( !m_d->hasPixelSelection ) {
        KisPixelSelectionSP pixelSelection;
        if(m_d->parentPaintDevice)
            pixelSelection = new KisPixelSelection(m_d->parentPaintDevice);
        else
            pixelSelection = new KisPixelSelection();
        setPixelSelection( pixelSelection );
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
    
    clear();
    if(m_d->hasPixelSelection) {
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
    
    clear(r);
    if(m_d->hasPixelSelection) {
         quint8 defPixel = *(m_d->pixelSelection->dataManager()->defaultPixel());
         dataManager()->setDefaultPixel(&defPixel);

        m_d->pixelSelection->renderToProjection(this, r);
    }
    m_d->shapeSelection->renderToProjection(this, r);
}
