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

#include <QTime>
#include <QImage>
#include <QVector>

#include <kdebug.h>

#include "KoColorSpaceRegistry.h"
#include "KoIntegerMaths.h"

#include "kis_debug_areas.h"
#include "kis_types.h"
#include "kis_iterators_pixel.h"
#include "kis_image.h"
#include "kis_datamanager.h"
#include "kis_selection_component.h"
#include "kis_mask.h"
#include "kis_pixel_selection.h"


KisSelection::KisSelection(KisPaintDeviceSP dev)
    : super(dev,
            QString("selection for ") + dev->objectName())
    , m_parentPaintDevice(dev)
    , m_dirty(false)
    , m_hasPixelSelection(false)
    , m_hasShapeSelection(false)
{
    Q_ASSERT(dev);
}


KisSelection::KisSelection( KisPaintDeviceSP parent, KisMaskSP mask )
    : super( parent, "selection from mask" )
    , m_parentPaintDevice( parent )
    , m_dirty( false )
    , m_hasPixelSelection(false)
    , m_hasShapeSelection(false)
{
    m_datamanager = mask->dataManager();
}

KisSelection::KisSelection()
    : super("anonymous selection")
    , m_parentPaintDevice(0)
    , m_dirty(false)
    , m_hasPixelSelection(false)
    , m_hasShapeSelection(false)
{
}

KisSelection::KisSelection(const KisSelection& rhs)
    : super(rhs)
    , m_parentPaintDevice(rhs.m_parentPaintDevice)
    , m_dirty(false)
    , m_hasPixelSelection(false)
    , m_hasShapeSelection(false)
{
}

KisSelection::~KisSelection()
{
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
    m_datamanager->setDefaultPixel(&defPixel);
    m_datamanager->clear();
}

void KisSelection::invert()
{
    qint32 x,y,w,h;

    extent(x, y, w, h);
    KisRectIterator it = createRectIterator(x, y, w, h);
    while ( ! it.isDone() )
    {
        // CBR this is wrong only first byte is inverted
        // BSAR: But we have always only one byte in this color model :-).
        *(it.rawData()) = MAX_SELECTED - *(it.rawData());
        ++it;
    }
    quint8 defPixel = MAX_SELECTED - *(m_datamanager->defaultPixel());
    m_datamanager->setDefaultPixel(&defPixel);
}

bool KisSelection::isTotallyUnselected(QRect r) const
{
    if(*(m_datamanager->defaultPixel()) != MIN_SELECTED)
        return false;
    QRect sr = selectedExactRect();
    return ! r.intersects(sr);
}

bool KisSelection::isProbablyTotallyUnselected(QRect r) const
{
    if(*(m_datamanager->defaultPixel()) != MIN_SELECTED)
        return false;
    QRect sr = selectedRect();
    return ! r.intersects(sr);
}



QRect KisSelection::selectedRect() const
{
    if(*(m_datamanager->defaultPixel()) == MIN_SELECTED || !m_parentPaintDevice)
        return extent();
    else
        return extent().unite(m_parentPaintDevice->extent());
}

QRect KisSelection::selectedExactRect() const
{
    if(*(m_datamanager->defaultPixel()) == MIN_SELECTED || !m_parentPaintDevice) {
        return exactBounds();
    }
    else {
        return exactBounds().unite(m_parentPaintDevice->exactBounds());
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

    quint8* buffer = new quint8[width*height];
    readBytes(buffer, r.x(), r.y(), width, height);

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

void KisSelection::setDirty(const QRect& rc)
{
    if (m_dirty)
        super::setDirty(rc);
}

void KisSelection::setDirty()
{
    if (m_dirty)
        super::setDirty();
}

bool KisSelection::hasPixelSelection() const
{
    return m_hasPixelSelection;
}


bool KisSelection::hasShapeSelection() const
{
    return m_hasShapeSelection;
}


KisPixelSelectionSP KisSelection::pixelSelection()
{
    return m_pixelSelection;
}


KisSelectionComponent* KisSelection::shapeSelection()
{
   return m_shapeSelection;
}

void KisSelection::setPixelSelection(KisPixelSelectionSP pixelSelection)
{
     m_pixelSelection = pixelSelection;
     m_hasPixelSelection = true;
}

void KisSelection::setShapeSelection(KisSelectionComponent* shapeSelection)
{
    m_shapeSelection = shapeSelection;
    m_hasShapeSelection = true;
}

void KisSelection::updateProjection()
{
    QTime t;
    t.start();
    clear();
    if(m_hasPixelSelection) {
         quint8 defPixel = *(m_pixelSelection->dataManager()->defaultPixel());
         m_datamanager->setDefaultPixel(&defPixel);

        m_pixelSelection->renderToProjection(this);
    }
    if(m_hasShapeSelection) {
        m_shapeSelection->renderToProjection(this);
    }
    kDebug(41010) << "Selection rendering took: " << t.elapsed();
}
