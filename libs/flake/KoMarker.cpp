/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2011 Thorsten Zachmann <zachmann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KoMarker.h"

#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include "KoPathShape.h"
#include "KoPathShapeLoader.h"
#include "KoShapeLoadingContext.h"
#include "KoShapeSavingContext.h"
#include "KoShapePainter.h"
#include <KoShapeStroke.h>
#include <KoGradientBackground.h>
#include <KoColorBackground.h>


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
        // shape manager that is stored in the painter should be destroyed
        // before the shapes themselves
        shapePainter.reset();
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
    QScopedPointer<KoShapePainter> shapePainter;

    bool compareShapesTo(const QList<KoShape*> other) const {
        if (shapes.size() != other.size()) return false;

        for (int i = 0; i < shapes.size(); i++) {
            if (shapes[i]->outline() != other[i]->outline() ||
                shapes[i]->absoluteTransformation() != other[i]->absoluteTransformation()) {

                return false;
            }
        }

        return true;
    }

    QTransform markerTransform(qreal strokeWidth, qreal nodeAngle, const QPointF &pos = QPointF()) {
        const QTransform translate = QTransform::fromTranslate(-referencePoint.x(), -referencePoint.y());

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

    if (d->shapePainter) {
        d->shapePainter->setShapes(shapes);
    }
}

QList<KoShape *> KoMarker::shapes() const
{
    return d->shapes;
}

void KoMarker::paintAtPosition(QPainter *painter, const QPointF &pos, qreal strokeWidth, qreal nodeAngle)
{
    QTransform oldTransform = painter->transform();

    if (!d->shapePainter) {
        d->shapePainter.reset(new KoShapePainter());
        d->shapePainter->setShapes(d->shapes);
    }

    painter->setTransform(d->markerTransform(strokeWidth, nodeAngle, pos), true);
    d->shapePainter->paint(*painter);

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

    return result;
}

QRectF KoMarker::boundingRect(qreal strokeWidth, qreal nodeAngle) const
{
    QRectF shapesBounds = KoShape::boundingRect(d->shapes);

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
        outline |= shape->absoluteTransformation().map(shape->outline());
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

void KoMarker::applyShapeStroke(const KoShape *parentShape, KoShapeStroke *stroke, const QPointF &pos, qreal strokeWidth, qreal nodeAngle)
{
    const QGradient *originalGradient = stroke->lineBrush().gradient();

    if (!originalGradient) {
        QList<KoShape*> linearizedShapes = KoShape::linearizeSubtree(d->shapes);
        Q_FOREACH(KoShape *shape, linearizedShapes) {
            // update the stroke
            KoShapeStrokeSP shapeStroke = shape->stroke() ?
                        qSharedPointerDynamicCast<KoShapeStroke>(shape->stroke()) :
                        KoShapeStrokeSP();

            if (shapeStroke) {
                shapeStroke = toQShared(new KoShapeStroke(*shapeStroke));

                shapeStroke->setLineBrush(QBrush());
                shapeStroke->setColor(stroke->color());

                shape->setStroke(shapeStroke);
            }

            // update the background
            if (shape->background()) {
                QSharedPointer<KoColorBackground> bg(new KoColorBackground(stroke->color()));
                shape->setBackground(bg);
            }
        }
    } else {
        QScopedPointer<QGradient> g(KoFlake::cloneGradient(originalGradient));
        KIS_ASSERT_RECOVER_RETURN(g);

        const QTransform markerTransformInverted =
                d->markerTransform(strokeWidth, nodeAngle, pos).inverted();

        QTransform gradientToUser;

        // Unwrap the gradient to work in global mode
        if (g->coordinateMode() == QGradient::ObjectBoundingMode) {
            QRectF boundingRect =
                parentShape ?
                parentShape->outline().boundingRect() :
                this->boundingRect(strokeWidth, nodeAngle);

            boundingRect = KisAlgebra2D::ensureRectNotSmaller(boundingRect, QSizeF(1.0, 1.0));

            gradientToUser = QTransform(boundingRect.width(), 0, 0, boundingRect.height(),
                                        boundingRect.x(), boundingRect.y());

            g->setCoordinateMode(QGradient::LogicalMode);
        }

        QList<KoShape*> linearizedShapes = KoShape::linearizeSubtree(d->shapes);
        Q_FOREACH(KoShape *shape, linearizedShapes) {
            // shape-unwinding transform
            QTransform t = gradientToUser * markerTransformInverted * shape->absoluteTransformation().inverted();

            // update the stroke
            KoShapeStrokeSP shapeStroke = shape->stroke() ?
                        qSharedPointerDynamicCast<KoShapeStroke>(shape->stroke()) :
                        KoShapeStrokeSP();

            if (shapeStroke) {
                shapeStroke = toQShared(new KoShapeStroke(*shapeStroke));

                QBrush brush(*g);
                brush.setTransform(t);
                shapeStroke->setLineBrush(brush);
                shapeStroke->setColor(Qt::transparent);
                shape->setStroke(shapeStroke);
            }

            // update the background
            if (shape->background()) {

                QSharedPointer<KoGradientBackground> bg(new KoGradientBackground(KoFlake::cloneGradient(g.data()), t));
                shape->setBackground(bg);
            }
        }
    }
}
