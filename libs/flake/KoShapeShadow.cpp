/* This file is part of the KDE project
 * Copyright (C) 2008-2009 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 Ariya Hidayat <ariya.hidayat@gmail.com>
 * Copyright (C) 2010-2011 Yue Liu <yue.liu@mail.com>
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
#include "KoSelection.h"
#include "KoShapeSavingContext.h"
#include "KoShapeStrokeModel.h"
#include "KoShape.h"
#include "KoInsets.h"
#include "KoPathShape.h"
#include <KoGenStyle.h>
#include <FlakeDebug.h>
#include <QPainter>
#include <QAtomicInt>
#include <QImage>
#include <QRectF>

class Q_DECL_HIDDEN KoShapeShadow::Private
{
public:
    Private()
            : offset(2, 2), color(Qt::black), blur(8), visible(true), refCount(0) {
    }
    QPointF offset;
    QColor color;
    qreal blur;
    bool visible;
    QAtomicInt refCount;

    /**
     * Paints the shadow of the shape group to the buffer image.
     * @param group the shape group to paint around
     * @param painter the painter to paint on the image
     * @param converter to convert between internal and view coordinates.
     */
    void paintGroupShadow(KoShapeGroup *group, QPainter &painter);
    /**
     * Paints the shadow of the shape to the buffer image.
     * @param shape the shape to paint around
     * @param painter the painter to paint on the image
     */
    void paintShadow(KoShape *shape, QPainter &painter);
    void blurShadow(QImage &image, int radius, const QColor& shadowColor);
};

void KoShapeShadow::Private::paintGroupShadow(KoShapeGroup *group, QPainter &painter)
{
    QList<KoShape*> shapes = group->shapes();
    Q_FOREACH (KoShape *child, shapes) {
        // we paint recursively here, so we do not have to check recursively for visibility
        if (!child->isVisible(false))
            continue;
        painter.save();
        //apply group child's transformation
        painter.setTransform(child->absoluteTransformation(), true);
        paintShadow(child, painter);
        painter.restore();
    }
}

void KoShapeShadow::Private::paintShadow(KoShape *shape, QPainter &painter)
{
    QPainterPath path(shape->shadowOutline());
    if (!path.isEmpty()) {
        painter.save();
        painter.setBrush(QBrush(color));

        // Make sure the shadow has the same fill rule as the shape.
        KoPathShape * pathShape = dynamic_cast<KoPathShape*>(shape);
        if (pathShape)
            path.setFillRule(pathShape->fillRule());

        painter.drawPath(path);
        painter.restore();
    }

    if (shape->stroke()) {
        shape->stroke()->paint(shape, painter);
    }
}

/* You can also find a BSD version to this method from
 * http://gitorious.org/ofi-labs/x2/blobs/master/graphics/shadowblur/
 */
void KoShapeShadow::Private::blurShadow(QImage &image, int radius, const QColor& shadowColor)
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


// ----------------------------------------------------------------
//                         KoShapeShadow


KoShapeShadow::KoShapeShadow()
        : d(new Private())
{
}

KoShapeShadow::~KoShapeShadow()
{
    delete d;
}

KoShapeShadow::KoShapeShadow(const KoShapeShadow &rhs)
    : d(new Private(*rhs.d))
{
    d->refCount = 0;
}

KoShapeShadow& KoShapeShadow::operator=(const KoShapeShadow &rhs)
{
    *d = *rhs.d;
    d->refCount = 0;
    return *this;
}

void KoShapeShadow::fillStyle(KoGenStyle &style, KoShapeSavingContext &context)
{
    Q_UNUSED(context);

    style.addProperty("draw:shadow", d->visible ? "visible" : "hidden", KoGenStyle::GraphicType);
    style.addProperty("draw:shadow-color", d->color.name(), KoGenStyle::GraphicType);
    if (d->color.alphaF() != 1.0)
        style.addProperty("draw:shadow-opacity", QString("%1%").arg(d->color.alphaF() * 100.0), KoGenStyle::GraphicType);
    style.addProperty("draw:shadow-offset-x", QString("%1pt").arg(d->offset.x()), KoGenStyle::GraphicType);
    style.addProperty("draw:shadow-offset-y", QString("%1pt").arg(d->offset.y()), KoGenStyle::GraphicType);
    if (d->blur != 0)
        style.addProperty("calligra:shadow-blur-radius", QString("%1pt").arg(d->blur), KoGenStyle::GraphicType);
}

void KoShapeShadow::paint(KoShape *shape, QPainter &painter)
{
    if (! d->visible)
        return;

    // So the approach we are taking here is to draw into a buffer image the size of boundingRect
    // We offset by the shadow offset at the time we draw into the buffer
    // Then we filter the image and draw it at the position of the bounding rect on canvas

    QTransform documentToView = painter.transform();

    //the boundingRect of the shape or the KoSelection boundingRect of the group
    QRectF shadowRect = shape->boundingRect();
    QRectF zoomedClipRegion = documentToView.mapRect(shadowRect);

    // Init the buffer image
    QImage sourceGraphic(zoomedClipRegion.size().toSize(), QImage::Format_ARGB32_Premultiplied);
    sourceGraphic.fill(qRgba(0,0,0,0));
    // Init the buffer painter
    QPainter imagePainter(&sourceGraphic);
    imagePainter.setPen(Qt::NoPen);
    imagePainter.setBrush(Qt::NoBrush);
    imagePainter.setRenderHint(QPainter::Antialiasing, painter.testRenderHint(QPainter::Antialiasing));
    // Since our imagebuffer and the canvas don't align we need to offset our drawings
    imagePainter.translate(-1.0f*documentToView.map(shadowRect.topLeft()));

    // Handle the shadow offset
    imagePainter.translate(documentToView.map(offset()));

    KoShapeGroup *group = dynamic_cast<KoShapeGroup*>(shape);
    if (group) {
        d->paintGroupShadow(group, imagePainter);
    } else {
        //apply shape's transformation
        imagePainter.setTransform(shape->absoluteTransformation(), true);

        d->paintShadow(shape, imagePainter);
    }
    imagePainter.end();

    // Blur the shadow (well the entire buffer)
    d->blurShadow(sourceGraphic, qRound(documentToView.m11() * d->blur), d->color);

    // Paint the result
    painter.save();
    // The painter is initialized for us with canvas transform 'plus' shape transform
    // we are only interested in the canvas transform so 'subtract' the shape transform part
    painter.setTransform(shape->absoluteTransformation().inverted() * painter.transform());
    painter.drawImage(zoomedClipRegion.topLeft(), sourceGraphic);
    painter.restore();
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

void KoShapeShadow::setBlur(qreal blur)
{
    // force positive blur radius
    d->blur = qAbs(blur);
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

    qreal expand = d->blur;

    insets.left = (d->offset.x() < 0.0) ? qAbs(d->offset.x()) : 0.0;
    insets.top = (d->offset.y() < 0.0) ? qAbs(d->offset.y()) : 0.0;
    insets.right = (d->offset.x() > 0.0) ? d->offset.x() : 0.0;
    insets.bottom = (d->offset.y() > 0.0) ? d->offset.y() : 0.0;

    insets.left += expand;
    insets.top += expand;
    insets.right += expand;
    insets.bottom += expand;
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
