/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#include "KoPatternBackground.h"
#include "KoShapeSavingContext.h"
#include "KoImageData.h"
#include "KoImageCollection.h"
#include <KoStyleStack.h>
#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include <KoXmlNS.h>
#include <KoOdfLoadingContext.h>
#include <KoOdfGraphicStyles.h>
#include <KoOdfStylesReader.h>
#include <KoStoreDevice.h>
#include <KoUnit.h>

#include <KDebug>

#include <QtGui/QBrush>
#include <QtGui/QPainter>

class KoPatternBackground::Private
{
public:
    Private()
            : repeat(Tiled), refPoint(TopLeft), imageCollection(0), imageData(0) {
    }

    ~Private() {
        delete imageData;
    }

    QSizeF targetSize() {
        QSizeF size = imageData->imageSize();
        if (targetImageSizePercent.width() > 0.0)
            size.setWidth(0.01 * targetImageSizePercent.width() * size.width());
        else if (targetImageSize.width() > 0.0)
            size.setWidth(targetImageSize.width());
        if (targetImageSizePercent.height() > 0.0)
            size.setHeight(0.01 * targetImageSizePercent.height() * size.height());
        else if (targetImageSize.height() > 0.0)
            size.setHeight(targetImageSize.height());

        return size;
    }

    QPointF offsetFromRect(const QRectF &fillRect, const QSizeF &imageSize) {
        QPointF offset;
        switch (refPoint) {
        case TopLeft:
            offset = fillRect.topLeft();
            break;
        case Top:
            offset.setX(fillRect.center().x() - 0.5 * imageSize.width());
            offset.setY(fillRect.top());
            break;
        case TopRight:
            offset.setX(fillRect.right() - imageSize.width());
            offset.setY(fillRect.top());
            break;
        case Left:
            offset.setX(fillRect.left());
            offset.setY(fillRect.center().y() - 0.5 * imageSize.height());
            break;
        case Center:
            offset.setX(fillRect.center().x() - 0.5 * imageSize.width());
            offset.setY(fillRect.center().y() - 0.5 * imageSize.height());
            break;
        case Right:
            offset.setX(fillRect.right() - imageSize.width());
            offset.setY(fillRect.center().y() - 0.5 * imageSize.height());
            break;
        case BottomLeft:
            offset.setX(fillRect.left());
            offset.setY(fillRect.bottom() - imageSize.height());
            break;
        case Bottom:
            offset.setX(fillRect.center().x() - 0.5 * imageSize.width());
            offset.setY(fillRect.bottom() - imageSize.height());
            break;
        case BottomRight:
            offset.setX(fillRect.right() - imageSize.width());
            offset.setY(fillRect.bottom() - imageSize.height());
            break;
        default:
            break;
        }
        if (refPointOffsetPercent.x() > 0.0)
            offset += QPointF(0.01 * refPointOffsetPercent.x() * imageSize.width(), 0);
        if (refPointOffsetPercent.y() > 0.0)
            offset += QPointF(0, 0.01 * refPointOffsetPercent.y() * imageSize.height());

        return offset;
    }

    QMatrix matrix;
    KoPatternBackground::PatternRepeat repeat;
    KoPatternBackground::ReferencePoint refPoint;
    QSizeF targetImageSize;
    QSizeF targetImageSizePercent;
    QPointF refPointOffsetPercent;
    QPointF tileRepeatOffsetPercent;
    KoImageCollection * imageCollection;
    KoImageData * imageData;
};

KoPatternBackground::KoPatternBackground(KoImageCollection * imageCollection)
        : d(new Private())
{
    d->imageCollection = imageCollection;
    Q_ASSERT(d->imageCollection);
}

KoPatternBackground::~KoPatternBackground()
{
    delete d;
}

void KoPatternBackground::setMatrix(const QMatrix &matrix)
{
    d->matrix = matrix;
}

QMatrix KoPatternBackground::matrix() const
{
    return d->matrix;
}

void KoPatternBackground::setPattern(const QImage &pattern)
{
    if (d->imageData)
        delete d->imageData;

    d->imageData = d->imageCollection->createImageData(pattern);
}

QImage KoPatternBackground::pattern()
{
    if (d->imageData)
        return d->imageData->image();
    return QImage();
}

void KoPatternBackground::setRepeat(PatternRepeat repeat)
{
    d->repeat = repeat;
}

KoPatternBackground::PatternRepeat KoPatternBackground::repeat() const
{
    return d->repeat;
}

KoPatternBackground::ReferencePoint KoPatternBackground::referencePoint() const
{
    return d->refPoint;
}

void KoPatternBackground::setReferencePoint(ReferencePoint referencePoint)
{
    d->refPoint = referencePoint;
}

QPointF KoPatternBackground::referencePointOffset() const
{
    return d->refPointOffsetPercent;
}

void KoPatternBackground::setReferencePointOffset(const QPointF &offset)
{
    qreal ox = qMax(qreal(0.0), qMin(qreal(100.0), offset.x()));
    qreal oy = qMax(qreal(0.0), qMin(qreal(100.0), offset.y()));

    d->refPointOffsetPercent = QPointF(ox, oy);
}

QPointF KoPatternBackground::tileRepeatOffset() const
{
    return d->tileRepeatOffsetPercent;
}

void KoPatternBackground::setTileRepeatOffset(const QPointF &offset)
{
    d->tileRepeatOffsetPercent = offset;
}

QSizeF KoPatternBackground::patternDisplaySize() const
{
    return d->targetSize();
}

void KoPatternBackground::setPatternDisplaySize(const QSizeF &size)
{
    d->targetImageSizePercent = QSizeF();
    d->targetImageSize = size;
}

QSizeF KoPatternBackground::patternOriginalSize() const
{
    return d->imageData->imageSize();
}

KoPatternBackground& KoPatternBackground::operator = (const KoPatternBackground & rhs)
{
    if (this == &rhs)
        return *this;

    d->matrix = rhs.d->matrix;
    d->repeat = rhs.d->repeat;
    d->refPoint = rhs.d->refPoint;
    d->targetImageSize = rhs.d->targetImageSize;
    d->targetImageSizePercent = rhs.d->targetImageSizePercent;
    d->refPointOffsetPercent = rhs.d->refPointOffsetPercent;
    d->tileRepeatOffsetPercent = rhs.d->tileRepeatOffsetPercent;
    d->imageCollection = rhs.d->imageCollection;

    if (rhs.d->imageData) {
        if (d->imageData) {
            *(d->imageData) = *(rhs.d->imageData);
        }
        else {
            d->imageData = new KoImageData(*rhs.d->imageData);
        }
    } else {
        delete d->imageData;
        d->imageData = 0;
    }

    return *this;
}

void KoPatternBackground::paint(QPainter &painter, const QPainterPath &fillPath) const
{
    if (! d->imageData)
        return;

    painter.save();

    if (d->repeat == Tiled) {
        // calculate scaling of pixmap
        QSizeF targetSize = d->targetSize();
        QSizeF imageSize = d->imageData->pixmap().size();
        qreal scaleX = targetSize.width() / imageSize.width();
        qreal scaleY = targetSize.height() / imageSize.height();

        QRectF targetRect = fillPath.boundingRect();
        // undo scaling on target rectangle
        targetRect.setWidth(targetRect.width() / scaleX);
        targetRect.setHeight(targetRect.height() / scaleY);

        // determine pattern offset
        QPointF offset = d->offsetFromRect(targetRect, d->imageData->pixmap().size());

        // create matrix for pixmap scaling
        QMatrix matrix;
        matrix.scale(scaleX, scaleY);

        painter.setClipPath(fillPath);
        painter.setWorldMatrix(matrix, true);
        painter.drawTiledPixmap(targetRect, d->imageData->pixmap(), -offset);
    } else if (d->repeat == Original) {
        QRectF sourceRect = QRectF(QPoint(0, 0), d->imageData->pixmap().size());
        QRectF targetRect = QRectF(QPoint(0, 0), d->targetSize());
        targetRect.moveCenter(fillPath.boundingRect().center());
        painter.setClipPath(fillPath);
        painter.drawPixmap(targetRect, d->imageData->pixmap(), sourceRect);
    } else if (d->repeat == Stretched) {
        QRectF sourceRect = QRectF(QPoint(0, 0), d->imageData->pixmap().size());
        QRectF targetRect = fillPath.boundingRect();
        painter.setClipPath(fillPath);
        painter.drawPixmap(targetRect, d->imageData->pixmap(), sourceRect);
    }

    painter.restore();
}

void KoPatternBackground::fillStyle(KoGenStyle &style, KoShapeSavingContext &context)
{
    if (! d->imageData)
        return;

    switch (d->repeat) {
    case Original:
        style.addProperty("style:repeat", "no-repeat");
        break;
    case Tiled:
        style.addProperty("style:repeat", "repeat");
        break;
    case Stretched:
        style.addProperty("style:repeat", "stretch");
        break;
    }

    if (d->repeat == Tiled) {
        QString refPointId = "top-left";
        switch (d->refPoint) {
        case TopLeft: refPointId = "top-left"; break;
        case Top: refPointId = "top"; break;
        case TopRight: refPointId = "top-right"; break;
        case Left: refPointId = "left"; break;
        case Center: refPointId = "center"; break;
        case Right: refPointId = "right"; break;
        case BottomLeft: refPointId = "bottom-left"; break;
        case Bottom: refPointId = "bottom"; break;
        case BottomRight: refPointId = "bottom-right"; break;
        }
        style.addProperty("draw:fill-image-ref-point", refPointId);
        if (d->refPointOffsetPercent.x() > 0.0)
            style.addProperty("draw:fill-image-ref-point-x", QString("%1%").arg(d->refPointOffsetPercent.x()));
        if (d->refPointOffsetPercent.y() > 0.0)
            style.addProperty("draw:fill-image-ref-point-y", QString("%1%").arg(d->refPointOffsetPercent.y()));
    }

    if (d->repeat != Stretched) {
        QSizeF targetSize = d->targetSize();
        QSizeF imageSize = d->imageData->imageSize();
        if (targetSize.height() != imageSize.height())
            style.addProperty("draw:fill-image-height", QString("%1").arg(targetSize.height()));
        if (targetSize.width() != imageSize.width())
            style.addProperty("draw:fill-image-width", QString("%1").arg(targetSize.width()));
    }

    KoGenStyle patternStyle(KoGenStyle::StyleFillImage /*no family name*/);
    patternStyle.addAttribute("xlink:show", "embed");
    patternStyle.addAttribute("xlink:actuate", "onLoad");
    patternStyle.addAttribute("xlink:type", "simple");
    patternStyle.addAttribute("xlink:href", context.imageHref(d->imageData));

    QString patternStyleName = context.mainStyles().lookup(patternStyle, "picture");
    context.mainStyles().lookup(style, context.isSet(KoShapeSavingContext::PresentationShape) ? "pr" : "gr");
    style.addProperty("draw:fill", "bitmap");
    style.addProperty("draw:fill-image-name", patternStyleName);

    context.addDataCenter(d->imageCollection);
}

bool KoPatternBackground::loadStyle(KoOdfLoadingContext & context, const QSizeF &)
{
    KoStyleStack &styleStack = context.styleStack();
    if (! styleStack.hasProperty(KoXmlNS::draw, "fill"))
        return false;

    QString fillStyle = styleStack.property(KoXmlNS::draw, "fill");
    if (fillStyle != "bitmap")
        return false;

    QString styleName = styleStack.property(KoXmlNS::draw, "fill-image-name");

    KoXmlElement* e = context.stylesReader().drawStyles()[styleName];
    if (! e)
        return false;

    const QString href = e->attributeNS(KoXmlNS::xlink, "href", QString());
    if (href.isEmpty())
        return false;

    delete d->imageData;
    d->imageData = d->imageCollection->createImageData(href,context.store());
    if (! d->imageData)
        return false;

    // read the pattern repeat style
    QString style = styleStack.property(KoXmlNS::style, "repeat");
    if (style == "stretch")
        d->repeat = Stretched;
    else if (style == "no-repeat")
        d->repeat = Original;
    else
        d->repeat = Tiled;

    if (style != "stretch") {
        // optional attributes which can override original image size
        if (styleStack.hasProperty(KoXmlNS::draw, "fill-image-height")) {
            QString height = styleStack.property(KoXmlNS::draw, "fill-image-height");
            if (height.endsWith('%'))
                d->targetImageSizePercent.setHeight(height.remove('%').toDouble());
            else
                d->targetImageSize.setHeight(KoUnit::parseValue(height));
        }
        if (styleStack.hasProperty(KoXmlNS::draw, "fill-image-width")) {
            QString width = styleStack.property(KoXmlNS::draw, "fill-image-width");
            if (width.endsWith('%'))
                d->targetImageSizePercent.setWidth(width.remove('%').toDouble());
            else
                d->targetImageSize.setWidth(KoUnit::parseValue(width));
        }
    }

    if (style == "repeat") {
        if (styleStack.hasProperty(KoXmlNS::draw, "fill-image-ref-point")) {
            // align pattern to the given size
            QString align = styleStack.property(KoXmlNS::draw, "fill-image-ref-point");
            if (align == "top-left")
                d->refPoint = TopLeft;
            else if (align == "top")
                d->refPoint = Top;
            else if (align == "top-right")
                d->refPoint = TopRight;
            else if (align == "left")
                d->refPoint = Left;
            else if (align == "center")
                d->refPoint = Center;
            else if (align == "right")
                d->refPoint = Right;
            else if (align == "bottom-left")
                d->refPoint = BottomLeft;
            else if (align == "bottom")
                d->refPoint = Bottom;
            else if (align == "bottom-right")
                d->refPoint = BottomRight;
        }
        if (styleStack.hasProperty(KoXmlNS::draw, "fill-image-ref-point-x")) {
            QString pointX = styleStack.property(KoXmlNS::draw, "fill-image-ref-point-x");
            d->refPointOffsetPercent.setX(pointX.remove('%').toDouble());
        }
        if (styleStack.hasProperty(KoXmlNS::draw, "fill-image-ref-point-y")) {
            QString pointY = styleStack.property(KoXmlNS::draw, "fill-image-ref-point-y");
            d->refPointOffsetPercent.setY(pointY.remove('%').toDouble());
        }
        if (styleStack.hasProperty(KoXmlNS::draw, "tile-repeat-offset")) {
            QString repeatOffset = styleStack.property(KoXmlNS::draw, "tile-repeat-offset");
            QStringList tokens = repeatOffset.split('%');
            if (tokens.count() == 2) {
                QString direction = tokens[1].simplified();
                if (direction == "horizontal")
                    d->tileRepeatOffsetPercent.setX(tokens[0].toDouble());
                else if (direction == "vertical")
                    d->tileRepeatOffsetPercent.setY(tokens[0].toDouble());
            }
        }
    }

    return true;
}

QRectF KoPatternBackground::patternRectFromFillSize(const QSizeF &size)
{
    QRectF rect;

    switch (d->repeat) {
    case Tiled:
        rect.setTopLeft(d->offsetFromRect(QRectF(QPointF(), size), d->targetSize()));
        rect.setSize(d->targetSize());
        break;
    case Original:
        rect.setLeft(0.5 * (size.width() - d->targetSize().width()));
        rect.setTop(0.5 * (size.height() - d->targetSize().height()));
        rect.setSize(d->targetSize());
        break;
    case Stretched:
        rect.setTopLeft(QPointF(0.0, 0.0));
        rect.setSize(size);
        break;
    }

    return rect;
}
