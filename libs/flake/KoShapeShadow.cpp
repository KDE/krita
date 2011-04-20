/* This file is part of the KDE project
 * Copyright (C) 2008-2009 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 Ariya Hidayat <ariya.hidayat@gmail.com>
 * Copyright (C) 2010 Yue Liu <opuspace@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoShapeShadow.h"
#include "KoShapeGroup.h"
#include "KoShapeSavingContext.h"
#include "KoShapeBorderModel.h"
#include "KoShape.h"
#include "KoInsets.h"
#include "KoPathShape.h"
#include <KoGenStyle.h>
#include <KoViewConverter.h>
#include <kdebug.h>
#include <QtGui/QPainter>
#include <QtCore/QAtomicInt>
#include <QImage>

class KoShapeShadow::Private
{
public:
    Private()
            : offset(0, 0), color(Qt::black), blur(8), visible(true), refCount(0) {
    }
    QPointF offset;
    QColor color;
    qreal blur;
    bool visible;
    QAtomicInt refCount;
};

KoShapeShadow::KoShapeShadow()
        : d(new Private())
{
}

KoShapeShadow::~KoShapeShadow()
{
    delete d;
}

void KoShapeShadow::fillStyle(KoGenStyle &style, KoShapeSavingContext &context)
{
    Q_UNUSED(context);

    style.addProperty("draw:shadow", d->visible ? "visible" : "hidden");
    style.addProperty("draw:shadow-color", d->color.name());
    if (d->color.alphaF() != 1.0)
        style.addProperty("draw:shadow-opacity", QString("%1%").arg(d->color.alphaF() * 100.0));
    style.addProperty("draw:shadow-offset-x", QString("%1pt").arg(d->offset.x()));
    style.addProperty("draw:shadow-offset-y", QString("%1pt").arg(d->offset.y()));
    if (d->blur != 0)
        style.addProperty("calligra:shadow-blur-radius", QString("%1pt").arg(d->blur));
}

void KoShapeShadow::paintGroup(KoShapeGroup *group, QPainter &painter, const KoViewConverter &converter, QTransform &offsetMatrix)
{
    QList<KoShape*> shapes = group->shapes();
    foreach(KoShape *child, shapes) {
        // we paint recursively here, so we do not have to check recursively for visibility
        if (!child->isVisible())
            continue;
        KoShapeGroup *childGroup = dynamic_cast<KoShapeGroup*>(child);
        if (childGroup) {
            paintGroup(childGroup, painter, converter, offsetMatrix);
        } else {
            painter.save();
            paintShape(child, painter, converter, offsetMatrix);
            painter.restore();
        }
    }
}

//offsetMatrix left
void KoShapeShadow::paintShape(KoShape *shape, QPainter &painter, const KoViewConverter &converter, QTransform &offsetMatrix)
{
    if (shape->background()) {
        painter.save();
        KoShape::applyConversion(painter, converter);
        // the shadow direction is independent of the shapes transformation
        // please only change if you know what you are doing
        painter.setTransform(offsetMatrix * painter.transform());
        painter.setBrush(QBrush(d->color));
        QPainterPath path(shape->outline());
        KoPathShape * pathShape = dynamic_cast<KoPathShape*>(shape);
        if (pathShape)
            path.setFillRule(pathShape->fillRule());
        painter.drawPath(path);
        painter.restore();
    }

    if (shape->border()) {
        painter.save();
        QTransform oldPainterMatrix = painter.transform();
        KoShape::applyConversion(painter, converter);
        QTransform newPainterMatrix = painter.transform();
        // the shadow direction is independent of the shapes transformation
        // please only change if you know what you are doing
        painter.setTransform(offsetMatrix * painter.transform());
        // compensate applyConversion call in paint
        QTransform scaleMatrix = newPainterMatrix * oldPainterMatrix.inverted();
        painter.setTransform(scaleMatrix.inverted() * painter.transform());
        shape->border()->paint(shape, painter, converter);
        painter.restore();
    }
}

void KoShapeShadow::paint(KoShape *shape, QPainter &painter, const KoViewConverter &converter)
{
    if (! d->visible)
        return;

    // calculate the shadow offset independent of shape transformation
    QTransform tm;
    tm.translate(d->offset.x(), d->offset.y());
    QTransform tr = shape->absoluteTransformation(&converter);
    QTransform offsetMatrix = tr * tm * tr.inverted();

    QRectF shadowRect(0, 0, 0, 0);
    qreal shapeWidth;
    KoShapeGroup *group = dynamic_cast<KoShapeGroup*>(shape);

    if (group) {
        shadowRect.setSize(group->size());
        shapeWidth = group->size().width();
    } else {
        shadowRect.setSize(shape->boundingRect().size());
        shapeWidth = shape->boundingRect().width();
    }

    //convert relative radius to absolute radius
    qreal absBR = d->blur*0.01*shapeWidth;
    qreal expand = 3 * absBR; //blur would cause the shadow to be bigger
    QRectF clipRegion = shadowRect.adjusted(-expand, -expand, expand, expand);
    QRectF zoomedClipRegion = converter.documentToView(clipRegion);

    // determine the offset from the blur expand edge to the shadow bound's origin
    QPointF blurOffset(expand, expand);
    QPointF zoomedBlurOffset = converter.documentToView(blurOffset);

    QPointF clippingOffset = zoomedClipRegion.topLeft() + converter.documentToView(d->offset);

    // Init the buffer image
    QImage sourceGraphic(zoomedClipRegion.size().toSize(), QImage::Format_ARGB32_Premultiplied);
    sourceGraphic.fill(qRgba(0,0,0,0));

    // Init the buffer painter
    QPainter bufferPainter(&sourceGraphic);
    bufferPainter.translate(zoomedBlurOffset);
    bufferPainter.setPen(Qt::NoPen);
    bufferPainter.setBrush(Qt::NoBrush);
    bufferPainter.setRenderHint(QPainter::Antialiasing, painter.testRenderHint(QPainter::Antialiasing));

    if (group)
        paintGroup(group, bufferPainter, converter, offsetMatrix);
    else
        paintShape(shape, bufferPainter, converter, offsetMatrix);

    bufferPainter.end();
    blurShadow(sourceGraphic, absBR, d->color);
    // Paint the result
    painter.save();
    painter.drawImage(clippingOffset, sourceGraphic);
    painter.restore();
}

void KoShapeShadow::paintBuffer(QPointF &clippingOffset, QImage image, QPainter &painter, const KoViewConverter &converter) {
    kDebug() << "Shadow painted!";
}

void KoShapeShadow::setOffset(const QPointF & offset)
{
    d->offset = offset;
}

QPointF KoShapeShadow::offset() const
{
    return d->offset;
}

void KoShapeShadow::setColor(const QColor &color)
{
    d->color = color;
}

QColor KoShapeShadow::color() const
{
    return d->color;
}

void KoShapeShadow::setBlur(const qreal &blur)
{
    d->blur = blur;
}

qreal KoShapeShadow::blur() const
{
    return d->blur;
}

void KoShapeShadow::setVisible(bool visible)
{
    d->visible = visible;
}

bool KoShapeShadow::isVisible() const
{
    return d->visible;
}

void KoShapeShadow::insets(KoInsets &insets) const
{
    if (!d->visible) {
        insets.top = 0;
        insets.bottom = 0;
        insets.left = 0;
        insets.right = 0;
        return;
    }

    insets.left = (d->offset.x() < 0.0) ? qAbs(d->offset.x()) : 0.0;
    insets.top = (d->offset.y() < 0.0) ? qAbs(d->offset.y()) : 0.0;
    insets.right = (d->offset.x() > 0.0) ? d->offset.x() : 0.0;
    insets.bottom = (d->offset.y() > 0.0) ? d->offset.y() : 0.0;
}

bool KoShapeShadow::ref()
{
    return d->refCount.ref();
}

bool KoShapeShadow::deref()
{
    return d->refCount.deref();
}

int KoShapeShadow::useCount() const
{
    return d->refCount;
}

/* You can also find a BSD version to this method from
 * http://gitorious.org/ofi-labs/x2/blobs/master/graphics/shadowblur/
 */
void KoShapeShadow::blurShadow(QImage &image, int radius, const QColor& shadowColor)
{
    static const int BlurSumShift = 15;

    // Check http://www.w3.org/TR/SVG/filters.html#
    // As noted in the SVG filter specification, ru
    // approximates a real gaussian blur nicely.
    // See comments in http://webkit.org/b/40793, it seems sensible
    // to follow Skia's limit of 128 pixels for the blur radius.
    if (radius > 128)
        radius = 128;

    int channels[4] = { 3, 0, 1, 3 };
    int dmax = radius >> 1;
    int dmin = dmax - 1 + (radius & 1);
    if (dmin < 0)
        dmin = 0;

    // Two stages: horizontal and vertical
    for (int k = 0; k < 2; ++k) {

        unsigned char* pixels = image.bits();
        int stride = (k == 0) ? 4 : image.bytesPerLine();
        int delta = (k == 0) ? image.bytesPerLine() : 4;
        int jfinal = (k == 0) ? image.height() : image.width();
        int dim = (k == 0) ? image.width() : image.height();

        for (int j = 0; j < jfinal; ++j, pixels += delta) {

            // For each step, we blur the alpha in a channel and store the result
            // in another channel for the subsequent step.
            // We use sliding window algorithm to accumulate the alpha values.
            // This is much more efficient than computing the sum of each pixels
            // covered by the box kernel size for each x.

            for (int step = 0; step < 3; ++step) {
                int side1 = (step == 0) ? dmin : dmax;
                int side2 = (step == 1) ? dmin : dmax;
                int pixelCount = side1 + 1 + side2;
                int invCount = ((1 << BlurSumShift) + pixelCount - 1) / pixelCount;
                int ofs = 1 + side2;
                int alpha1 = pixels[channels[step]];
                int alpha2 = pixels[(dim - 1) * stride + channels[step]];
                unsigned char* ptr = pixels + channels[step + 1];
                unsigned char* prev = pixels + stride + channels[step];
                unsigned char* next = pixels + ofs * stride + channels[step];

                int i;
                int sum = side1 * alpha1 + alpha1;
                int limit = (dim < side2 + 1) ? dim : side2 + 1;
                for (i = 1; i < limit; ++i, prev += stride)
                    sum += *prev;
                if (limit <= side2)
                    sum += (side2 - limit + 1) * alpha2;

                limit = (side1 < dim) ? side1 : dim;
                for (i = 0; i < limit; ptr += stride, next += stride, ++i, ++ofs) {
                    *ptr = (sum * invCount) >> BlurSumShift;
                    sum += ((ofs < dim) ? *next : alpha2) - alpha1;
                }
                prev = pixels + channels[step];
                for (; ofs < dim; ptr += stride, prev += stride, next += stride, ++i, ++ofs) {
                    *ptr = (sum * invCount) >> BlurSumShift;
                    sum += (*next) - (*prev);
                }
                for (; i < dim; ptr += stride, prev += stride, ++i) {
                    *ptr = (sum * invCount) >> BlurSumShift;
                    sum += alpha2 - (*prev);
                }
            }
        }
    }

    // "Colorize" with the right shadow color.
    QPainter p(&image);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(image.rect(), shadowColor);
    p.end();
}
