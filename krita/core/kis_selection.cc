/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
 */

#include <qimage.h>
//Added by qt3to4:
#include <Q3MemArray>

#include <kdebug.h>
#include <klocale.h>
#include <qcolor.h>

#include "kis_layer.h"
#include "kis_debug_areas.h"
#include "kis_types.h"
#include "kis_colorspace_factory_registry.h"
#include "kis_fill_painter.h"
#include "kis_iterators_pixel.h"
#include "kis_integer_maths.h"
#include "kis_image.h"
#include "kis_datamanager.h"
#include "kis_fill_painter.h"
#include "kis_selection.h"

KisSelection::KisSelection(KisPaintDeviceSP dev)
    : super(dev->parentLayer(), KisMetaRegistry::instance()->csRegistry()->getAlpha8(), (QString("selection for ") + dev->name()).latin1())
    , m_parentPaintDevice(dev)
{
    Q_ASSERT(dev);
}

KisSelection::KisSelection()
    : super(KisMetaRegistry::instance()->csRegistry()->getAlpha8(), "anonymous selection")
    , m_parentPaintDevice(0)
{
}

KisSelection::KisSelection(const KisSelection& rhs)
    : super(rhs)
{
    m_parentPaintDevice = rhs.m_parentPaintDevice;
}

KisSelection::~KisSelection()
{
}

quint8 KisSelection::selected(qint32 x, qint32 y)
{
    KisHLineIteratorPixel iter = createHLineIterator(x, y, 1, false);

    quint8 *pix = iter.rawData();

    return *pix;
}

void KisSelection::setSelected(qint32 x, qint32 y, quint8 s)
{
    KisHLineIteratorPixel iter = createHLineIterator(x, y, 1, true);

    quint8 *pix = iter.rawData();

    *pix = s;
}

QImage KisSelection::maskImage()
{
    // If part of a KisAdjustmentLayer, there may be no parent device.
    QImage img;
    qint32 x, y, w, h, y2, x2;
    if (m_parentPaintDevice) {

        m_parentPaintDevice->exactBounds(x, y, w, h);
        img = QImage(w, h, 32);
    }
    else {
        x = 0;
        y = 0;
        w = image()->width();
        h = image()->width();
        img = QImage(w, h, 32);
    }

    for (y2 = y; y2 < h - y; ++y2) {
            KisHLineIteratorPixel it = createHLineIterator(x, y2, w, false);
            x2 = 0;
            while (!it.isDone()) {
                    quint8 s = MAX_SELECTED - *(it.rawData());
                    qint32 c = qRgb(s, s, s);
                    img.setPixel(x2, y2, c);
                    ++x2;
                    ++it;
            }
    }
    return img;
}
void KisSelection::select(QRect r)
{
    KisFillPainter painter(this);
    KisColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->getRGB8();
    painter.fillRect(r, KisColor(Qt::white, cs), MAX_SELECTED);
    qint32 x, y, w, h;
    extent(x, y, w, h);
}

void KisSelection::clear(QRect r)
{
    KisFillPainter painter(this);
    KisColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->getRGB8();
    painter.fillRect(r, KisColor(Qt::white, cs), MIN_SELECTED);
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
    KisRectIterator it = createRectIterator(x, y, w, h, true);
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

bool KisSelection::isTotallyUnselected(QRect r)
{
    if(*(m_datamanager->defaultPixel()) != MIN_SELECTED)
        return false;
    QRect sr = selectedExactRect();
    return ! r.intersects(sr);
}

QRect KisSelection::selectedRect()
{
    if(*(m_datamanager->defaultPixel()) == MIN_SELECTED || !m_parentPaintDevice)
        return extent();
    else
        return extent().unite(m_parentPaintDevice->extent());
}

QRect KisSelection::selectedExactRect()
{
    if(*(m_datamanager->defaultPixel()) == MIN_SELECTED || !m_parentPaintDevice)
        return exactBounds();
    else
        return exactBounds().unite(m_parentPaintDevice->exactBounds());
}

void KisSelection::paintUniformSelectionRegion(QImage img, const QRect& imageRect, const QRegion& uniformRegion)
{
    Q_ASSERT(img.size() == imageRect.size());
    Q_ASSERT(imageRect.contains(uniformRegion.boundingRect()));

    if (img.isNull() || img.size() != imageRect.size() || !imageRect.contains(uniformRegion.boundingRect())) {
        return;
    }

    if (*m_datamanager->defaultPixel() == MIN_SELECTED) {

        QRegion region = uniformRegion & QRegion(imageRect);

        if (!region.isEmpty()) {
            Q3MemArray<QRect> rects = region.rects();

            for (unsigned int i = 0; i < rects.count(); i++) {
                QRect r = rects[i];

                for (qint32 y = 0; y < r.height(); ++y) {

                    QRgb *imagePixel = reinterpret_cast<QRgb *>(img.scanLine(r.y() - imageRect.y() + y));
                    imagePixel += r.x() - imageRect.x();

                    qint32 numPixels = r.width();

                    while (numPixels > 0) {

                        QRgb srcPixel = *imagePixel;
                        quint8 srcGrey = (qRed(srcPixel) + qGreen(srcPixel) + qBlue(srcPixel)) / 9;
                        quint8 srcAlpha = qAlpha(srcPixel);

                        srcGrey = UINT8_MULT(srcGrey, srcAlpha);
                        quint8 dstAlpha = qMax(srcAlpha, 192);

                        QRgb dstPixel = qRgba(128 + srcGrey, 128 + srcGrey, 165 + srcGrey, dstAlpha);
                        *imagePixel = dstPixel;

                        ++imagePixel;
                        --numPixels;
                    }
                }
            }
        }
    }
}

void KisSelection::paintSelection(QImage img, qint32 imageRectX, qint32 imageRectY, qint32 imageRectWidth, qint32 imageRectHeight)
{
    Q_ASSERT(img.size() == QSize(imageRectWidth, imageRectHeight));

    if (img.isNull() || img.size() != QSize(imageRectWidth, imageRectHeight)) {
        return;
    }

    QRect imageRect(imageRectX, imageRectY, imageRectWidth, imageRectHeight);
    QRect selectionExtent = extent();

    selectionExtent.setLeft(selectionExtent.left() - 1);
    selectionExtent.setTop(selectionExtent.top() - 1);
    selectionExtent.setWidth(selectionExtent.width() + 2);
    selectionExtent.setHeight(selectionExtent.height() + 2);

    QRegion uniformRegion = QRegion(imageRect);
    uniformRegion -= QRegion(selectionExtent);

    if (!uniformRegion.isEmpty()) {
        paintUniformSelectionRegion(img, imageRect, uniformRegion);
    }

    QRect nonuniformRect = imageRect & selectionExtent;

    if (!nonuniformRect.isEmpty()) {

        const qint32 imageRectOffsetX = nonuniformRect.x() - imageRectX;
        const qint32 imageRectOffsetY = nonuniformRect.y() - imageRectY;

        imageRectX = nonuniformRect.x();
        imageRectY = nonuniformRect.y();
        imageRectWidth = nonuniformRect.width();
        imageRectHeight = nonuniformRect.height();

        const qint32 NUM_SELECTION_ROWS = 3;
    
        quint8 *selectionRow[NUM_SELECTION_ROWS];
    
        qint32 aboveRowIndex = 0;
        qint32 centreRowIndex = 1;
        qint32 belowRowIndex = 2;
    
        selectionRow[aboveRowIndex] = new quint8[imageRectWidth + 2];
        selectionRow[centreRowIndex] = new quint8[imageRectWidth + 2];
        selectionRow[belowRowIndex] = new quint8[imageRectWidth + 2];
    
        readBytes(selectionRow[centreRowIndex], imageRectX - 1, imageRectY - 1, imageRectWidth + 2, 1);
        readBytes(selectionRow[belowRowIndex], imageRectX - 1, imageRectY, imageRectWidth + 2, 1);
    
        for (qint32 y = 0; y < imageRectHeight; ++y) {
    
            qint32 oldAboveRowIndex = aboveRowIndex;
            aboveRowIndex = centreRowIndex;
            centreRowIndex = belowRowIndex;
            belowRowIndex = oldAboveRowIndex;
    
            readBytes(selectionRow[belowRowIndex], imageRectX - 1, imageRectY + y + 1, imageRectWidth + 2, 1);
    
            const quint8 *aboveRow = selectionRow[aboveRowIndex] + 1;
            const quint8 *centreRow = selectionRow[centreRowIndex] + 1;
            const quint8 *belowRow = selectionRow[belowRowIndex] + 1;
    
            QRgb *imagePixel = reinterpret_cast<QRgb *>(img.scanLine(imageRectOffsetY + y));
            imagePixel += imageRectOffsetX;
    
            for (qint32 x = 0; x < imageRectWidth; ++x) {
    
                quint8 centre = *centreRow;
    
                if (centre != MAX_SELECTED) {
    
                    // this is where we come if the pixels should be blue or bluish
    
                    QRgb srcPixel = *imagePixel;
                    quint8 srcGrey = (qRed(srcPixel) + qGreen(srcPixel) + qBlue(srcPixel)) / 9;
                    quint8 srcAlpha = qAlpha(srcPixel);
    
                    // Colour influence is proportional to alphaPixel.
                    srcGrey = UINT8_MULT(srcGrey, srcAlpha);
    
                    QRgb dstPixel;
    
                    if (centre == MIN_SELECTED) {
                        //this is where we come if the pixels should be blue (or red outline)
    
                        quint8 left = *(centreRow - 1);
                        quint8 right = *(centreRow + 1);
                        quint8 above = *aboveRow;
                        quint8 below = *belowRow;
    
                        // Stop unselected transparent areas from appearing the same
                        // as selected transparent areas.
                        quint8 dstAlpha = qMax(srcAlpha, 192);
    
                        // now for a simple outline based on 4-connectivity
                        if (left != MIN_SELECTED || right != MIN_SELECTED || above != MIN_SELECTED || below != MIN_SELECTED) {
                            dstPixel = qRgba(255, 0, 0, dstAlpha);
                        } else {
                            dstPixel = qRgba(128 + srcGrey, 128 + srcGrey, 165 + srcGrey, dstAlpha);
                        }
                    } else {
                        dstPixel = qRgba(UINT8_BLEND(qRed(srcPixel), srcGrey + 128, centre),
                                         UINT8_BLEND(qGreen(srcPixel), srcGrey + 128, centre),
                                         UINT8_BLEND(qBlue(srcPixel), srcGrey + 165, centre), 
                                         srcAlpha);
                    }
    
                    *imagePixel = dstPixel;
                }
    
                aboveRow++;
                centreRow++;
                belowRow++;
                imagePixel++;
            }
        }
    
        delete [] selectionRow[aboveRowIndex];
        delete [] selectionRow[centreRowIndex];
        delete [] selectionRow[belowRowIndex];
    }
}

void KisSelection::paintSelection(QImage img, const QRect& scaledImageRect, const QSize& scaledImageSize, const QSize& imageSize)
{
    if (img.isNull() || scaledImageRect.isEmpty() || scaledImageSize.isEmpty() || imageSize.isEmpty()) {
        return;
    }

    Q_ASSERT(img.size() == scaledImageRect.size());

    if (img.size() != scaledImageRect.size()) {
        return;
    }

    qint32 imageWidth = imageSize.width();
    qint32 imageHeight = imageSize.height();

    QRect selectionExtent = extent();

    selectionExtent.setLeft(selectionExtent.left() - 1);
    selectionExtent.setTop(selectionExtent.top() - 1);
    selectionExtent.setWidth(selectionExtent.width() + 2);
    selectionExtent.setHeight(selectionExtent.height() + 2);

    double xScale = static_cast<double>(scaledImageSize.width()) / imageWidth;
    double yScale = static_cast<double>(scaledImageSize.height()) / imageHeight;

    QRect scaledSelectionExtent;

    scaledSelectionExtent.setLeft(static_cast<int>(selectionExtent.left() * xScale));
    scaledSelectionExtent.setRight(static_cast<int>(ceil((selectionExtent.right() + 1) * xScale)) - 1);
    scaledSelectionExtent.setTop(static_cast<int>(selectionExtent.top() * yScale));
    scaledSelectionExtent.setBottom(static_cast<int>(ceil((selectionExtent.bottom() + 1) * yScale)) - 1);

    QRegion uniformRegion = QRegion(scaledImageRect);
    uniformRegion -= QRegion(scaledSelectionExtent);

    if (!uniformRegion.isEmpty()) {
        paintUniformSelectionRegion(img, scaledImageRect, uniformRegion);
    }

    QRect nonuniformRect = scaledImageRect & scaledSelectionExtent;

    if (!nonuniformRect.isEmpty()) {

        const qint32 scaledImageRectXOffset = nonuniformRect.x() - scaledImageRect.x();
        const qint32 scaledImageRectYOffset = nonuniformRect.y() - scaledImageRect.y();

        const qint32 scaledImageRectX = nonuniformRect.x();
        const qint32 scaledImageRectY = nonuniformRect.y();
        const qint32 scaledImageRectWidth = nonuniformRect.width();
        const qint32 scaledImageRectHeight = nonuniformRect.height();

        const qint32 imageRowLeft = static_cast<qint32>(scaledImageRectX / xScale);
        const qint32 imageRowRight = static_cast<qint32>((ceil((scaledImageRectX + scaledImageRectWidth - 1 + 1) / xScale)) - 1);

        const qint32 imageRowWidth = imageRowRight - imageRowLeft + 1;
        const qint32 imageRowStride = imageRowWidth + 2;

        const qint32 NUM_SELECTION_ROWS = 3;

        qint32 aboveRowIndex = 0;
        qint32 centreRowIndex = 1;
        qint32 belowRowIndex = 2;

        qint32 aboveRowSrcY = -3;
        qint32 centreRowSrcY = -3;
        qint32 belowRowSrcY = -3;

        quint8 *selectionRows = new quint8[imageRowStride * NUM_SELECTION_ROWS];
        quint8 *selectionRow[NUM_SELECTION_ROWS];

        selectionRow[0] = selectionRows + 1;
        selectionRow[1] = selectionRow[0] + imageRowStride;
        selectionRow[2] = selectionRow[0] + (2 * imageRowStride);

        for (qint32 y = 0; y < scaledImageRectHeight; ++y) {

            qint32 scaledY = scaledImageRectY + y;
            qint32 srcY = (scaledY * imageHeight) / scaledImageSize.height();

            quint8 *aboveRow;
            quint8 *centreRow;
            quint8 *belowRow;

            if (srcY - 1 == aboveRowSrcY) {
                aboveRow = selectionRow[aboveRowIndex];
                centreRow = selectionRow[centreRowIndex];
                belowRow = selectionRow[belowRowIndex];
            } else if (srcY - 1 == centreRowSrcY) {

                qint32 oldAboveRowIndex = aboveRowIndex;

                aboveRowIndex = centreRowIndex;
                centreRowIndex = belowRowIndex;
                belowRowIndex = oldAboveRowIndex;

                aboveRow = selectionRow[aboveRowIndex];
                centreRow = selectionRow[centreRowIndex];
                belowRow = selectionRow[belowRowIndex];

                readBytes(belowRow - 1, imageRowLeft - 1, srcY + 1, imageRowStride, 1);

            } else if (srcY - 1 == belowRowSrcY) {

                qint32 oldAboveRowIndex = aboveRowIndex;
                qint32 oldCentreRowIndex = centreRowIndex;

                aboveRowIndex = belowRowIndex;
                centreRowIndex = oldAboveRowIndex;
                belowRowIndex = oldCentreRowIndex;

                aboveRow = selectionRow[aboveRowIndex];
                centreRow = selectionRow[centreRowIndex];
                belowRow = selectionRow[belowRowIndex];

                if (belowRowIndex == centreRowIndex + 1) {
                    readBytes(centreRow - 1, imageRowLeft - 1, srcY, imageRowStride, 2);
                } else {
                    readBytes(centreRow - 1, imageRowLeft - 1, srcY, imageRowStride, 1);
                    readBytes(belowRow - 1, imageRowLeft - 1, srcY + 1, imageRowStride, 1);
                }

            } else {

                aboveRowIndex = 0;
                centreRowIndex = 1;
                belowRowIndex = 2;

                aboveRow = selectionRow[aboveRowIndex];
                centreRow = selectionRow[centreRowIndex];
                belowRow = selectionRow[belowRowIndex];

                readBytes(selectionRows, imageRowLeft - 1, srcY - 1, imageRowStride, NUM_SELECTION_ROWS);
            }

            aboveRowSrcY = srcY - 1;
            centreRowSrcY = aboveRowSrcY + 1;
            belowRowSrcY = centreRowSrcY + 1;

            QRgb *imagePixel = reinterpret_cast<QRgb *>(img.scanLine(scaledImageRectYOffset + y));
            imagePixel += scaledImageRectXOffset;

            for (qint32 x = 0; x < scaledImageRectWidth; ++x) {

                qint32 scaledX = scaledImageRectX + x;
                qint32 srcX = (scaledX * imageWidth) / scaledImageSize.width();

                quint8 centre = *(centreRow + srcX - imageRowLeft);

                if (centre != MAX_SELECTED) {

                    // this is where we come if the pixels should be blue or bluish

                    QRgb srcPixel = *imagePixel;
                    quint8 srcGrey = (qRed(srcPixel) + qGreen(srcPixel) + qBlue(srcPixel)) / 9;
                    quint8 srcAlpha = qAlpha(srcPixel);

                    // Colour influence is proportional to alphaPixel.
                    srcGrey = UINT8_MULT(srcGrey, srcAlpha);

                    QRgb dstPixel;

                    if (centre == MIN_SELECTED) {
                        //this is where we come if the pixels should be blue (or red outline)

                        quint8 left = *(centreRow + (srcX - imageRowLeft) - 1);
                        quint8 right = *(centreRow + (srcX - imageRowLeft) + 1);
                        quint8 above = *(aboveRow + (srcX - imageRowLeft));
                        quint8 below = *(belowRow + (srcX - imageRowLeft));

                        // Stop unselected transparent areas from appearing the same
                        // as selected transparent areas.
                        quint8 dstAlpha = qMax(srcAlpha, 192);

                        // now for a simple outline based on 4-connectivity
                        if (left != MIN_SELECTED || right != MIN_SELECTED || above != MIN_SELECTED || below != MIN_SELECTED) {
                            dstPixel = qRgba(255, 0, 0, dstAlpha);
                        } else {
                            dstPixel = qRgba(128 + srcGrey, 128 + srcGrey, 165 + srcGrey, dstAlpha);
                        }
                    } else {
                        dstPixel = qRgba(UINT8_BLEND(qRed(srcPixel), srcGrey + 128, centre),
                                         UINT8_BLEND(qGreen(srcPixel), srcGrey + 128, centre),
                                         UINT8_BLEND(qBlue(srcPixel), srcGrey + 165, centre), 
                                         srcAlpha);
                    }

                    *imagePixel = dstPixel;
                }

                imagePixel++;
            }
        }

        delete [] selectionRows;
    }
}

