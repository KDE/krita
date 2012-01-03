/*
 *  Copyright (c) 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
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

//libc
#include <cmath>
//Qt
#include <QColor>
#include <QImage>
#include <QPainter>
#include <QPainterPath>

//pigment
#include <KoColor.h>

//Krita
#include "kis_fixed_painter.h"
#include "kis_fixed_paint_device.h"

static const QColor BLACK(Qt::black);
static const QBrush KIS_WHITE_BRUSH(Qt::white);

struct KisFixedPainter::Private {
    KisFixedPaintDeviceSP device;
    QImage polygonMaskImage;
    KoColor paintColor;
};

KisFixedPainter::KisFixedPainter():d(new Private())
{

}

KisFixedPainter::KisFixedPainter(KisFixedPaintDeviceSP device):d(new Private())
{
    d->device = device;
}

KisFixedPainter::~KisFixedPainter()
{
    delete d;
}



void KisFixedPainter::init()
{
    //d->polygonMaskImage = QImage(4, 4, QImage::Format_ARGB32_Premultiplied);
}


QRect KisFixedPainter::fillPainterPath(const QPainterPath& path)
{
    // anti-aliasing
    QRectF boundingRect = path.boundingRect();
    QRect fillRect;

    fillRect.setTop((qint32)floor(boundingRect.top()));
    fillRect.setLeft((qint32)floor(boundingRect.left()));
    fillRect.setBottom((qint32)ceil(boundingRect.bottom()));
    fillRect.setRight((qint32)ceil(boundingRect.right()));
    // Expand the rectangle to allow for anti-aliasing.
    fillRect.adjust(-1, -1, 1, 1);

    QSize fillRectSize = fillRect.size();
    if (d->polygonMaskImage.isNull() ||
        d->polygonMaskImage.width() < fillRectSize.width() ||
        d->polygonMaskImage.height() < fillRectSize.height()) {

        d->polygonMaskImage = QImage(fillRectSize.width(), fillRectSize.height(), QImage::Format_ARGB32_Premultiplied);
    }
    d->polygonMaskImage.fill( BLACK.rgb() );

    // save to QImage
    QPainter pathPainter(&d->polygonMaskImage);
    pathPainter.setRenderHint(QPainter::Antialiasing, true);
    pathPainter.translate(-fillRect.topLeft());
    pathPainter.fillPath(path, KIS_WHITE_BRUSH);

    //convert to device
    QRect polygonRect(0,0,fillRectSize.width(), fillRectSize.height());
    d->device->setRect(polygonRect);

    if (d->device->allocatedPixels() < fillRectSize.width() * fillRectSize.height()) {
        // pre-allocate the memory to double to avoid reallocations
        d->device->setRect(QRect(0,0,fillRectSize.width() * 2 ,fillRectSize.height() * 2));
        d->device->initialize();
        d->device->setRect(polygonRect);
    }

    // fill the area with color
    quint8 * data = d->device->data();
    quint8 pixelSize = d->device->pixelSize();
    int rowSize = fillRectSize.width() * pixelSize;
    quint8 * colorLine = new quint8[ rowSize ];
    quint8 * alphaLine = new quint8[ fillRectSize.width() ]; // alpha has pixelSize == 1
    quint8 * it = alphaLine;

    // prepare the color line
    quint8 * pit = colorLine;
    for (int i = 0; i < fillRectSize.width(); i++){
        memcpy(pit, d->paintColor.data(), pixelSize);
        pit += pixelSize;
    }

    const KoColorSpace * cs = d->device->colorSpace();
    for (int y = 0; y < fillRectSize.height(); y++){
        QRgb * line = reinterpret_cast<QRgb*>(d->polygonMaskImage.scanLine(y));
        for (int x = 0; x < fillRectSize.width(); x++){
            *it = quint8(qRed(line[x]));
            it += 1;
        }

        memcpy( data, colorLine, rowSize);
        cs->applyAlphaU8Mask( data, alphaLine, fillRectSize.width() );
        data += rowSize;
        it = alphaLine;
    }
    delete [] alphaLine;
    delete [] colorLine;

    return QRect(fillRect.topLeft(),polygonRect.size()).normalized();
}

void KisFixedPainter::setPaintColor(const KoColor& color)
{
    d->paintColor = color;
}
