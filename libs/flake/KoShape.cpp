/* This file is part of the KDE project
   Copyright (C) 2006 C. Boemann Rasmussen <cbo@boemann.dk>
   Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
   Copyright (C) 2006-2010 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007-2009,2011 Jan Hambrecht <jaham@gmx.net>
   CopyRight (C) 2010 Boudewijn Rempt <boud@kogmbh.com>

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
   Boston, MA 02110-1301, USA.
*/

#include "KoShape.h"
#include "KoShape_p.h"
#include "KoShapeContainer.h"
#include "KoShapeLayer.h"
#include "KoShapeContainerModel.h"
#include "KoSelection.h"
#include "KoPointerEvent.h"
#include "KoInsets.h"
#include "KoShapeStrokeModel.h"
#include "KoShapeBackground.h"
#include "KoColorBackground.h"
#include "KoGradientBackground.h"
#include "KoPatternBackground.h"
#include "KoShapeManager.h"
#include "KoShapeUserData.h"
#include "KoShapeApplicationData.h"
#include "KoShapeSavingContext.h"
#include "KoShapeLoadingContext.h"
#include "KoViewConverter.h"
#include "KoShapeStroke.h"
#include "ShapeDeleter_p.h"
#include "KoShapeShadow.h"
#include "KoClipPath.h"
#include "KoPathShape.h"
#include "KoEventAction.h"
#include "KoEventActionRegistry.h"
#include "KoOdfWorkaround.h"
#include "KoFilterEffectStack.h"
#include <KoSnapData.h>
#include <KoElementReference.h>

#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include <KoUnit.h>
#include <KoOdfStylesReader.h>
#include <KoOdfGraphicStyles.h>
#include <KoOdfLoadingContext.h>

#include <QPainter>
#include <QVariant>
#include <QPainterPath>
#include <QList>
#include <QMap>
#include <QByteArray>
#include <kdebug.h>

#include <limits>
#include "KoOdfGradientBackground.h"

// KoShapePrivate

KoShapePrivate::KoShapePrivate(KoShape *shape)
    : q_ptr(shape),
      size(50, 50),
      parent(0),
      userData(0),
      appData(0),
      stroke(0),
      fill(0),
      shadow(0),
      clipPath(0),
      filterEffectStack(0),
      transparency(0.0),
      zIndex(0),
      runThrough(0),
      visible(true),
      printable(true),
      geometryProtected(false),
      keepAspect(false),
      selectable(true),
      detectCollision(false),
      protectContent(false),
      textRunAroundSide(KoShape::BiggestRunAroundSide),
      textRunAroundDistanceLeft(0.0),
      textRunAroundDistanceTop(0.0),
      textRunAroundDistanceRight(0.0),
      textRunAroundDistanceBottom(0.0),
      textRunAroundThreshold(0.0),
      textRunAroundContour(KoShape::ContourFull)
{
    connectors[KoConnectionPoint::TopConnectionPoint] = KoConnectionPoint::defaultConnectionPoint(KoConnectionPoint::TopConnectionPoint);
    connectors[KoConnectionPoint::RightConnectionPoint] = KoConnectionPoint::defaultConnectionPoint(KoConnectionPoint::RightConnectionPoint);
    connectors[KoConnectionPoint::BottomConnectionPoint] = KoConnectionPoint::defaultConnectionPoint(KoConnectionPoint::BottomConnectionPoint);
    connectors[KoConnectionPoint::LeftConnectionPoint] = KoConnectionPoint::defaultConnectionPoint(KoConnectionPoint::LeftConnectionPoint);
}

KoShapePrivate::~KoShapePrivate()
{
    Q_Q(KoShape);
    if (parent)
        parent->removeShape(q);
    foreach(KoShapeManager *manager, shapeManagers) {
        manager->remove(q);
        manager->removeAdditional(q);
    }
    delete userData;
    delete appData;
    if (stroke && !stroke->deref())
        delete stroke;
    if (shadow && !shadow->deref())
        delete shadow;
    if (fill && !fill->deref())
        delete fill;
    if (filterEffectStack && !filterEffectStack->deref())
        delete filterEffectStack;
    delete clipPath;
    qDeleteAll(eventActions);
}

void KoShapePrivate::shapeChanged(KoShape::ChangeType type)
{
    Q_Q(KoShape);
    if (parent)
        parent->model()->childChanged(q, type);
    q->shapeChanged(type);
    foreach(KoShape * shape, dependees)
        shape->shapeChanged(type, q);
}

void KoShapePrivate::updateStroke()
{
    Q_Q(KoShape);
    if (stroke == 0)
        return;
    KoInsets insets;
    stroke->strokeInsets(q, insets);
    QSizeF inner = q->size();
    // update left
    q->update(QRectF(-insets.left, -insets.top, insets.left,
                     inner.height() + insets.top + insets.bottom));
    // update top
    q->update(QRectF(-insets.left, -insets.top,
                     inner.width() + insets.left + insets.right, insets.top));
    // update right
    q->update(QRectF(inner.width(), -insets.top, insets.right,
                     inner.height() + insets.top + insets.bottom));
    // update bottom
    q->update(QRectF(-insets.left, inner.height(),
                     inner.width() + insets.left + insets.right, insets.bottom));
}

void KoShapePrivate::addShapeManager(KoShapeManager *manager)
{
    shapeManagers.insert(manager);
}

void KoShapePrivate::removeShapeManager(KoShapeManager *manager)
{
    shapeManagers.remove(manager);
}

void KoShapePrivate::convertFromShapeCoordinates(KoConnectionPoint &point, const QSizeF &shapeSize) const
{
    switch(point.alignment) {
        case KoConnectionPoint::AlignNone:
            point.position = KoFlake::toRelative(point.position, shapeSize);
            point.position.rx() = qBound<qreal>(0.0, point.position.x(), 1.0);
            point.position.ry() = qBound<qreal>(0.0, point.position.y(), 1.0);
            break;
        case KoConnectionPoint::AlignRight:
            point.position.rx() -= shapeSize.width();
        case KoConnectionPoint::AlignLeft:
            point.position.ry() = 0.5*shapeSize.height();
            break;
        case KoConnectionPoint::AlignBottom:
            point.position.ry() -= shapeSize.height();
        case KoConnectionPoint::AlignTop:
            point.position.rx() = 0.5*shapeSize.width();
            break;
        case KoConnectionPoint::AlignTopLeft:
            // nothing to do here
            break;
        case KoConnectionPoint::AlignTopRight:
            point.position.rx() -= shapeSize.width();
            break;
        case KoConnectionPoint::AlignBottomLeft:
            point.position.ry() -= shapeSize.height();
            break;
        case KoConnectionPoint::AlignBottomRight:
            point.position.rx() -= shapeSize.width();
            point.position.ry() -= shapeSize.height();
            break;
        case KoConnectionPoint::AlignCenter:
            point.position.rx() -= 0.5 * shapeSize.width();
            point.position.ry() -= 0.5 * shapeSize.height();
            break;
    }
}

void KoShapePrivate::convertToShapeCoordinates(KoConnectionPoint &point, const QSizeF &shapeSize) const
{
    switch(point.alignment) {
        case KoConnectionPoint::AlignNone:
            point.position = KoFlake::toAbsolute(point.position, shapeSize);
            break;
        case KoConnectionPoint::AlignRight:
            point.position.rx() += shapeSize.width();
        case KoConnectionPoint::AlignLeft:
            point.position.ry() = 0.5*shapeSize.height();
            break;
        case KoConnectionPoint::AlignBottom:
            point.position.ry() += shapeSize.height();
        case KoConnectionPoint::AlignTop:
            point.position.rx() = 0.5*shapeSize.width();
            break;
        case KoConnectionPoint::AlignTopLeft:
            // nothing to do here
            break;
        case KoConnectionPoint::AlignTopRight:
            point.position.rx() += shapeSize.width();
            break;
        case KoConnectionPoint::AlignBottomLeft:
            point.position.ry() += shapeSize.height();
            break;
        case KoConnectionPoint::AlignBottomRight:
            point.position.rx() += shapeSize.width();
            point.position.ry() += shapeSize.height();
            break;
        case KoConnectionPoint::AlignCenter:
            point.position.rx() += 0.5 * shapeSize.width();
            point.position.ry() += 0.5 * shapeSize.height();
            break;
    }
}

// static
QString KoShapePrivate::getStyleProperty(const char *property, KoShapeLoadingContext &context)
{
    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
    QString value;

    if (styleStack.hasProperty(KoXmlNS::draw, property)) {
        value = styleStack.property(KoXmlNS::draw, property);
    }

    return value;
}



// ======== KoShape
KoShape::KoShape()
    : d_ptr(new KoShapePrivate(this))
{
    notifyChanged();
}

KoShape::KoShape(KoShapePrivate &dd)
    : d_ptr(&dd)
{
}

KoShape::~KoShape()
{
    Q_D(KoShape);
    d->shapeChanged(Deleted);
    delete d_ptr;
}

void KoShape::scale(qreal sx, qreal sy)
{
    Q_D(KoShape);
    QPointF pos = position();
    QTransform scaleMatrix;
    scaleMatrix.translate(pos.x(), pos.y());
    scaleMatrix.scale(sx, sy);
    scaleMatrix.translate(-pos.x(), -pos.y());
    d->localMatrix = d->localMatrix * scaleMatrix;

    notifyChanged();
    d->shapeChanged(ScaleChanged);
}

void KoShape::rotate(qreal angle)
{
    Q_D(KoShape);
    QPointF center = d->localMatrix.map(QPointF(0.5 * size().width(), 0.5 * size().height()));
    QTransform rotateMatrix;
    rotateMatrix.translate(center.x(), center.y());
    rotateMatrix.rotate(angle);
    rotateMatrix.translate(-center.x(), -center.y());
    d->localMatrix = d->localMatrix * rotateMatrix;

    notifyChanged();
    d->shapeChanged(RotationChanged);
}

void KoShape::shear(qreal sx, qreal sy)
{
    Q_D(KoShape);
    QPointF pos = position();
    QTransform shearMatrix;
    shearMatrix.translate(pos.x(), pos.y());
    shearMatrix.shear(sx, sy);
    shearMatrix.translate(-pos.x(), -pos.y());
    d->localMatrix = d->localMatrix * shearMatrix;

    notifyChanged();
    d->shapeChanged(ShearChanged);
}

void KoShape::setSize(const QSizeF &newSize)
{
    Q_D(KoShape);
    QSizeF oldSize(size());

    // always set size, as d->size and size() may vary
    d->size = newSize;

    if (oldSize == newSize)
        return;

    notifyChanged();
    d->shapeChanged(SizeChanged);
}

void KoShape::setPosition(const QPointF &newPosition)
{
    Q_D(KoShape);
    QPointF currentPos = position();
    if (newPosition == currentPos)
        return;
    QTransform translateMatrix;
    translateMatrix.translate(newPosition.x() - currentPos.x(), newPosition.y() - currentPos.y());
    d->localMatrix = d->localMatrix * translateMatrix;

    notifyChanged();
    d->shapeChanged(PositionChanged);
}

bool KoShape::hitTest(const QPointF &position) const
{
    Q_D(const KoShape);
    if (d->parent && d->parent->isClipped(this) && !d->parent->hitTest(position))
        return false;

    QPointF point = absoluteTransformation(0).inverted().map(position);
    QRectF bb(QPointF(), size());
    if (d->stroke) {
        KoInsets insets;
        d->stroke->strokeInsets(this, insets);
        bb.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
    }
    if (bb.contains(point))
        return true;

    // if there is no shadow we can as well just leave
    if (! d->shadow)
        return false;

    // the shadow has an offset to the shape, so we simply
    // check if the position minus the shadow offset hits the shape
    point = absoluteTransformation(0).inverted().map(position - d->shadow->offset());

    return bb.contains(point);
}

QRectF KoShape::boundingRect() const
{
    Q_D(const KoShape);

    QTransform transform = absoluteTransformation(0);
    QRectF bb = outlineRect();
    if (d->stroke) {
        KoInsets insets;
        d->stroke->strokeInsets(this, insets);
        bb.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
    }
    bb = transform.mapRect(bb);
    if (d->shadow) {
        KoInsets insets;
        d->shadow->insets(insets);
        bb.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
    }
    if (d->filterEffectStack) {
        QRectF clipRect = d->filterEffectStack->clipRectForBoundingRect(outlineRect());
        bb |= transform.mapRect(clipRect);
    }

    return bb;
}

QTransform KoShape::absoluteTransformation(const KoViewConverter *converter) const
{
    Q_D(const KoShape);
    QTransform matrix;
    // apply parents matrix to inherit any transformations done there.
    KoShapeContainer * container = d->parent;
    if (container) {
        if (container->inheritsTransform(this)) {
            // We do need to pass the converter here, otherwise the parent's
            // translation is not inherited.
            matrix = container->absoluteTransformation(converter);
        } else {
            QSizeF containerSize = container->size();
            QPointF containerPos = container->absolutePosition() - QPointF(0.5 * containerSize.width(), 0.5 * containerSize.height());
            if (converter)
                containerPos = converter->documentToView(containerPos);
            matrix.translate(containerPos.x(), containerPos.y());
        }
    }

    if (converter) {
        QPointF pos = d->localMatrix.map(QPointF());
        QPointF trans = converter->documentToView(pos) - pos;
        matrix.translate(trans.x(), trans.y());
    }

    return d->localMatrix * matrix;
}

void KoShape::applyAbsoluteTransformation(const QTransform &matrix)
{
    QTransform globalMatrix = absoluteTransformation(0);
    // the transformation is relative to the global coordinate system
    // but we want to change the local matrix, so convert the matrix
    // to be relative to the local coordinate system
    QTransform transformMatrix = globalMatrix * matrix * globalMatrix.inverted();
    applyTransformation(transformMatrix);
}

void KoShape::applyTransformation(const QTransform &matrix)
{
    Q_D(KoShape);
    d->localMatrix = matrix * d->localMatrix;
    notifyChanged();
    d->shapeChanged(GenericMatrixChange);
}

void KoShape::setTransformation(const QTransform &matrix)
{
    Q_D(KoShape);
    d->localMatrix = matrix;
    notifyChanged();
    d->shapeChanged(GenericMatrixChange);
}

QTransform KoShape::transformation() const
{
    Q_D(const KoShape);
    return d->localMatrix;
}

KoShape::ChildZOrderPolicy KoShape::childZOrderPolicy()
{
    return ChildZDefault;
}

bool KoShape::compareShapeZIndex(KoShape *s1, KoShape *s2)
{
    // First sort according to runThrough which is sort of a master level
    KoShape *parentShapeS1 = s1->parent();
    KoShape *parentShapeS2 = s2->parent();
    int runThrough1 = s1->runThrough();
    int runThrough2 = s2->runThrough();
    while (parentShapeS1) {
        if (parentShapeS1->childZOrderPolicy() == KoShape::ChildZParentChild) {
            runThrough1 = parentShapeS1->runThrough();
        } else {
            runThrough1 = runThrough1 + parentShapeS1->runThrough();
        }
        parentShapeS1 = parentShapeS1->parent();
    }

    while (parentShapeS2) {
        if (parentShapeS2->childZOrderPolicy() == KoShape::ChildZParentChild) {
            runThrough2 = parentShapeS2->runThrough();
        } else {
            runThrough2 = runThrough2 + parentShapeS2->runThrough();
        }
        parentShapeS2 = parentShapeS2->parent();
    }

    if (runThrough1 > runThrough2) {
        return false;
    }
    if (runThrough1 < runThrough2) {
        return true;
    }

    // If on the same runThrough level then the zIndex is all that matters.
    //
    // We basically walk up through the parents until we find a common base parent
    // To do that we need two loops where the inner loop walks up through the parents
    // of s2 every time we step up one parent level on s1
    //
    // We don't update the index value until after we have seen that it's not a common base
    // That way we ensure that two children of a common base are sorted according to their respective
    // z value
    bool foundCommonParent = false;
    int index1 = s1->zIndex();
    int index2 = s2->zIndex();
    parentShapeS1 = s1;
    parentShapeS2 = s2;
    while (parentShapeS1 && !foundCommonParent) {
        parentShapeS2 = s2;
        index2 = parentShapeS2->zIndex();
        while (parentShapeS2) {
            if (parentShapeS2 == parentShapeS1) {
                foundCommonParent = true;
                break;
            }
            if (parentShapeS2->childZOrderPolicy() == KoShape::ChildZParentChild) {
                index2 = parentShapeS2->zIndex();
            }
            parentShapeS2 = parentShapeS2->parent();
        }

        if (!foundCommonParent) {
            if (parentShapeS1->childZOrderPolicy() == KoShape::ChildZParentChild) {
                index1 = parentShapeS1->zIndex();
            }
            parentShapeS1 = parentShapeS1->parent();
        }
    }

    // If the one shape is a parent/child of the other then sort so.
    if (s1 == parentShapeS2) {
        return true;
    }
    if (s2 == parentShapeS1) {
        return false;
    }

    // If we went that far then the z-Index is used for sorting.
    return index1 < index2;
}

void KoShape::setParent(KoShapeContainer *parent)
{
    Q_D(KoShape);
    if (d->parent == parent)
        return;
    KoShapeContainer *oldParent = d->parent;
    d->parent = 0; // avoids recursive removing
    if (oldParent)
        oldParent->removeShape(this);
    if (parent && parent != this) {
        d->parent = parent;
        parent->addShape(this);
    }
    notifyChanged();
    d->shapeChanged(ParentChanged);
}

int KoShape::zIndex() const
{
    Q_D(const KoShape);
    return d->zIndex;
}

void KoShape::update() const
{
    Q_D(const KoShape);

    if (!d->shapeManagers.empty()) {
        QRectF rect(boundingRect());
        foreach(KoShapeManager * manager, d->shapeManagers) {
            manager->update(rect, this, true);
        }
    }
}

void KoShape::update(const QRectF &rect) const
{

    if (rect.isEmpty() && !rect.isNull()) {
        return;
    }

    Q_D(const KoShape);

    if (!d->shapeManagers.empty() && isVisible()) {
        QRectF rc(absoluteTransformation(0).mapRect(rect));
        foreach(KoShapeManager * manager, d->shapeManagers) {
            manager->update(rc);
        }
    }
}

QPainterPath KoShape::outline() const
{
    QPainterPath path;
    path.addRect(outlineRect());
    return path;
}

QRectF KoShape::outlineRect() const
{
    const QSizeF s = size();
    return QRectF(QPointF(0, 0), QSizeF(qMax(s.width(),  qreal(0.0001)),
                                        qMax(s.height(), qreal(0.0001))));
}

QPainterPath KoShape::shadowOutline() const
{
    Q_D(const KoShape);
    if (d->fill) {
        return outline();
    }

    return QPainterPath();
}

QPointF KoShape::absolutePosition(KoFlake::Position anchor) const
{
    QPointF point;
    switch (anchor) {
    case KoFlake::TopLeftCorner: break;
    case KoFlake::TopRightCorner: point = QPointF(size().width(), 0.0); break;
    case KoFlake::BottomLeftCorner: point = QPointF(0.0, size().height()); break;
    case KoFlake::BottomRightCorner: point = QPointF(size().width(), size().height()); break;
    case KoFlake::CenteredPosition: point = QPointF(size().width() / 2.0, size().height() / 2.0); break;
    }
    return absoluteTransformation(0).map(point);
}

void KoShape::setAbsolutePosition(QPointF newPosition, KoFlake::Position anchor)
{
    Q_D(KoShape);
    QPointF currentAbsPosition = absolutePosition(anchor);
    QPointF translate = newPosition - currentAbsPosition;
    QTransform translateMatrix;
    translateMatrix.translate(translate.x(), translate.y());
    applyAbsoluteTransformation(translateMatrix);
    notifyChanged();
    d->shapeChanged(PositionChanged);
}

void KoShape::copySettings(const KoShape *shape)
{
    Q_D(KoShape);
    d->size = shape->size();
    d->connectors.clear();
    foreach(const KoConnectionPoint &point, shape->connectionPoints())
        addConnectionPoint(point);
    d->zIndex = shape->zIndex();
    d->visible = shape->isVisible();

    // Ensure printable is true by default
    if (!d->visible)
        d->printable = true;
    else
        d->printable = shape->isPrintable();

    d->geometryProtected = shape->isGeometryProtected();
    d->protectContent = shape->isContentProtected();
    d->selectable = shape->isSelectable();
    d->keepAspect = shape->keepAspectRatio();
    d->localMatrix = shape->d_ptr->localMatrix;
}

void KoShape::notifyChanged()
{
    Q_D(KoShape);
    foreach(KoShapeManager * manager, d->shapeManagers) {
        manager->notifyShapeChanged(this);
    }
}

void KoShape::setUserData(KoShapeUserData *userData)
{
    Q_D(KoShape);
    delete d->userData;
    d->userData = userData;
}

KoShapeUserData *KoShape::userData() const
{
    Q_D(const KoShape);
    return d->userData;
}

void KoShape::setApplicationData(KoShapeApplicationData *appData)
{
    Q_D(KoShape);
    // appdata is deleted by the application.
    d->appData = appData;
}

KoShapeApplicationData *KoShape::applicationData() const
{
    Q_D(const KoShape);
    return d->appData;
}

bool KoShape::hasTransparency() const
{
    Q_D(const KoShape);
    if (! d->fill)
        return true;
    else
        return d->fill->hasTransparency() || d->transparency > 0.0;
}

void KoShape::setTransparency(qreal transparency)
{
    Q_D(KoShape);
    d->transparency = qBound<qreal>(0.0, transparency, 1.0);
}

qreal KoShape::transparency(bool recursive) const
{
    Q_D(const KoShape);
    if (!recursive || !parent()) {
        return d->transparency;
    } else {
        const qreal parentOpacity = 1.0-parent()->transparency(recursive);
        const qreal childOpacity = 1.0-d->transparency;
        return 1.0-(parentOpacity*childOpacity);
    }
}

KoInsets KoShape::strokeInsets() const
{
    Q_D(const KoShape);
    KoInsets answer;
    if (d->stroke)
        d->stroke->strokeInsets(this, answer);
    return answer;
}

qreal KoShape::rotation() const
{
    Q_D(const KoShape);
    // try to extract the rotation angle out of the local matrix
    // if it is a pure rotation matrix

    // check if the matrix has shearing mixed in
    if (fabs(fabs(d->localMatrix.m12()) - fabs(d->localMatrix.m21())) > 1e-10)
        return std::numeric_limits<qreal>::quiet_NaN();
    // check if the matrix has scaling mixed in
    if (fabs(d->localMatrix.m11() - d->localMatrix.m22()) > 1e-10)
        return std::numeric_limits<qreal>::quiet_NaN();

    // calculate the angle from the matrix elements
    qreal angle = atan2(-d->localMatrix.m21(), d->localMatrix.m11()) * 180.0 / M_PI;
    if (angle < 0.0)
        angle += 360.0;

    return angle;
}

QSizeF KoShape::size() const
{
    Q_D(const KoShape);
    return d->size;
}

QPointF KoShape::position() const
{
    Q_D(const KoShape);
    QPointF center(0.5*size().width(), 0.5*size().height());
    return d->localMatrix.map(center) - center;
}

int KoShape::addConnectionPoint(const KoConnectionPoint &point)
{
    Q_D(KoShape);

    // get next glue point id
    int nextConnectionPointId = KoConnectionPoint::FirstCustomConnectionPoint;
    if (d->connectors.size())
        nextConnectionPointId = qMax(nextConnectionPointId, (--d->connectors.end()).key()+1);

    KoConnectionPoint p = point;
    d->convertFromShapeCoordinates(p, size());
    d->connectors[nextConnectionPointId] = p;

    return nextConnectionPointId;
}

bool KoShape::setConnectionPoint(int connectionPointId, const KoConnectionPoint &point)
{
    Q_D(KoShape);
    if (connectionPointId < 0)
        return false;

    const bool insertPoint = !hasConnectionPoint(connectionPointId);

    switch(connectionPointId) {
        case KoConnectionPoint::TopConnectionPoint:
        case KoConnectionPoint::RightConnectionPoint:
        case KoConnectionPoint::BottomConnectionPoint:
        case KoConnectionPoint::LeftConnectionPoint:
        {
            KoConnectionPoint::PointId id = static_cast<KoConnectionPoint::PointId>(connectionPointId);
            d->connectors[id] = KoConnectionPoint::defaultConnectionPoint(id);
            break;
        }
        default:
        {
            KoConnectionPoint p = point;
            d->convertFromShapeCoordinates(p, size());
            d->connectors[connectionPointId] = p;
            break;
        }
    }

    if(!insertPoint)
        d->shapeChanged(ConnectionPointChanged);

    return true;
}

bool KoShape::hasConnectionPoint(int connectionPointId) const
{
    Q_D(const KoShape);
    return d->connectors.contains(connectionPointId);
}

KoConnectionPoint KoShape::connectionPoint(int connectionPointId) const
{
    Q_D(const KoShape);
    KoConnectionPoint p = d->connectors.value(connectionPointId, KoConnectionPoint());
    // convert glue point to shape coordinates
    d->convertToShapeCoordinates(p, size());
    return p;
}

KoConnectionPoints KoShape::connectionPoints() const
{
    Q_D(const KoShape);
    QSizeF s = size();
    KoConnectionPoints points = d->connectors;
    KoConnectionPoints::iterator point = points.begin();
    KoConnectionPoints::iterator lastPoint = points.end();
    // convert glue points to shape coordinates
    for(; point != lastPoint; ++point) {
        d->convertToShapeCoordinates(point.value(), s);
    }

    return points;
}

void KoShape::removeConnectionPoint(int connectionPointId)
{
    Q_D(KoShape);
    d->connectors.remove(connectionPointId);
    d->shapeChanged(ConnectionPointChanged);
}

void KoShape::clearConnectionPoints()
{
    Q_D(KoShape);
    d->connectors.clear();
}

void KoShape::addEventAction(KoEventAction *action)
{
    Q_D(KoShape);
    d->eventActions.insert(action);
}

void KoShape::removeEventAction(KoEventAction *action)
{
    Q_D(KoShape);
    d->eventActions.remove(action);
}

QSet<KoEventAction *> KoShape::eventActions() const
{
    Q_D(const KoShape);
    return d->eventActions;
}

KoShape::TextRunAroundSide KoShape::textRunAroundSide() const
{
    Q_D(const KoShape);
    return d->textRunAroundSide;
}

void KoShape::setTextRunAroundSide(TextRunAroundSide side, RunThroughLevel runThrought)
{
    Q_D(KoShape);

    if (side == RunThrough) {
        if (runThrought == Background) {
            setRunThrough(-1);
        } else {
            setRunThrough(1);
        }
    } else {
        setRunThrough(0);
    }

    if ( d->textRunAroundSide == side) {
        return;
    }

    d->textRunAroundSide = side;
    notifyChanged();
    d->shapeChanged(TextRunAroundChanged);
}

qreal KoShape::textRunAroundDistanceTop() const
{
    Q_D(const KoShape);
    return d->textRunAroundDistanceTop;
}

void KoShape::setTextRunAroundDistanceTop(qreal distance)
{
    Q_D(KoShape);
    d->textRunAroundDistanceTop = distance;
}

qreal KoShape::textRunAroundDistanceLeft() const
{
    Q_D(const KoShape);
    return d->textRunAroundDistanceLeft;
}

void KoShape::setTextRunAroundDistanceLeft(qreal distance)
{
    Q_D(KoShape);
    d->textRunAroundDistanceLeft = distance;
}

qreal KoShape::textRunAroundDistanceRight() const
{
    Q_D(const KoShape);
    return d->textRunAroundDistanceRight;
}

void KoShape::setTextRunAroundDistanceRight(qreal distance)
{
    Q_D(KoShape);
    d->textRunAroundDistanceRight = distance;
}

qreal KoShape::textRunAroundDistanceBottom() const
{
    Q_D(const KoShape);
    return d->textRunAroundDistanceBottom;
}

void KoShape::setTextRunAroundDistanceBottom(qreal distance)
{
    Q_D(KoShape);
    d->textRunAroundDistanceBottom = distance;
}

qreal KoShape::textRunAroundThreshold() const
{
    Q_D(const KoShape);
    return d->textRunAroundThreshold;
}

void KoShape::setTextRunAroundThreshold(qreal threshold)
{
    Q_D(KoShape);
    d->textRunAroundThreshold = threshold;
}

KoShape::TextRunAroundContour KoShape::textRunAroundContour() const
{
    Q_D(const KoShape);
    return d->textRunAroundContour;
}

void KoShape::setTextRunAroundContour(KoShape::TextRunAroundContour contour)
{
    Q_D(KoShape);
    d->textRunAroundContour = contour;
}

void KoShape::setBackground(KoShapeBackground *fill)
{
    Q_D(KoShape);
    if (d->fill)
        d->fill->deref();
    d->fill = fill;
    if (d->fill)
        d->fill->ref();
    d->shapeChanged(BackgroundChanged);
    notifyChanged();
}

KoShapeBackground *KoShape::background() const
{
    Q_D(const KoShape);
    return d->fill;
}

void KoShape::setZIndex(int zIndex)
{
    Q_D(KoShape);
    if (d->zIndex == zIndex)
        return;
    d->zIndex = zIndex;
    notifyChanged();
}

int KoShape::runThrough()
{
    Q_D(const KoShape);
    return d->runThrough;
}

void KoShape::setRunThrough(short int runThrough)
{
    Q_D(KoShape);
    d->runThrough = runThrough;
}

void KoShape::setVisible(bool on)
{
    Q_D(KoShape);
    if (d->visible == on) return;
    d->visible = on;
}

bool KoShape::isVisible(bool recursive) const
{
    Q_D(const KoShape);
    if (! recursive)
        return d->visible;
    if (recursive && ! d->visible)
        return false;

    KoShapeContainer * parentShape = parent();
    while (parentShape) {
        if (! parentShape->isVisible())
            return false;
        parentShape = parentShape->parent();
    }
    return true;
}

void KoShape::setPrintable(bool on)
{
    Q_D(KoShape);
    d->printable = on;
}

bool KoShape::isPrintable() const
{
    Q_D(const KoShape);
    if (d->visible)
        return d->printable;
    else
        return false;
}

void KoShape::setSelectable(bool selectable)
{
    Q_D(KoShape);
    d->selectable = selectable;
}

bool KoShape::isSelectable() const
{
    Q_D(const KoShape);
    return d->selectable;
}

void KoShape::setGeometryProtected(bool on)
{
    Q_D(KoShape);
    d->geometryProtected = on;
}

bool KoShape::isGeometryProtected() const
{
    Q_D(const KoShape);
    return d->geometryProtected;
}

void KoShape::setContentProtected(bool protect)
{
    Q_D(KoShape);
    d->protectContent = protect;
}

bool KoShape::isContentProtected() const
{
    Q_D(const KoShape);
    return d->protectContent;
}

KoShapeContainer *KoShape::parent() const
{
    Q_D(const KoShape);
    return d->parent;
}

void KoShape::setKeepAspectRatio(bool keepAspect)
{
    Q_D(KoShape);
    d->keepAspect = keepAspect;
}

bool KoShape::keepAspectRatio() const
{
    Q_D(const KoShape);
    return d->keepAspect;
}

QString KoShape::shapeId() const
{
    Q_D(const KoShape);
    return d->shapeId;
}

void KoShape::setShapeId(const QString &id)
{
    Q_D(KoShape);
    d->shapeId = id;
}

void KoShape::setCollisionDetection(bool detect)
{
    Q_D(KoShape);
    d->detectCollision = detect;
}

bool KoShape::collisionDetection()
{
    Q_D(KoShape);
    return d->detectCollision;
}

KoShapeStrokeModel *KoShape::stroke() const
{
    Q_D(const KoShape);
    return d->stroke;
}

void KoShape::setStroke(KoShapeStrokeModel *stroke)
{
    Q_D(KoShape);
    if (stroke)
        stroke->ref();
    d->updateStroke();
    if (d->stroke)
        d->stroke->deref();
    d->stroke = stroke;
    d->updateStroke();
    d->shapeChanged(StrokeChanged);
    notifyChanged();
}

void KoShape::setShadow(KoShapeShadow *shadow)
{
    Q_D(KoShape);
    if (d->shadow)
        d->shadow->deref();
    d->shadow = shadow;
    if (d->shadow) {
        d->shadow->ref();
        // TODO update changed area
    }
    d->shapeChanged(ShadowChanged);
    notifyChanged();
}

KoShapeShadow *KoShape::shadow() const
{
    Q_D(const KoShape);
    return d->shadow;
}

void KoShape::setClipPath(KoClipPath *clipPath)
{
    Q_D(KoShape);
    d->clipPath = clipPath;
    d->shapeChanged(ClipPathChanged);
    notifyChanged();
}

KoClipPath * KoShape::clipPath() const
{
    Q_D(const KoShape);
    return d->clipPath;
}

QTransform KoShape::transform() const
{
    Q_D(const KoShape);
    return d->localMatrix;
}

QString KoShape::name() const
{
    Q_D(const KoShape);
    return d->name;
}

void KoShape::setName(const QString &name)
{
    Q_D(KoShape);
    d->name = name;
}

void KoShape::waitUntilReady(const KoViewConverter &converter, bool asynchronous) const
{
    Q_UNUSED(converter);
    Q_UNUSED(asynchronous);
}

void KoShape::deleteLater()
{
    Q_D(KoShape);
    foreach(KoShapeManager *manager, d->shapeManagers)
        manager->remove(this);
    d->shapeManagers.clear();
    new ShapeDeleter(this);
}

bool KoShape::isEditable() const
{
    Q_D(const KoShape);
    if (!d->visible || d->geometryProtected)
        return false;

    if (d->parent && d->parent->isChildLocked(this))
        return false;

    return true;
}

// loading & saving methods
QString KoShape::saveStyle(KoGenStyle &style, KoShapeSavingContext &context) const
{
    Q_D(const KoShape);
    // and fill the style
    KoShapeStrokeModel *b = stroke();
    if (b) {
        b->fillStyle(style, context);
    }
    else {
        style.addProperty("draw:stroke", "none", KoGenStyle::GraphicType);
    }
    KoShapeShadow *s = shadow();
    if (s)
        s->fillStyle(style, context);

    KoShapeBackground *bg = background();
    if (bg) {
        bg->fillStyle(style, context);
    }
    else {
        style.addProperty("draw:fill", "none", KoGenStyle::GraphicType);
    }

    if (context.isSet(KoShapeSavingContext::AutoStyleInStyleXml)) {
        style.setAutoStyleInStylesDotXml(true);
    }

    QString value;
    if (isGeometryProtected()) {
        value = "position size";
    }
    if (isContentProtected()) {
        if (! value.isEmpty())
            value += ' ';
        value += "content";
    }
    if (!value.isEmpty()) {
        style.addProperty("style:protect", value, KoGenStyle::GraphicType);
    }

    QMap<QByteArray, QString>::const_iterator it(d->additionalStyleAttributes.constBegin());
    for (; it != d->additionalStyleAttributes.constEnd(); ++it) {
        style.addProperty(it.key(), it.value());
    }

    if (parent() && parent()->isClipped(this)) {
        /*
         * In Calligra clipping is done using a parent shape which can be rotated, sheared etc
         * and even non-square.  So the ODF interoperability version we write here is really
         * just a very simple version of that...
         */
        qreal top = -position().y();
        qreal left = -position().x();
        qreal right = parent()->size().width() - size().width() - left;
        qreal bottom = parent()->size().height() - size().height() - top;

        style.addProperty("fo:clip", QString("rect(%1pt, %2pt, %3pt, %4pt)")
                          .arg(top, 10, 'f').arg(right, 10, 'f')
                          .arg(bottom, 10, 'f').arg(left, 10, 'f'), KoGenStyle::GraphicType);

    }

    QString wrap;
    switch (textRunAroundSide()) {
        case BiggestRunAroundSide:
            wrap = "biggest";
            break;
        case LeftRunAroundSide:
            wrap = "left";
            break;
        case RightRunAroundSide:
            wrap = "right";
            break;
        case EnoughRunAroundSide:
            wrap = "dynamic";
            break;
        case BothRunAroundSide:
            wrap = "parallel";
            break;
        case NoRunAround:
            wrap = "none";
            break;
        case RunThrough:
            wrap = "run-through";
            break;
    }
    style.addProperty("style:wrap", wrap, KoGenStyle::GraphicType);
    switch (textRunAroundContour()) {
        case ContourBox:
            style.addProperty("style:wrap-contour", "false", KoGenStyle::GraphicType);
            break;
        case ContourFull:
            style.addProperty("style:wrap-contour", "true", KoGenStyle::GraphicType);
            style.addProperty("style:wrap-contour-mode", "full", KoGenStyle::GraphicType);
            break;
        case ContourOutside:
            style.addProperty("style:wrap-contour", "true", KoGenStyle::GraphicType);
            style.addProperty("style:wrap-contour-mode", "outside", KoGenStyle::GraphicType);
            break;
    }
    style.addPropertyPt("style:wrap-dynamic-threshold", textRunAroundThreshold(), KoGenStyle::GraphicType);
    if ((textRunAroundDistanceLeft() == textRunAroundDistanceRight())
                && (textRunAroundDistanceTop() == textRunAroundDistanceBottom())
                && (textRunAroundDistanceLeft() == textRunAroundDistanceTop())) {
        style.addPropertyPt("fo:margin", textRunAroundDistanceLeft(), KoGenStyle::GraphicType);
    } else {
        style.addPropertyPt("fo:margin-left", textRunAroundDistanceLeft(), KoGenStyle::GraphicType);
        style.addPropertyPt("fo:margin-top", textRunAroundDistanceTop(), KoGenStyle::GraphicType);
        style.addPropertyPt("fo:margin-right", textRunAroundDistanceRight(), KoGenStyle::GraphicType);
        style.addPropertyPt("fo:margin-bottom", textRunAroundDistanceBottom(), KoGenStyle::GraphicType);
    }

    return context.mainStyles().insert(style, context.isSet(KoShapeSavingContext::PresentationShape) ? "pr" : "gr");
}

void KoShape::loadStyle(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_D(KoShape);

    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
    styleStack.setTypeProperties("graphic");

    if (d->fill && !d->fill->deref()) {
        delete d->fill;
        d->fill = 0;
    }
    if (d->stroke && !d->stroke->deref()) {
        delete d->stroke;
        d->stroke = 0;
    }
    if (d->shadow && !d->shadow->deref()) {
        delete d->shadow;
        d->shadow = 0;
    }
    setBackground(loadOdfFill(context));
    setStroke(loadOdfStroke(element, context));
    setShadow(d->loadOdfShadow(context));

    QString protect(styleStack.property(KoXmlNS::style, "protect"));
    setGeometryProtected(protect.contains("position") || protect.contains("size"));
    setContentProtected(protect.contains("content"));

    QString margin = styleStack.property(KoXmlNS::fo, "margin");
    if (!margin.isEmpty()) {
        setTextRunAroundDistanceLeft(KoUnit::parseValue(margin));
        setTextRunAroundDistanceTop(KoUnit::parseValue(margin));
        setTextRunAroundDistanceRight(KoUnit::parseValue(margin));
        setTextRunAroundDistanceBottom(KoUnit::parseValue(margin));
    }
    margin = styleStack.property(KoXmlNS::fo, "margin-left");
    if (!margin.isEmpty()) {
        setTextRunAroundDistanceLeft(KoUnit::parseValue(margin));
    }

    margin = styleStack.property(KoXmlNS::fo, "margin-top");
    if (!margin.isEmpty()) {
        setTextRunAroundDistanceTop(KoUnit::parseValue(margin));
    }
    margin = styleStack.property(KoXmlNS::fo, "margin-right");
    if (!margin.isEmpty()) {
        setTextRunAroundDistanceRight(KoUnit::parseValue(margin));
    }
    margin = styleStack.property(KoXmlNS::fo, "margin-bottom");
    if (!margin.isEmpty()) {
        setTextRunAroundDistanceBottom(KoUnit::parseValue(margin));
    }

    QString wrap;
    if (styleStack.hasProperty(KoXmlNS::style, "wrap")) {
        wrap = styleStack.property(KoXmlNS::style, "wrap");
    } else {
        // no value given in the file, but guess biggest
        wrap = "biggest";
    }
    if (wrap == "none") {
        setTextRunAroundSide(KoShape::NoRunAround);
    } else if (wrap == "run-through") {
        QString runTrought = styleStack.property(KoXmlNS::style, "run-through", "background");
        if (runTrought == "background") {
            setTextRunAroundSide(KoShape::RunThrough, KoShape::Background);
        } else {
            setTextRunAroundSide(KoShape::RunThrough, KoShape::Foreground);
        }
    } else {
        if (wrap == "biggest")
            setTextRunAroundSide(KoShape::BiggestRunAroundSide);
        else if (wrap == "left")
            setTextRunAroundSide(KoShape::LeftRunAroundSide);
        else if (wrap == "right")
            setTextRunAroundSide(KoShape::RightRunAroundSide);
        else if (wrap == "dynamic")
            setTextRunAroundSide(KoShape::EnoughRunAroundSide);
        else if (wrap == "parallel")
            setTextRunAroundSide(KoShape::BothRunAroundSide);
    }

    if (styleStack.hasProperty(KoXmlNS::style, "wrap-dynamic-threshold")) {
        QString wrapThreshold = styleStack.property(KoXmlNS::style, "wrap-dynamic-threshold");
        if (!wrapThreshold.isEmpty()) {
            setTextRunAroundThreshold(KoUnit::parseValue(wrapThreshold));
        }
    }
    if (styleStack.property(KoXmlNS::style, "wrap-contour", "false") == "true") {
        if (styleStack.property(KoXmlNS::style, "wrap-contour-mode", "full") == "full") {
            setTextRunAroundContour(KoShape::ContourFull);
        } else {
            setTextRunAroundContour(KoShape::ContourOutside);
        }
    } else {
        setTextRunAroundContour(KoShape::ContourBox);
    }
}

bool KoShape::loadOdfAttributes(const KoXmlElement &element, KoShapeLoadingContext &context, int attributes)
{
    Q_D(KoShape);
    if (attributes & OdfPosition) {
        QPointF pos(position());
        if (element.hasAttributeNS(KoXmlNS::svg, "x"))
            pos.setX(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "x", QString())));
        if (element.hasAttributeNS(KoXmlNS::svg, "y"))
            pos.setY(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "y", QString())));
        setPosition(pos);
    }

    if (attributes & OdfSize) {
        QSizeF s(size());
        if (element.hasAttributeNS(KoXmlNS::svg, "width"))
            s.setWidth(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "width", QString())));
        if (element.hasAttributeNS(KoXmlNS::svg, "height"))
            s.setHeight(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "height", QString())));
        setSize(s);
    }

    if (attributes & OdfLayer) {
        if (element.hasAttributeNS(KoXmlNS::draw, "layer")) {
            KoShapeLayer *layer = context.layer(element.attributeNS(KoXmlNS::draw, "layer"));
            if (layer) {
                setParent(layer);
            }
        }
    }

    if (attributes & OdfId) {
        KoElementReference ref;
        ref.loadOdf(element);
        if (ref.isValid()) {
            context.addShapeId(this, ref.toString());
        }
    }

    if (attributes & OdfZIndex) {
        if (element.hasAttributeNS(KoXmlNS::draw, "z-index")) {
            setZIndex(element.attributeNS(KoXmlNS::draw, "z-index").toInt());
        } else {
            setZIndex(context.zIndex());
        }
    }

    if (attributes & OdfName) {
        if (element.hasAttributeNS(KoXmlNS::draw, "name")) {
            setName(element.attributeNS(KoXmlNS::draw, "name"));
        }
    }

    if (attributes & OdfStyle) {
        KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
        styleStack.save();
        if (element.hasAttributeNS(KoXmlNS::draw, "style-name")) {
            context.odfLoadingContext().fillStyleStack(element, KoXmlNS::draw, "style-name", "graphic");
        }
        if (element.hasAttributeNS(KoXmlNS::presentation, "style-name")) {
            context.odfLoadingContext().fillStyleStack(element, KoXmlNS::presentation, "style-name", "presentation");
        }
        loadStyle(element, context);

        styleStack.restore();
    }

    if (attributes & OdfTransformation) {
        QString transform = element.attributeNS(KoXmlNS::draw, "transform", QString());
        if (! transform.isEmpty())
            applyAbsoluteTransformation(parseOdfTransform(transform));
    }

    if (attributes & OdfAdditionalAttributes) {
        QSet<KoShapeLoadingContext::AdditionalAttributeData> additionalAttributeData = KoShapeLoadingContext::additionalAttributeData();
        foreach(const KoShapeLoadingContext::AdditionalAttributeData &attributeData, additionalAttributeData) {
            if (element.hasAttributeNS(attributeData.ns, attributeData.tag)) {
                QString value = element.attributeNS(attributeData.ns, attributeData.tag);
                //kDebug(30006) << "load additional attribute" << attributeData.tag << value;
                setAdditionalAttribute(attributeData.name, value);
            }
        }
    }

    if (attributes & OdfCommonChildElements) {
        const KoXmlElement eventActionsElement(KoXml::namedItemNS(element, KoXmlNS::office, "event-listeners"));
        if (!eventActionsElement.isNull()) {
            d->eventActions = KoEventActionRegistry::instance()->createEventActionsFromOdf(eventActionsElement, context);
        }
        // load glue points (connection points)
        loadOdfGluePoints(element, context);
    }

    return true;
}

KoShapeBackground *KoShape::loadOdfFill(KoShapeLoadingContext &context) const
{
    QString fill = KoShapePrivate::getStyleProperty("fill", context);
    KoShapeBackground *bg = 0;
    if (fill == "solid" || fill == "hatch") {
        bg = new KoColorBackground();
    } else if (fill == "gradient") {
        QString styleName = KoShapePrivate::getStyleProperty("fill-gradient-name", context);
        KoXmlElement *e = context.odfLoadingContext().stylesReader().drawStyles("gradient")[styleName];
        QString style;
        if (e) {
            style = e->attributeNS(KoXmlNS::draw, "style", QString());
        }
        if ((style == "rectangular") || (style == "square")) {
            bg = new KoOdfGradientBackground();
        } else {
            QGradient *gradient = new QLinearGradient();
            gradient->setCoordinateMode(QGradient::ObjectBoundingMode);
            bg = new KoGradientBackground(gradient);
        }
    } else if (fill == "bitmap") {
        bg = new KoPatternBackground(context.imageCollection());
#ifndef NWORKAROUND_ODF_BUGS
    } else if (fill.isEmpty()) {
        bg = KoOdfWorkaround::fixBackgroundColor(this, context);
        return bg;
#endif
    } else {
        return 0;
    }

    if (!bg->loadStyle(context.odfLoadingContext(), size())) {
        delete bg;
        return 0;
    }

    return bg;
}

KoShapeStrokeModel *KoShape::loadOdfStroke(const KoXmlElement &element, KoShapeLoadingContext &context) const
{
    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
    KoOdfStylesReader &stylesReader = context.odfLoadingContext().stylesReader();

    QString stroke = KoShapePrivate::getStyleProperty("stroke", context);
    if (stroke == "solid" || stroke == "dash") {
        QPen pen = KoOdfGraphicStyles::loadOdfStrokeStyle(styleStack, stroke, stylesReader);

        KoShapeStroke *stroke = new KoShapeStroke();

        if (styleStack.hasProperty(KoXmlNS::calligra, "stroke-gradient")) {
            QString gradientName = styleStack.property(KoXmlNS::calligra, "stroke-gradient");
            QBrush brush = KoOdfGraphicStyles::loadOdfGradientStyleByName(stylesReader, gradientName, size());
            stroke->setLineBrush(brush);
        } else {
            stroke->setColor(pen.color());
        }

#ifndef NWORKAROUND_ODF_BUGS
        KoOdfWorkaround::fixPenWidth(pen, context);
#endif
        stroke->setLineWidth(pen.widthF());
        stroke->setJoinStyle(pen.joinStyle());
        stroke->setLineStyle(pen.style(), pen.dashPattern());
        stroke->setCapStyle(pen.capStyle());

        return stroke;
#ifndef NWORKAROUND_ODF_BUGS
    } else if (stroke.isEmpty()) {
        QPen pen = KoOdfGraphicStyles::loadOdfStrokeStyle(styleStack, "solid", stylesReader);
        if (KoOdfWorkaround::fixMissingStroke(pen, element, context, this)) {
            KoShapeStroke *stroke = new KoShapeStroke();

#ifndef NWORKAROUND_ODF_BUGS
            KoOdfWorkaround::fixPenWidth(pen, context);
#endif
            stroke->setLineWidth(pen.widthF());
            stroke->setJoinStyle(pen.joinStyle());
            stroke->setLineStyle(pen.style(), pen.dashPattern());
            stroke->setCapStyle(pen.capStyle());
            stroke->setColor(pen.color());

            return stroke;
        }
#endif
    }

    return 0;
}

KoShapeShadow *KoShapePrivate::loadOdfShadow(KoShapeLoadingContext &context) const
{
    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
    QString shadowStyle = KoShapePrivate::getStyleProperty("shadow", context);
    if (shadowStyle == "visible" || shadowStyle == "hidden") {
        KoShapeShadow *shadow = new KoShapeShadow();
        QColor shadowColor(styleStack.property(KoXmlNS::draw, "shadow-color"));
        qreal offsetX = KoUnit::parseValue(styleStack.property(KoXmlNS::draw, "shadow-offset-x"));
        qreal offsetY = KoUnit::parseValue(styleStack.property(KoXmlNS::draw, "shadow-offset-y"));
        shadow->setOffset(QPointF(offsetX, offsetY));
        qreal blur = KoUnit::parseValue(styleStack.property(KoXmlNS::calligra, "shadow-blur-radius"));
        shadow->setBlur(blur);

        QString opacity = styleStack.property(KoXmlNS::draw, "shadow-opacity");
        if (! opacity.isEmpty() && opacity.right(1) == "%")
            shadowColor.setAlphaF(opacity.left(opacity.length() - 1).toFloat() / 100.0);
        shadow->setColor(shadowColor);
        shadow->setVisible(shadowStyle == "visible");

        return shadow;
    }
    return 0;
}

void KoShape::loadOdfGluePoints(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_D(KoShape);

    KoXmlElement child;
    forEachElement(child, element) {
        if (child.namespaceURI() != KoXmlNS::draw)
            continue;
        if (child.localName() != "glue-point")
            continue;

        // NOTE: this uses draw:id, but apparently while ODF 1.2 has deprecated
        // all use of draw:id for xml:id, it didn't specify that here, so it
        // doesn't support xml:id (and so, maybe, shouldn't use KoElementReference.
        const QString id = child.attributeNS(KoXmlNS::draw, "id", QString());
        const int index = id.toInt();
        if(id.isEmpty() || index < 4 || d->connectors.contains(index)) {
            kWarning(30006) << "glue-point with no or invalid id";
            continue;
        }
        QString xStr = child.attributeNS(KoXmlNS::svg, "x", QString()).simplified();
        QString yStr = child.attributeNS(KoXmlNS::svg, "y", QString()).simplified();
        if(xStr.isEmpty() || yStr.isEmpty()) {
            kWarning(30006) << "glue-point with invald position";
            continue;
        }

        KoConnectionPoint connector;

        const QString align = child.attributeNS(KoXmlNS::draw, "align", QString());
        if (align.isEmpty()) {
#ifndef NWORKAROUND_ODF_BUGS
            KoOdfWorkaround::fixGluePointPosition(xStr, context);
            KoOdfWorkaround::fixGluePointPosition(yStr, context);
#endif
            if(!xStr.endsWith('%') || !yStr.endsWith('%')) {
                kWarning(30006) << "glue-point with invald position";
                continue;
            }
            // x and y are relative to drawing object center
            connector.position.setX(xStr.remove('%').toDouble()/100.0);
            connector.position.setY(yStr.remove('%').toDouble()/100.0);
            // convert position to be relative to top-left corner
            connector.position += QPointF(0.5, 0.5);
            connector.position.rx() = qBound<qreal>(0.0, connector.position.x(), 1.0);
            connector.position.ry() = qBound<qreal>(0.0, connector.position.y(), 1.0);
        } else {
            // absolute distances to the edge specified by align
            connector.position.setX(KoUnit::parseValue(xStr));
            connector.position.setY(KoUnit::parseValue(yStr));
            if (align == "top-left") {
                connector.alignment = KoConnectionPoint::AlignTopLeft;
            } else if (align == "top") {
                connector.alignment = KoConnectionPoint::AlignTop;
            } else if (align == "top-right") {
                connector.alignment = KoConnectionPoint::AlignTopRight;
            } else if (align == "left") {
                connector.alignment = KoConnectionPoint::AlignLeft;
            } else if (align == "center") {
                connector.alignment = KoConnectionPoint::AlignCenter;
            } else if (align == "right") {
                connector.alignment = KoConnectionPoint::AlignRight;
            } else if (align == "bottom-left") {
                connector.alignment = KoConnectionPoint::AlignBottomLeft;
            } else if (align == "bottom") {
                connector.alignment = KoConnectionPoint::AlignBottom;
            } else if (align == "bottom-right") {
                connector.alignment = KoConnectionPoint::AlignBottomRight;
            }
            kDebug(30006) << "using alignment" << align;
        }
        const QString escape = child.attributeNS(KoXmlNS::draw, "escape-direction", QString());
        if (!escape.isEmpty()) {
            if (escape == "horizontal") {
                connector.escapeDirection = KoConnectionPoint::HorizontalDirections;
            } else if (escape == "vertical") {
                connector.escapeDirection = KoConnectionPoint::VerticalDirections;
            } else if (escape == "left") {
                connector.escapeDirection = KoConnectionPoint::LeftDirection;
            } else if (escape == "right") {
                connector.escapeDirection = KoConnectionPoint::RightDirection;
            } else if (escape == "up") {
                connector.escapeDirection = KoConnectionPoint::UpDirection;
            } else if (escape == "down") {
                connector.escapeDirection = KoConnectionPoint::DownDirection;
            }
            kDebug(30006) << "using escape direction" << escape;
        }
        d->connectors[index] = connector;
        kDebug(30006) << "loaded glue-point" << index << "at position" << connector.position;
    }
    kDebug(30006) << "shape has now" << d->connectors.count() << "glue-points";
}

void KoShape::loadOdfClipContour(const KoXmlElement &element, KoShapeLoadingContext &context, const QSizeF &scaleFactor)
{
    Q_D(KoShape);

    KoXmlElement child;
    forEachElement(child, element) {
        if (child.namespaceURI() != KoXmlNS::draw)
            continue;
        if (child.localName() != "contour-polygon")
            continue;

        kDebug(30006) << "shape loads contour-polygon";
        KoPathShape *ps = new KoPathShape();
        ps->loadContourOdf(child, context, scaleFactor);
        ps->setTransformation(transformation());

        KoClipData *cd = new KoClipData(ps);
        KoClipPath *clipPath = new KoClipPath(this, cd);
        d->clipPath = clipPath;
    }
}

QTransform KoShape::parseOdfTransform(const QString &transform)
{
    QTransform matrix;

    // Split string for handling 1 transform statement at a time
    QStringList subtransforms = transform.split(')', QString::SkipEmptyParts);
    QStringList::ConstIterator it = subtransforms.constBegin();
    QStringList::ConstIterator end = subtransforms.constEnd();
    for (; it != end; ++it) {
        QStringList subtransform = (*it).split('(', QString::SkipEmptyParts);

        subtransform[0] = subtransform[0].trimmed().toLower();
        subtransform[1] = subtransform[1].simplified();
        QRegExp reg("[,( ]");
        QStringList params = subtransform[1].split(reg, QString::SkipEmptyParts);

        if (subtransform[0].startsWith(';') || subtransform[0].startsWith(','))
            subtransform[0] = subtransform[0].right(subtransform[0].length() - 1);

        QString cmd = subtransform[0].toLower();

        if (cmd == "rotate") {
            QTransform rotMatrix;
            if (params.count() == 3) {
                qreal x = KoUnit::parseValue(params[1]);
                qreal y = KoUnit::parseValue(params[2]);

                rotMatrix.translate(x, y);
                // oo2 rotates by radians
                rotMatrix.rotate(-params[0].toDouble()*180.0 / M_PI);
                rotMatrix.translate(-x, -y);
            } else {
                // oo2 rotates by radians
                rotMatrix.rotate(-params[0].toDouble()*180.0 / M_PI);
            }
            matrix = matrix * rotMatrix;
        } else if (cmd == "translate") {
            QTransform moveMatrix;
            if (params.count() == 2) {
                qreal x = KoUnit::parseValue(params[0]);
                qreal y = KoUnit::parseValue(params[1]);
                moveMatrix.translate(x, y);
            } else   // Spec : if only one param given, assume 2nd param to be 0
                moveMatrix.translate(KoUnit::parseValue(params[0]) , 0);
            matrix = matrix * moveMatrix;
        } else if (cmd == "scale") {
            QTransform scaleMatrix;
            if (params.count() == 2)
                scaleMatrix.scale(params[0].toDouble(), params[1].toDouble());
            else    // Spec : if only one param given, assume uniform scaling
                scaleMatrix.scale(params[0].toDouble(), params[0].toDouble());
            matrix = matrix * scaleMatrix;
        } else if (cmd == "skewx") {
            QPointF p = absolutePosition(KoFlake::TopLeftCorner);
            QTransform shearMatrix;
            shearMatrix.translate(p.x(), p.y());
            shearMatrix.shear(tan(-params[0].toDouble()), 0.0F);
            shearMatrix.translate(-p.x(), -p.y());
            matrix = matrix * shearMatrix;
        } else if (cmd == "skewy") {
            QPointF p = absolutePosition(KoFlake::TopLeftCorner);
            QTransform shearMatrix;
            shearMatrix.translate(p.x(), p.y());
            shearMatrix.shear(0.0F, tan(-params[0].toDouble()));
            shearMatrix.translate(-p.x(), -p.y());
            matrix = matrix * shearMatrix;
        } else if (cmd == "matrix") {
            QTransform m;
            if (params.count() >= 6) {
                m.setMatrix(params[0].toDouble(), params[1].toDouble(), 0,
                            params[2].toDouble(), params[3].toDouble(), 0,
                            KoUnit::parseValue(params[4]), KoUnit::parseValue(params[5]), 1);
            }
            matrix = matrix * m;
        }
    }

    return matrix;
}

void KoShape::saveOdfAttributes(KoShapeSavingContext &context, int attributes) const
{
    Q_D(const KoShape);
    if (attributes & OdfStyle) {
        KoGenStyle style;
        // all items that should be written to 'draw:frame' and any other 'draw:' object that inherits this shape
        if (context.isSet(KoShapeSavingContext::PresentationShape)) {
            style = KoGenStyle(KoGenStyle::PresentationAutoStyle, "presentation");
            context.xmlWriter().addAttribute("presentation:style-name", saveStyle(style, context));
        } else {
            style = KoGenStyle(KoGenStyle::GraphicAutoStyle, "graphic");
            context.xmlWriter().addAttribute("draw:style-name", saveStyle(style, context));
        }
    }

    if (attributes & OdfId)  {
        if (context.isSet(KoShapeSavingContext::DrawId)) {
            KoElementReference ref = context.xmlid(this, "shape", KoElementReference::Counter);
            ref.saveOdf(&context.xmlWriter(), KoElementReference::DrawId);
        }
    }

    if (attributes & OdfName) {
        if (! name().isEmpty())
            context.xmlWriter().addAttribute("draw:name", name());
    }

    if (attributes & OdfLayer) {
        KoShape *parent = d->parent;
        while (parent) {
            if (dynamic_cast<KoShapeLayer*>(parent)) {
                context.xmlWriter().addAttribute("draw:layer", parent->name());
                break;
            }
            parent = parent->parent();
        }
    }

    if (attributes & OdfZIndex && context.isSet(KoShapeSavingContext::ZIndex)) {
        context.xmlWriter().addAttribute("draw:z-index", zIndex());
    }

    if (attributes & OdfSize) {
        QSizeF s(size());
        if (parent() && parent()->isClipped(this)) { // being clipped shrinks our visible size
            // clipping in ODF is done using a combination of visual size and content cliprect.
            // A picture of 10cm x 10cm displayed in a box of 2cm x 4cm will be scaled (out
            // of proportion in this case).  If we then add a fo:clip like;
            // fo:clip="rect(2cm, 3cm, 4cm, 5cm)" (top, right, bottom, left)
            // our original 10x10 is clipped to 2cm x 4cm  and *then* fitted in that box.

            // TODO do this properly by subtracting rects
            s = parent()->size();
        }
        context.xmlWriter().addAttributePt("svg:width", s.width());
        context.xmlWriter().addAttributePt("svg:height", s.height());
    }

    // The position is implicitly stored in the transformation matrix
    // if the transformation is saved as well
    if ((attributes & OdfPosition) && !(attributes & OdfTransformation)) {
        const QPointF p(position() * context.shapeOffset(this));
        context.xmlWriter().addAttributePt("svg:x", p.x());
        context.xmlWriter().addAttributePt("svg:y", p.y());
    }

    if (attributes & OdfTransformation) {
        QTransform matrix = absoluteTransformation(0) * context.shapeOffset(this);
        if (! matrix.isIdentity()) {
            if (qAbs(matrix.m11() - 1) < 1E-5           // 1
                    && qAbs(matrix.m12()) < 1E-5        // 0
                    && qAbs(matrix.m21()) < 1E-5        // 0
                    && qAbs(matrix.m22() - 1) < 1E-5) { // 1
                context.xmlWriter().addAttributePt("svg:x", matrix.dx());
                context.xmlWriter().addAttributePt("svg:y", matrix.dy());
            } else {
                QString m = QString("matrix(%1 %2 %3 %4 %5pt %6pt)")
                            .arg(matrix.m11(), 0, 'f', 11)
                            .arg(matrix.m12(), 0, 'f', 11)
                            .arg(matrix.m21(), 0, 'f', 11)
                            .arg(matrix.m22(), 0, 'f', 11)
                            .arg(matrix.dx(), 0, 'f', 11)
                            .arg(matrix.dy(), 0, 'f', 11);
                context.xmlWriter().addAttribute("draw:transform", m);
            }
        }
    }

    if (attributes & OdfViewbox) {
        const QSizeF s(size());
        QString viewBox = QString("0 0 %1 %2").arg(qRound(s.width())).arg(qRound(s.height()));
        context.xmlWriter().addAttribute("svg:viewBox", viewBox);
    }

    if (attributes & OdfAdditionalAttributes) {
        QMap<QString, QString>::const_iterator it(d->additionalAttributes.constBegin());
        for (; it != d->additionalAttributes.constEnd(); ++it) {
            context.xmlWriter().addAttribute(it.key().toUtf8(), it.value());
        }
    }
}

void KoShape::saveOdfCommonChildElements(KoShapeSavingContext &context) const
{
    Q_D(const KoShape);
    // save event listeners see ODF 9.2.21 Event Listeners
    if (d->eventActions.size() > 0) {
        context.xmlWriter().startElement("office:event-listeners");
        foreach(KoEventAction * action, d->eventActions) {
            action->saveOdf(context);
        }
        context.xmlWriter().endElement();
    }

    // save glue points see ODF 9.2.19 Glue Points
    if(d->connectors.count()) {
        KoConnectionPoints::const_iterator cp = d->connectors.constBegin();
        KoConnectionPoints::const_iterator lastCp = d->connectors.constEnd();
        for(; cp != lastCp; ++cp) {
            // do not save default glue points
            if(cp.key() < 4)
                continue;
            context.xmlWriter().startElement("draw:glue-point");
            context.xmlWriter().addAttribute("draw:id", QString("%1").arg(cp.key()));
            if (cp.value().alignment == KoConnectionPoint::AlignNone) {
                // convert to percent from center
                const qreal x = cp.value().position.x() * 100.0 - 50.0;
                const qreal y = cp.value().position.y() * 100.0 - 50.0;
                context.xmlWriter().addAttribute("svg:x", QString("%1%").arg(x));
                context.xmlWriter().addAttribute("svg:y", QString("%1%").arg(y));
            } else {
                context.xmlWriter().addAttributePt("svg:x", cp.value().position.x());
                context.xmlWriter().addAttributePt("svg:y", cp.value().position.y());
            }
            QString escapeDirection;
            switch(cp.value().escapeDirection) {
                case KoConnectionPoint::HorizontalDirections:
                    escapeDirection = "horizontal";
                    break;
                case KoConnectionPoint::VerticalDirections:
                    escapeDirection = "vertical";
                    break;
                case KoConnectionPoint::LeftDirection:
                    escapeDirection = "left";
                    break;
                case KoConnectionPoint::RightDirection:
                    escapeDirection = "right";
                    break;
                case KoConnectionPoint::UpDirection:
                    escapeDirection = "up";
                    break;
                case KoConnectionPoint::DownDirection:
                    escapeDirection = "down";
                    break;
                default:
                    // fall through
                    break;
            }
            if(!escapeDirection.isEmpty()) {
                context.xmlWriter().addAttribute("draw:escape-direction", escapeDirection);
            }
            QString alignment;
            switch(cp.value().alignment) {
                case KoConnectionPoint::AlignTopLeft:
                    alignment = "top-left";
                    break;
                case KoConnectionPoint::AlignTop:
                    alignment = "top";
                    break;
                case KoConnectionPoint::AlignTopRight:
                    alignment = "top-right";
                    break;
                case KoConnectionPoint::AlignLeft:
                    alignment = "left";
                    break;
                case KoConnectionPoint::AlignCenter:
                    alignment = "center";
                    break;
                case KoConnectionPoint::AlignRight:
                    alignment = "right";
                    break;
                case KoConnectionPoint::AlignBottomLeft:
                    alignment = "bottom-left";
                    break;
                case KoConnectionPoint::AlignBottom:
                    alignment = "bottom";
                    break;
                case KoConnectionPoint::AlignBottomRight:
                    alignment = "bottom-right";
                    break;
                default:
                    // fall through
                    break;
            }
            if(!alignment.isEmpty()) {
                context.xmlWriter().addAttribute("draw:align", alignment);
            }
            context.xmlWriter().endElement();
        }
    }
}

void KoShape::saveOdfClipContour(KoShapeSavingContext &context, const QSizeF &originalSize) const
{
    Q_D(const KoShape);

    kDebug(30006) << "shape saves contour-polygon";
    if (d->clipPath && !d->clipPath->clipPathShapes().isEmpty()) {
        // This will loose data as odf can only save one set of contour wheras
        // svg loading and at least karbon editing can produce more than one
        // TODO, FIXME see if we can save more than one clipshape to odf
        d->clipPath->clipPathShapes().first()->saveContourOdf(context, originalSize);
    }
}

// end loading & saving methods

// static
void KoShape::applyConversion(QPainter &painter, const KoViewConverter &converter)
{
    qreal zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);
    painter.scale(zoomX, zoomY);
}

QPointF KoShape::shapeToDocument(const QPointF &point) const
{
    return absoluteTransformation(0).map(point);
}

QRectF KoShape::shapeToDocument(const QRectF &rect) const
{
    return absoluteTransformation(0).mapRect(rect);
}

QPointF KoShape::documentToShape(const QPointF &point) const
{
    return absoluteTransformation(0).inverted().map(point);
}

QRectF KoShape::documentToShape(const QRectF &rect) const
{
    return absoluteTransformation(0).inverted().mapRect(rect);
}

bool KoShape::addDependee(KoShape *shape)
{
    Q_D(KoShape);
    if (! shape)
        return false;

    // refuse to establish a circular dependency
    if (shape->hasDependee(this))
        return false;

    if (! d->dependees.contains(shape))
        d->dependees.append(shape);

    return true;
}

void KoShape::removeDependee(KoShape *shape)
{
    Q_D(KoShape);
    int index = d->dependees.indexOf(shape);
    if (index >= 0)
        d->dependees.removeAt(index);
}

bool KoShape::hasDependee(KoShape *shape) const
{
    Q_D(const KoShape);
    return d->dependees.contains(shape);
}

QList<KoShape*> KoShape::dependees() const
{
    Q_D(const KoShape);
    return d->dependees;
}

void KoShape::shapeChanged(ChangeType type, KoShape *shape)
{
    Q_UNUSED(type);
    Q_UNUSED(shape);
}

KoSnapData KoShape::snapData() const
{
    return KoSnapData();
}

void KoShape::setAdditionalAttribute(const QString &name, const QString &value)
{
    Q_D(KoShape);
    d->additionalAttributes.insert(name, value);
}

void KoShape::removeAdditionalAttribute(const QString &name)
{
    Q_D(KoShape);
    d->additionalAttributes.remove(name);
}

bool KoShape::hasAdditionalAttribute(const QString &name) const
{
    Q_D(const KoShape);
    return d->additionalAttributes.contains(name);
}

QString KoShape::additionalAttribute(const QString &name) const
{
    Q_D(const KoShape);
    return d->additionalAttributes.value(name);
}

void KoShape::setAdditionalStyleAttribute(const char *name, const QString &value)
{
    Q_D(KoShape);
    d->additionalStyleAttributes.insert(name, value);
}

void KoShape::removeAdditionalStyleAttribute(const char *name)
{
    Q_D(KoShape);
    d->additionalStyleAttributes.remove(name);
}

KoFilterEffectStack *KoShape::filterEffectStack() const
{
    Q_D(const KoShape);
    return d->filterEffectStack;
}

void KoShape::setFilterEffectStack(KoFilterEffectStack *filterEffectStack)
{
    Q_D(KoShape);
    if (d->filterEffectStack)
        d->filterEffectStack->deref();
    d->filterEffectStack = filterEffectStack;
    if (d->filterEffectStack) {
        d->filterEffectStack->ref();
    }
    notifyChanged();
}

QSet<KoShape*> KoShape::toolDelegates() const
{
    Q_D(const KoShape);
    return d->toolDelegates;
}

void KoShape::setToolDelegates(const QSet<KoShape*> &delegates)
{
    Q_D(KoShape);
    d->toolDelegates = delegates;
}

QString KoShape::hyperLink () const
{
    Q_D(const KoShape);
    return d->hyperLink;
}

void KoShape::setHyperLink (QString & hyperLink)
{
    Q_D(KoShape);
    d->hyperLink = hyperLink;
}

KoShapePrivate *KoShape::priv()
{
    Q_D(KoShape);
    return d;
}
