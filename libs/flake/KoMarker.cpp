/* This file is part of the KDE project
   Copyright (C) 2011 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoMarker.h"

#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include "KoPathShape.h"
#include "KoPathShapeLoader.h"
#include "KoShapeLoadingContext.h"
#include "KoShapeSavingContext.h"
#include "KoOdfWorkaround.h"
#include "KoShapePainter.h"
#include "KoViewConverter.h"

#include <QString>
#include <QUrl>
#include <QPainterPath>
#include <QPainter>

#include "kis_global.h"
#include "kis_algebra_2d.h"

class Q_DECL_HIDDEN KoMarker::Private
{
public:
    Private()
        : coordinateSystem(StrokeWidth),
          referenceSize(3,3),
          hasAutoOrientation(false),
          explicitOrientation(0)
    {}

    ~Private() {
        qDeleteAll(shapes);
    }

    bool operator==(const KoMarker::Private &other) const
    {
        // WARNING: comparison of shapes is extremely fuzzy! Don't
        //          trust it in life-critical cases!

        return name == other.name &&
            coordinateSystem == other.coordinateSystem &&
            referencePoint == other.referencePoint &&
            referenceSize == other.referenceSize &&
            hasAutoOrientation == other.hasAutoOrientation &&
            explicitOrientation == other.explicitOrientation &&
            compareShapesTo(other.shapes);
    }

    Private(const Private &rhs)
        : name(rhs.name),
          coordinateSystem(rhs.coordinateSystem),
          referencePoint(rhs.referencePoint),
          referenceSize(rhs.referenceSize),
          hasAutoOrientation(rhs.hasAutoOrientation),
          explicitOrientation(rhs.explicitOrientation)
    {
        Q_FOREACH (KoShape *shape, rhs.shapes) {
            shapes << shape->cloneShape();
        }
    }

    QString name;
    MarkerCoordinateSystem coordinateSystem;
    QPointF referencePoint;
    QSizeF referenceSize;

    bool hasAutoOrientation;
    qreal explicitOrientation;

    QList<KoShape*> shapes;

    bool compareShapesTo(const QList<KoShape*> other) const {
        if (shapes.size() != other.size()) return false;

        for (int i = 0; i < shapes.size(); i++) {
            if (shapes[i]->outline() != other[i]->outline() ||
                shapes[i]->absoluteTransformation(0) != other[i]->absoluteTransformation(0)) {

                return false;
            }
        }

        return true;
    }

    QTransform markerTransform(qreal strokeWidth, qreal nodeAngle, const QPointF &pos = QPointF()) {
        const QTransform translate = QTransform::fromTranslate(referencePoint.x(), referencePoint.y());

        QTransform t = translate;

        if (coordinateSystem == StrokeWidth) {
            t *= QTransform::fromScale(strokeWidth, strokeWidth);
        }

        const qreal angle = hasAutoOrientation ? nodeAngle : explicitOrientation;
        if (angle != 0.0) {
            QTransform r;
            r.rotateRadians(angle);
            t *= r;
        }

        t *= QTransform::fromTranslate(pos.x(), pos.y());

        return t;
    }
};

KoMarker::KoMarker()
: d(new Private())
{
}

KoMarker::~KoMarker()
{
    delete d;
}

QString KoMarker::name() const
{
    return d->name;
}

KoMarker::KoMarker(const KoMarker &rhs)
    : QSharedData(rhs),
      d(new Private(*rhs.d))
{
}

bool KoMarker::operator==(const KoMarker &other) const
{
    return *d == *other.d;
}

void KoMarker::setCoordinateSystem(KoMarker::MarkerCoordinateSystem value)
{
    d->coordinateSystem = value;
}

KoMarker::MarkerCoordinateSystem KoMarker::coordinateSystem() const
{
    return d->coordinateSystem;
}

KoMarker::MarkerCoordinateSystem KoMarker::coordinateSystemFromString(const QString &value)
{
    MarkerCoordinateSystem result = StrokeWidth;

    if (value == "userSpaceOnUse") {
        result = UserSpaceOnUse;
    }

    return result;
}

QString KoMarker::coordinateSystemToString(KoMarker::MarkerCoordinateSystem value)
{
    return
        value == StrokeWidth ?
        "strokeWidth" :
                "userSpaceOnUse";
}

void KoMarker::setReferencePoint(const QPointF &value)
{
    d->referencePoint = value;
}

QPointF KoMarker::referencePoint() const
{
    return d->referencePoint;
}

void KoMarker::setReferenceSize(const QSizeF &size)
{
    d->referenceSize = size;
}

QSizeF KoMarker::referenceSize() const
{
    return d->referenceSize;
}

bool KoMarker::hasAutoOtientation() const
{
    return d->hasAutoOrientation;
}

void KoMarker::setAutoOrientation(bool value)
{
    d->hasAutoOrientation = value;
}

qreal KoMarker::explicitOrientation() const
{
    return d->explicitOrientation;
}

void KoMarker::setExplicitOrientation(qreal value)
{
    d->explicitOrientation = value;
}

void KoMarker::setShapes(const QList<KoShape *> &shapes)
{
    d->shapes = shapes;
}

QList<KoShape *> KoMarker::shapes() const
{
    return d->shapes;
}

void KoMarker::paintAtPosition(QPainter *painter, const QPointF &pos, qreal strokeWidth, qreal nodeAngle)
{
    QTransform oldTransform = painter->transform();

    KoViewConverter converter;
    KoShapePainter p;
    p.setShapes(d->shapes);

    painter->translate(pos);

    const qreal angle = d->hasAutoOrientation ? nodeAngle : d->explicitOrientation;
    painter->rotate(kisRadiansToDegrees(angle));

    if (d->coordinateSystem == StrokeWidth) {
        painter->scale(strokeWidth, strokeWidth);
    }

    painter->translate(-d->referencePoint);
    p.paint(*painter, converter);

    painter->setTransform(oldTransform);
}

qreal KoMarker::maxInset(qreal strokeWidth) const
{
    QRectF shapesBounds = boundingRect(strokeWidth, 0.0); // normalized to 0,0
    qreal result = 0.0;

    result = qMax(KisAlgebra2D::norm(shapesBounds.topLeft()), result);
    result = qMax(KisAlgebra2D::norm(shapesBounds.topRight()), result);
    result = qMax(KisAlgebra2D::norm(shapesBounds.bottomLeft()), result);
    result = qMax(KisAlgebra2D::norm(shapesBounds.bottomRight()), result);

    if (d->coordinateSystem == StrokeWidth) {
        result *= strokeWidth;
    }

    return result;
}

QRectF KoMarker::boundingRect(qreal strokeWidth, qreal nodeAngle) const
{
    QRectF shapesBounds;

    Q_FOREACH (KoShape *shape, d->shapes) {
        shapesBounds |= shape->boundingRect();
    }

    const QTransform t = d->markerTransform(strokeWidth, nodeAngle);

    if (!t.isIdentity()) {
        shapesBounds = t.mapRect(shapesBounds);
    }

    return shapesBounds;
}

QPainterPath KoMarker::outline(qreal strokeWidth, qreal nodeAngle) const
{
    QPainterPath outline;
    Q_FOREACH (KoShape *shape, d->shapes) {
        outline |= shape->absoluteTransformation(0).map(shape->outline());
    }

    const QTransform t = d->markerTransform(strokeWidth, nodeAngle);

    if (!t.isIdentity()) {
        outline = t.map(outline);
    }

    return outline;
}

void KoMarker::drawPreview(QPainter *painter, const QRectF &previewRect, const QPen &pen, KoFlake::MarkerPosition position)
{
    const QRectF outlineRect = outline(pen.widthF(), 0).boundingRect(); // normalized to 0,0
    QPointF marker;
    QPointF start;
    QPointF end;

    if (position == KoFlake::StartMarker) {
        marker = QPointF(-outlineRect.left() + previewRect.left(), previewRect.center().y());
        start = marker;
        end = QPointF(previewRect.right(), start.y());
    } else if (position == KoFlake::MidMarker) {
        start = QPointF(previewRect.left(), previewRect.center().y());
        marker = QPointF(-outlineRect.center().x() + previewRect.center().x(), start.y());
        end = QPointF(previewRect.right(), start.y());
    } else if (position == KoFlake::EndMarker) {
        start = QPointF(previewRect.left(), previewRect.center().y());
        marker = QPointF(-outlineRect.right() + previewRect.right(), start.y());
        end = marker;
    }

    painter->save();
    painter->setPen(pen);
    painter->setClipRect(previewRect);

    painter->drawLine(start, end);
    paintAtPosition(painter, marker, pen.widthF(), 0);

    painter->restore();
}
