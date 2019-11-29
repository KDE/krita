/* This file is part of the KDE project
   Copyright (C) 2006 C. Boemann Rasmussen <cbo@boemann.dk>
   Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
   Copyright (C) 2006-2010 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007-2009,2011 Jan Hambrecht <jaham@gmx.net>
   CopyRight (C) 2010 Boudewijn Rempt <boud@valdyas.org>

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

#include <limits>

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
#include "KoHatchBackground.h"
#include "KoGradientBackground.h"
#include "KoPatternBackground.h"
#include "KoShapeManager.h"
#include "KoShapeUserData.h"
#include "KoShapeApplicationData.h"
#include "KoShapeSavingContext.h"
#include "KoShapeLoadingContext.h"
#include "KoViewConverter.h"
#include "KoShapeStroke.h"
#include "KoShapeShadow.h"
#include "KoClipPath.h"
#include "KoPathShape.h"
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
#include <KoStyleStack.h>
#include <KoBorder.h>

#include <QPainter>
#include <QVariant>
#include <QPainterPath>
#include <QList>
#include <QMap>
#include <QByteArray>
#include <FlakeDebug.h>

#include "kis_assert.h"

#include "KoOdfGradientBackground.h"
#include <KisHandlePainterHelper.h>

// KoShape::Private

KoShape::Private::Private()
    : QSharedData()
    , size(50, 50)
    , parent(0)
    , shadow(0)
    , border(0)
    , filterEffectStack(0)
    , transparency(0.0)
    , zIndex(0)
    , runThrough(0)
    , visible(true)
    , printable(true)
    , geometryProtected(false)
    , keepAspect(false)
    , selectable(true)
    , detectCollision(false)
    , protectContent(false)
    , textRunAroundSide(KoShape::BiggestRunAroundSide)
    , textRunAroundDistanceLeft(0.0)
    , textRunAroundDistanceTop(0.0)
    , textRunAroundDistanceRight(0.0)
    , textRunAroundDistanceBottom(0.0)
    , textRunAroundThreshold(0.0)
    , textRunAroundContour(KoShape::ContourFull)
{
    connectors[KoConnectionPoint::TopConnectionPoint] = KoConnectionPoint::defaultConnectionPoint(KoConnectionPoint::TopConnectionPoint);
    connectors[KoConnectionPoint::RightConnectionPoint] = KoConnectionPoint::defaultConnectionPoint(KoConnectionPoint::RightConnectionPoint);
    connectors[KoConnectionPoint::BottomConnectionPoint] = KoConnectionPoint::defaultConnectionPoint(KoConnectionPoint::BottomConnectionPoint);
    connectors[KoConnectionPoint::LeftConnectionPoint] = KoConnectionPoint::defaultConnectionPoint(KoConnectionPoint::LeftConnectionPoint);
    connectors[KoConnectionPoint::FirstCustomConnectionPoint] = KoConnectionPoint(QPointF(0.5, 0.5), KoConnectionPoint::AllDirections, KoConnectionPoint::AlignCenter);
}

KoShape::Private::Private(const Private &rhs)
    : QSharedData()
    , size(rhs.size)
    , shapeId(rhs.shapeId)
    , name(rhs.name)
    , localMatrix(rhs.localMatrix)
    , connectors(rhs.connectors)
    , parent(0) // to be initialized later
    , shapeManagers() // to be initialized later
    , toolDelegates() // FIXME: how to initialize them?
    , userData(rhs.userData ? rhs.userData->clone() : 0)
    , stroke(rhs.stroke)
    , fill(rhs.fill)
    , inheritBackground(rhs.inheritBackground)
    , inheritStroke(rhs.inheritStroke)
    , dependees() // FIXME: how to initialize them?
    , shadow(0) // WARNING: not implemented in Krita
    , border(0) // WARNING: not implemented in Krita
    , clipPath(rhs.clipPath ? rhs.clipPath->clone() : 0)
    , clipMask(rhs.clipMask ? rhs.clipMask->clone() : 0)
    , additionalAttributes(rhs.additionalAttributes)
    , additionalStyleAttributes(rhs.additionalStyleAttributes)
    , filterEffectStack(0) // WARNING: not implemented in Krita
    , transparency(rhs.transparency)
    , hyperLink(rhs.hyperLink)

    , zIndex(rhs.zIndex)
    , runThrough(rhs.runThrough)
    , visible(rhs.visible)
    , printable(rhs.visible)
    , geometryProtected(rhs.geometryProtected)
    , keepAspect(rhs.keepAspect)
    , selectable(rhs.selectable)
    , detectCollision(rhs.detectCollision)
    , protectContent(rhs.protectContent)

    , textRunAroundSide(rhs.textRunAroundSide)
    , textRunAroundDistanceLeft(rhs.textRunAroundDistanceLeft)
    , textRunAroundDistanceTop(rhs.textRunAroundDistanceTop)
    , textRunAroundDistanceRight(rhs.textRunAroundDistanceRight)
    , textRunAroundDistanceBottom(rhs.textRunAroundDistanceBottom)
    , textRunAroundThreshold(rhs.textRunAroundThreshold)
    , textRunAroundContour(rhs.textRunAroundContour)
{
}

KoShape::Private::~Private()
{
    if (shadow && !shadow->deref())
        delete shadow;
    if (filterEffectStack && !filterEffectStack->deref())
        delete filterEffectStack;
}

void KoShape::shapeChangedPriv(KoShape::ChangeType type)
{
    if (d->parent)
        d->parent->model()->childChanged(this, type);

    this->shapeChanged(type);

    Q_FOREACH (KoShape * shape, d->dependees) {
        shape->shapeChanged(type, this);
    }

    Q_FOREACH (KoShape::ShapeChangeListener *listener, d->listeners) {
        listener->notifyShapeChangedImpl(type, this);
    }
}

void KoShape::addShapeManager(KoShapeManager *manager)
{
    d->shapeManagers.insert(manager);
}

void KoShape::removeShapeManager(KoShapeManager *manager)
{
    d->shapeManagers.remove(manager);
}

void KoShape::Private::convertFromShapeCoordinates(KoConnectionPoint &point, const QSizeF &shapeSize) const
{
    switch(point.alignment) {
    case KoConnectionPoint::AlignNone:
        point.position = KoFlake::toRelative(point.position, shapeSize);
        point.position.rx() = qBound<qreal>(0.0, point.position.x(), 1.0);
        point.position.ry() = qBound<qreal>(0.0, point.position.y(), 1.0);
        break;
    case KoConnectionPoint::AlignRight:
        point.position.rx() -= shapeSize.width();
        break;
    case KoConnectionPoint::AlignLeft:
        point.position.ry() = 0.5*shapeSize.height();
        break;
    case KoConnectionPoint::AlignBottom:
        point.position.ry() -= shapeSize.height();
        break;
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

void KoShape::Private::convertToShapeCoordinates(KoConnectionPoint &point, const QSizeF &shapeSize) const
{
    switch(point.alignment) {
    case KoConnectionPoint::AlignNone:
        point.position = KoFlake::toAbsolute(point.position, shapeSize);
        break;
    case KoConnectionPoint::AlignRight:
        point.position.rx() += shapeSize.width();
        break;
    case KoConnectionPoint::AlignLeft:
        point.position.ry() = 0.5*shapeSize.height();
        break;
    case KoConnectionPoint::AlignBottom:
        point.position.ry() += shapeSize.height();
        break;
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
QString KoShape::Private::getStyleProperty(const char *property, KoShapeLoadingContext &context)
{
    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
    QString value;

    if (styleStack.hasProperty(KoXmlNS::draw, property)) {
        value = styleStack.property(KoXmlNS::draw, property);
    }

    return value;
}



// ======== KoShape

const qint16 KoShape::maxZIndex = std::numeric_limits<qint16>::max();
const qint16 KoShape::minZIndex = std::numeric_limits<qint16>::min();

KoShape::KoShape()
    : d(new Private)
{
    notifyChanged();
}

KoShape::KoShape(const KoShape &rhs)
    : d(rhs.d)
{
}

KoShape::~KoShape()
{
    shapeChangedPriv(Deleted);
    d->listeners.clear();
    /**
     * The shape must have already been detached from all the parents and
     * shape managers. Otherwise we migh accidentally request some RTTI
     * information, which is not available anymore (we are in d-tor).
     *
     * TL;DR: fix the code that caused this destruction without unparenting
     *        instead of trying to remove these assert!
     */
    KIS_SAFE_ASSERT_RECOVER (!d->parent) {
        d->parent->removeShape(this);
    }

    KIS_SAFE_ASSERT_RECOVER (d->shapeManagers.isEmpty()) {
        Q_FOREACH (KoShapeManager *manager, d->shapeManagers) {
            manager->shapeInterface()->notifyShapeDestructed(this);
        }
        d->shapeManagers.clear();
    }
}

KoShape *KoShape::cloneShape() const
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(0 && "not implemented!");
    qWarning() << shapeId() << "cannot be cloned";
    return 0;
}

void KoShape::paintStroke(QPainter &painter, KoShapePaintingContext &paintcontext)
{
    Q_UNUSED(paintcontext);

    if (stroke()) {
        stroke()->paint(this, painter);
    }
}

void KoShape::scale(qreal sx, qreal sy)
{
    QPointF pos = position();
    QTransform scaleMatrix;
    scaleMatrix.translate(pos.x(), pos.y());
    scaleMatrix.scale(sx, sy);
    scaleMatrix.translate(-pos.x(), -pos.y());
    d->localMatrix = d->localMatrix * scaleMatrix;

    notifyChanged();
    shapeChangedPriv(ScaleChanged);
}

void KoShape::rotate(qreal angle)
{
    QPointF center = d->localMatrix.map(QPointF(0.5 * size().width(), 0.5 * size().height()));
    QTransform rotateMatrix;
    rotateMatrix.translate(center.x(), center.y());
    rotateMatrix.rotate(angle);
    rotateMatrix.translate(-center.x(), -center.y());
    d->localMatrix = d->localMatrix * rotateMatrix;

    notifyChanged();
    shapeChangedPriv(RotationChanged);
}

void KoShape::shear(qreal sx, qreal sy)
{
    QPointF pos = position();
    QTransform shearMatrix;
    shearMatrix.translate(pos.x(), pos.y());
    shearMatrix.shear(sx, sy);
    shearMatrix.translate(-pos.x(), -pos.y());
    d->localMatrix = d->localMatrix * shearMatrix;

    notifyChanged();
    shapeChangedPriv(ShearChanged);
}

void KoShape::setSize(const QSizeF &newSize)
{
    QSizeF oldSize(size());

    // always set size, as d->size and size() may vary
    setSizeImpl(newSize);

    if (oldSize == newSize)
        return;

    notifyChanged();
    shapeChangedPriv(SizeChanged);
}

void KoShape::setSizeImpl(const QSizeF &size) const
{
    d->size = size;
}

void KoShape::setPosition(const QPointF &newPosition)
{
    QPointF currentPos = position();
    if (newPosition == currentPos)
        return;
    QTransform translateMatrix;
    translateMatrix.translate(newPosition.x() - currentPos.x(), newPosition.y() - currentPos.y());
    d->localMatrix = d->localMatrix * translateMatrix;

    notifyChanged();
    shapeChangedPriv(PositionChanged);
}

bool KoShape::hitTest(const QPointF &position) const
{
    if (d->parent && d->parent->isClipped(this) && !d->parent->hitTest(position))
        return false;

    QPointF point = absoluteTransformation().inverted().map(position);
    QRectF bb = outlineRect();

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
    point = absoluteTransformation().inverted().map(position - d->shadow->offset());

    return bb.contains(point);
}

QRectF KoShape::boundingRect() const
{

    QTransform transform = absoluteTransformation();
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

QRectF KoShape::boundingRect(const QList<KoShape *> &shapes)
{
    QRectF boundingRect;
    Q_FOREACH (KoShape *shape, shapes) {
        boundingRect |= shape->boundingRect();
    }
    return boundingRect;
}

QRectF KoShape::absoluteOutlineRect() const
{
    return absoluteTransformation().map(outline()).boundingRect();
}

QRectF KoShape::absoluteOutlineRect(const QList<KoShape *> &shapes)
{
    QRectF absoluteOutlineRect;
    Q_FOREACH (KoShape *shape, shapes) {
        absoluteOutlineRect |= shape->absoluteOutlineRect();
    }
    return absoluteOutlineRect;
}

QTransform KoShape::absoluteTransformation() const
{
    QTransform matrix;
    // apply parents matrix to inherit any transformations done there.
    KoShapeContainer * container = d->parent;
    if (container) {
        if (container->inheritsTransform(this)) {
            matrix = container->absoluteTransformation();
        } else {
            QSizeF containerSize = container->size();
            QPointF containerPos = container->absolutePosition() - QPointF(0.5 * containerSize.width(), 0.5 * containerSize.height());
            matrix.translate(containerPos.x(), containerPos.y());
        }
    }

    return d->localMatrix * matrix;
}

void KoShape::applyAbsoluteTransformation(const QTransform &matrix)
{
    QTransform globalMatrix = absoluteTransformation();
    // the transformation is relative to the global coordinate system
    // but we want to change the local matrix, so convert the matrix
    // to be relative to the local coordinate system
    QTransform transformMatrix = globalMatrix * matrix * globalMatrix.inverted();
    applyTransformation(transformMatrix);
}

void KoShape::applyTransformation(const QTransform &matrix)
{
    d->localMatrix = matrix * d->localMatrix;
    notifyChanged();
    shapeChangedPriv(GenericMatrixChange);
}

void KoShape::setTransformation(const QTransform &matrix)
{
    d->localMatrix = matrix;
    notifyChanged();
    shapeChangedPriv(GenericMatrixChange);
}

QTransform KoShape::transformation() const
{
    return d->localMatrix;
}

KoShape::ChildZOrderPolicy KoShape::childZOrderPolicy()
{
    return ChildZDefault;
}

bool KoShape::compareShapeZIndex(KoShape *s1, KoShape *s2)
{
    /**
     * WARNING: Our definition of zIndex is not yet compatible with SVG2's
     *          definition. In SVG stacking context of groups with the same
     *          zIndex are **merged**, while in Krita the contents of groups
     *          is never merged. One group will always below than the other.
     *          Therefore, when zIndex of two groups inside the same parent
     *          coincide, the resulting painting order in Krita is
     *          **UNDEFINED**.
     *
     *          To avoid this trouble we use  KoShapeReorderCommand::mergeInShape()
     *          inside KoShapeCreateCommand.
     */

    /**
     * The algorithm below doesn't correctly handle the case when the two pointers actually
     * point to the same shape. So just check it in advance to guarantee strict weak ordering
     * relation requirement
     */
    if (s1 == s2) return false;


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

    if (d->parent == parent) {
        return;
    }

    KoShapeContainer *oldParent = d->parent;
    d->parent = 0; // avoids recursive removing

    if (oldParent) {
        oldParent->shapeInterface()->removeShape(this);
    }

    KIS_SAFE_ASSERT_RECOVER_NOOP(parent != this);

    if (parent && parent != this) {
        d->parent = parent;
        parent->shapeInterface()->addShape(this);
    }

    notifyChanged();
    shapeChangedPriv(ParentChanged);
}

bool KoShape::inheritsTransformFromAny(const QList<KoShape *> ancestorsInQuestion) const
{
    bool result = false;

    KoShape *shape = const_cast<KoShape*>(this);
    while (shape) {
        KoShapeContainer *parent = shape->parent();
        if (parent && !parent->inheritsTransform(shape)) {
            break;
        }

        if (ancestorsInQuestion.contains(shape)) {
            result = true;
            break;
        }

        shape = parent;
    }

    return result;
}

bool KoShape::hasCommonParent(const KoShape *shape) const
{
    const KoShape *thisShape = this;
    while (thisShape) {

        const KoShape *otherShape = shape;
        while (otherShape) {
            if (thisShape == otherShape) {
                return true;
            }
            otherShape = otherShape->parent();
        }

        thisShape = thisShape->parent();
    }

    return false;
}

qint16 KoShape::zIndex() const
{
    return d->zIndex;
}

void KoShape::update() const
{

    if (!d->shapeManagers.empty()) {
        QRectF rect(boundingRect());
        Q_FOREACH (KoShapeManager * manager, d->shapeManagers) {
            manager->update(rect, this, true);
        }
    }
}

void KoShape::updateAbsolute(const QRectF &rect) const
{
    if (rect.isEmpty() && !rect.isNull()) {
        return;
    }


    if (!d->shapeManagers.empty() && isVisible()) {
        Q_FOREACH (KoShapeManager *manager, d->shapeManagers) {
            manager->update(rect);
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
    if (background()) {
        return outline();
    }

    return QPainterPath();
}

QPointF KoShape::absolutePosition(KoFlake::AnchorPosition anchor) const
{
    const QRectF rc = outlineRect();

    QPointF point = rc.topLeft();

    bool valid = false;
    QPointF anchoredPoint = KoFlake::anchorToPoint(anchor, rc, &valid);
    if (valid) {
        point = anchoredPoint;
    }

    return absoluteTransformation().map(point);
}

void KoShape::setAbsolutePosition(const QPointF &newPosition, KoFlake::AnchorPosition anchor)
{
    QPointF currentAbsPosition = absolutePosition(anchor);
    QPointF translate = newPosition - currentAbsPosition;
    QTransform translateMatrix;
    translateMatrix.translate(translate.x(), translate.y());
    applyAbsoluteTransformation(translateMatrix);
    notifyChanged();
    shapeChangedPriv(PositionChanged);
}

void KoShape::copySettings(const KoShape *shape)
{
    d->size = shape->size();
    d->connectors.clear();
    Q_FOREACH (const KoConnectionPoint &point, shape->connectionPoints())
        addConnectionPoint(point);
    d->zIndex = shape->zIndex();
    d->visible = shape->isVisible(false);

    // Ensure printable is true by default
    if (!d->visible)
        d->printable = true;
    else
        d->printable = shape->isPrintable();

    d->geometryProtected = shape->isGeometryProtected();
    d->protectContent = shape->isContentProtected();
    d->selectable = shape->isSelectable();
    d->keepAspect = shape->keepAspectRatio();
    d->localMatrix = shape->d->localMatrix;
}

void KoShape::notifyChanged()
{
    Q_FOREACH (KoShapeManager * manager, d->shapeManagers) {
        manager->notifyShapeChanged(this);
    }
}

void KoShape::setUserData(KoShapeUserData *userData)
{
    d->userData.reset(userData);
}

KoShapeUserData *KoShape::userData() const
{
    return d->userData.data();
}

bool KoShape::hasTransparency() const
{
    QSharedPointer<KoShapeBackground> bg = background();

    return !bg || bg->hasTransparency() || d->transparency > 0.0;
}

void KoShape::setTransparency(qreal transparency)
{
    d->transparency = qBound<qreal>(0.0, transparency, 1.0);

    shapeChangedPriv(TransparencyChanged);
    notifyChanged();
}

qreal KoShape::transparency(bool recursive) const
{
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
    KoInsets answer;
    if (d->stroke)
        d->stroke->strokeInsets(this, answer);
    return answer;
}

qreal KoShape::rotation() const
{
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
    return d->size;
}

QPointF KoShape::position() const
{
    QPointF center = outlineRect().center();
    return d->localMatrix.map(center) - center;
}

int KoShape::addConnectionPoint(const KoConnectionPoint &point)
{

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
        shapeChangedPriv(ConnectionPointChanged);

    return true;
}

bool KoShape::hasConnectionPoint(int connectionPointId) const
{
    return d->connectors.contains(connectionPointId);
}

KoConnectionPoint KoShape::connectionPoint(int connectionPointId) const
{
    KoConnectionPoint p = d->connectors.value(connectionPointId, KoConnectionPoint());
    // convert glue point to shape coordinates
    d->convertToShapeCoordinates(p, size());
    return p;
}

KoConnectionPoints KoShape::connectionPoints() const
{
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
    d->connectors.remove(connectionPointId);
    shapeChangedPriv(ConnectionPointChanged);
}

void KoShape::clearConnectionPoints()
{
    d->connectors.clear();
}

KoShape::TextRunAroundSide KoShape::textRunAroundSide() const
{
    return d->textRunAroundSide;
}

void KoShape::setTextRunAroundSide(TextRunAroundSide side, RunThroughLevel runThrought)
{

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
    shapeChangedPriv(TextRunAroundChanged);
}

qreal KoShape::textRunAroundDistanceTop() const
{
    return d->textRunAroundDistanceTop;
}

void KoShape::setTextRunAroundDistanceTop(qreal distance)
{
    d->textRunAroundDistanceTop = distance;
}

qreal KoShape::textRunAroundDistanceLeft() const
{
    return d->textRunAroundDistanceLeft;
}

void KoShape::setTextRunAroundDistanceLeft(qreal distance)
{
    d->textRunAroundDistanceLeft = distance;
}

qreal KoShape::textRunAroundDistanceRight() const
{
    return d->textRunAroundDistanceRight;
}

void KoShape::setTextRunAroundDistanceRight(qreal distance)
{
    d->textRunAroundDistanceRight = distance;
}

qreal KoShape::textRunAroundDistanceBottom() const
{
    return d->textRunAroundDistanceBottom;
}

void KoShape::setTextRunAroundDistanceBottom(qreal distance)
{
    d->textRunAroundDistanceBottom = distance;
}

qreal KoShape::textRunAroundThreshold() const
{
    return d->textRunAroundThreshold;
}

void KoShape::setTextRunAroundThreshold(qreal threshold)
{
    d->textRunAroundThreshold = threshold;
}

KoShape::TextRunAroundContour KoShape::textRunAroundContour() const
{
    return d->textRunAroundContour;
}

void KoShape::setTextRunAroundContour(KoShape::TextRunAroundContour contour)
{
    d->textRunAroundContour = contour;
}

void KoShape::setBackground(QSharedPointer<KoShapeBackground> fill)
{
    d->inheritBackground = false;
    d->fill = fill;
    shapeChangedPriv(BackgroundChanged);
    notifyChanged();
}

QSharedPointer<KoShapeBackground> KoShape::background() const
{

    QSharedPointer<KoShapeBackground> bg;

    if (!d->inheritBackground) {
        bg = d->fill;
    } else if (parent()) {
        bg = parent()->background();
    }

    return bg;
}

void KoShape::setInheritBackground(bool value)
{

    d->inheritBackground = value;
    if (d->inheritBackground) {
        d->fill.clear();
    }
}

bool KoShape::inheritBackground() const
{
    return d->inheritBackground;
}

void KoShape::setZIndex(qint16 zIndex)
{
    if (d->zIndex == zIndex)
        return;
    d->zIndex = zIndex;
    notifyChanged();
}

int KoShape::runThrough()
{
    return d->runThrough;
}

void KoShape::setRunThrough(short int runThrough)
{
    d->runThrough = runThrough;
}

void KoShape::setVisible(bool on)
{
    int _on = (on ? 1 : 0);
    if (d->visible == _on) return;
    d->visible = _on;
}

bool KoShape::isVisible(bool recursive) const
{
    if (!recursive)
        return d->visible;

    if (!d->visible)
        return false;

    KoShapeContainer * parentShape = parent();

    if (parentShape) {
        return parentShape->isVisible(true);
    }

    return true;
}

void KoShape::setPrintable(bool on)
{
    d->printable = on;
}

bool KoShape::isPrintable() const
{
    if (d->visible)
        return d->printable;
    else
        return false;
}

void KoShape::setSelectable(bool selectable)
{
    d->selectable = selectable;
}

bool KoShape::isSelectable() const
{
    return d->selectable;
}

void KoShape::setGeometryProtected(bool on)
{
    d->geometryProtected = on;
}

bool KoShape::isGeometryProtected() const
{
    return d->geometryProtected;
}

void KoShape::setContentProtected(bool protect)
{
    d->protectContent = protect;
}

bool KoShape::isContentProtected() const
{
    return d->protectContent;
}

KoShapeContainer *KoShape::parent() const
{
    return d->parent;
}

void KoShape::setKeepAspectRatio(bool keepAspect)
{
    d->keepAspect = keepAspect;

    shapeChangedPriv(KeepAspectRatioChange);
    notifyChanged();
}

bool KoShape::keepAspectRatio() const
{
    return d->keepAspect;
}

QString KoShape::shapeId() const
{
    return d->shapeId;
}

void KoShape::setShapeId(const QString &id)
{
    d->shapeId = id;
}

void KoShape::setCollisionDetection(bool detect)
{
    d->detectCollision = detect;
}

bool KoShape::collisionDetection()
{
    return d->detectCollision;
}

KoShapeStrokeModelSP KoShape::stroke() const
{

    KoShapeStrokeModelSP stroke;

    if (!d->inheritStroke) {
        stroke = d->stroke;
    } else if (parent()) {
        stroke = parent()->stroke();
    }

    return stroke;
}

void KoShape::setStroke(KoShapeStrokeModelSP stroke)
{

    d->inheritStroke = false;
    d->stroke = stroke;
    shapeChangedPriv(StrokeChanged);
    notifyChanged();
}

void KoShape::setInheritStroke(bool value)
{
    d->inheritStroke = value;
    if (d->inheritStroke) {
        d->stroke.clear();
    }
}

bool KoShape::inheritStroke() const
{
    return d->inheritStroke;
}

void KoShape::setShadow(KoShapeShadow *shadow)
{
    if (d->shadow)
        d->shadow->deref();
    d->shadow = shadow;
    if (d->shadow) {
        d->shadow->ref();
        // TODO update changed area
    }
    shapeChangedPriv(ShadowChanged);
    notifyChanged();
}

KoShapeShadow *KoShape::shadow() const
{
    return d->shadow;
}

void KoShape::setBorder(KoBorder *border)
{
    if (d->border) {
        // The shape owns the border.
        delete d->border;
    }
    d->border = border;
    shapeChangedPriv(BorderChanged);
    notifyChanged();
}

KoBorder *KoShape::border() const
{
    return d->border;
}

void KoShape::setClipPath(KoClipPath *clipPath)
{
    d->clipPath.reset(clipPath);
    shapeChangedPriv(ClipPathChanged);
    notifyChanged();
}

KoClipPath * KoShape::clipPath() const
{
    return d->clipPath.data();
}

void KoShape::setClipMask(KoClipMask *clipMask)
{
    d->clipMask.reset(clipMask);
    shapeChangedPriv(ClipMaskChanged);
    notifyChanged();
}

KoClipMask* KoShape::clipMask() const
{
    return d->clipMask.data();
}

QTransform KoShape::transform() const
{
    return d->localMatrix;
}

QString KoShape::name() const
{
    return d->name;
}

void KoShape::setName(const QString &name)
{
    d->name = name;
}

void KoShape::waitUntilReady(bool asynchronous) const
{
    Q_UNUSED(asynchronous);
}

bool KoShape::isShapeEditable(bool recursive) const
{
    if (!d->visible || d->geometryProtected)
        return false;

    if (recursive && d->parent) {
        return d->parent->isShapeEditable(true);
    }

    return true;
}

// loading & saving methods
QString KoShape::saveStyle(KoGenStyle &style, KoShapeSavingContext &context) const
{
    // and fill the style
    KoShapeStrokeModelSP sm = stroke();
    if (sm) {
        sm->fillStyle(style, context);
    }
    else {
        style.addProperty("draw:stroke", "none", KoGenStyle::GraphicType);
    }
    KoShapeShadow *s = shadow();
    if (s)
        s->fillStyle(style, context);

    QSharedPointer<KoShapeBackground> bg = background();
    if (bg) {
        bg->fillStyle(style, context);
    }
    else {
        style.addProperty("draw:fill", "none", KoGenStyle::GraphicType);
    }

    KoBorder *b = border();
    if (b) {
        b->saveOdf(style);
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

    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
    styleStack.setTypeProperties("graphic");

    d->fill.clear();
    d->stroke.clear();

    if (d->shadow && !d->shadow->deref()) {
        delete d->shadow;
        d->shadow = 0;
    }
    setBackground(loadOdfFill(context));
    setStroke(loadOdfStroke(element, context));
    setShadow(d->loadOdfShadow(context));
    setBorder(d->loadOdfBorder(context));

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
        Q_FOREACH (const KoShapeLoadingContext::AdditionalAttributeData &attributeData, additionalAttributeData) {
            if (element.hasAttributeNS(attributeData.ns, attributeData.tag)) {
                QString value = element.attributeNS(attributeData.ns, attributeData.tag);
                //debugFlake << "load additional attribute" << attributeData.tag << value;
                setAdditionalAttribute(attributeData.name, value);
            }
        }
    }

    if (attributes & OdfCommonChildElements) {
        // load glue points (connection points)
        loadOdfGluePoints(element, context);
    }

    return true;
}

QSharedPointer<KoShapeBackground> KoShape::loadOdfFill(KoShapeLoadingContext &context) const
{
    QString fill = KoShape::Private::getStyleProperty("fill", context);
    QSharedPointer<KoShapeBackground> bg;
    if (fill == "solid") {
        bg = QSharedPointer<KoShapeBackground>(new KoColorBackground());
    }
    else if (fill == "hatch") {
        bg = QSharedPointer<KoShapeBackground>(new KoHatchBackground());
    }
    else if (fill == "gradient") {
        QString styleName = KoShape::Private::getStyleProperty("fill-gradient-name", context);
        KoXmlElement *e = context.odfLoadingContext().stylesReader().drawStyles("gradient")[styleName];
        QString style;
        if (e) {
            style = e->attributeNS(KoXmlNS::draw, "style", QString());
        }
        if ((style == "rectangular") || (style == "square")) {
            bg = QSharedPointer<KoShapeBackground>(new KoOdfGradientBackground());
        } else {
            QGradient *gradient = new QLinearGradient();
            gradient->setCoordinateMode(QGradient::ObjectBoundingMode);
            bg = QSharedPointer<KoShapeBackground>(new KoGradientBackground(gradient));
        }
    } else if (fill == "bitmap") {
        bg = QSharedPointer<KoShapeBackground>(new KoPatternBackground(context.imageCollection()));
#ifndef NWORKAROUND_ODF_BUGS
    } else if (fill.isEmpty()) {
        bg = QSharedPointer<KoShapeBackground>(KoOdfWorkaround::fixBackgroundColor(this, context));
        return bg;
#endif
    } else {
        return QSharedPointer<KoShapeBackground>(0);
    }

    if (!bg->loadStyle(context.odfLoadingContext(), size())) {
        return QSharedPointer<KoShapeBackground>(0);
    }

    return bg;
}

KoShapeStrokeModelSP KoShape::loadOdfStroke(const KoXmlElement &element, KoShapeLoadingContext &context) const
{
    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
    KoOdfStylesReader &stylesReader = context.odfLoadingContext().stylesReader();

    QString stroke = KoShape::Private::getStyleProperty("stroke", context);
    if (stroke == "solid" || stroke == "dash") {
        QPen pen = KoOdfGraphicStyles::loadOdfStrokeStyle(styleStack, stroke, stylesReader);

        QSharedPointer<KoShapeStroke> stroke(new KoShapeStroke());

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
            QSharedPointer<KoShapeStroke> stroke(new KoShapeStroke());

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

    return KoShapeStrokeModelSP();
}

KoShapeShadow *KoShape::Private::loadOdfShadow(KoShapeLoadingContext &context) const
{
    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
    QString shadowStyle = KoShape::Private::getStyleProperty("shadow", context);
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

KoBorder *KoShape::Private::loadOdfBorder(KoShapeLoadingContext &context) const
{
    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();

    KoBorder *border = new KoBorder();
    if (border->loadOdf(styleStack)) {
        return border;
    }
    delete border;
    return 0;
}


void KoShape::loadOdfGluePoints(const KoXmlElement &element, KoShapeLoadingContext &context)
{

    KoXmlElement child;
    bool hasCenterGluePoint = false;
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
        // connection point in center should be default but odf doesn't support,
        // in new shape, first custom point is in center, it's okay to replace that point
        // with point from xml now, we'll add it back later
        if(id.isEmpty() || index < KoConnectionPoint::FirstCustomConnectionPoint ||
                (index != KoConnectionPoint::FirstCustomConnectionPoint && d->connectors.contains(index))) {
            warnFlake << "glue-point with no or invalid id";
            continue;
        }
        QString xStr = child.attributeNS(KoXmlNS::svg, "x", QString()).simplified();
        QString yStr = child.attributeNS(KoXmlNS::svg, "y", QString()).simplified();
        if(xStr.isEmpty() || yStr.isEmpty()) {
            warnFlake << "glue-point with invald position";
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
                warnFlake << "glue-point with invald position";
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
            debugFlake << "using alignment" << align;
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
            debugFlake << "using escape direction" << escape;
        }
        d->connectors[index] = connector;
        debugFlake << "loaded glue-point" << index << "at position" << connector.position;
        if (d->connectors[index].position == QPointF(0.5, 0.5)) {
            hasCenterGluePoint = true;
            debugFlake << "center glue-point found at id " << index;
        }
    }
    if (!hasCenterGluePoint) {
        d->connectors[d->connectors.count()] = KoConnectionPoint(QPointF(0.5, 0.5),
                                                                 KoConnectionPoint::AllDirections, KoConnectionPoint::AlignCenter);
    }
    debugFlake << "shape has now" << d->connectors.count() << "glue-points";
}

void KoShape::loadOdfClipContour(const KoXmlElement &element, KoShapeLoadingContext &context, const QSizeF &scaleFactor)
{

    KoXmlElement child;
    forEachElement(child, element) {
        if (child.namespaceURI() != KoXmlNS::draw)
            continue;
        if (child.localName() != "contour-polygon")
            continue;

        debugFlake << "shape loads contour-polygon";
        KoPathShape *ps = new KoPathShape();
        ps->loadContourOdf(child, context, scaleFactor);
        ps->setTransformation(transformation());

        KoClipPath *clipPath = new KoClipPath({ps}, KoFlake::UserSpaceOnUse);
        d->clipPath.reset(clipPath);
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
            QPointF p = absolutePosition(KoFlake::TopLeft);
            QTransform shearMatrix;
            shearMatrix.translate(p.x(), p.y());
            shearMatrix.shear(tan(-params[0].toDouble()), 0.0F);
            shearMatrix.translate(-p.x(), -p.y());
            matrix = matrix * shearMatrix;
        } else if (cmd == "skewy") {
            QPointF p = absolutePosition(KoFlake::TopLeft);
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
        context.xmlWriter().addAttribute("svg:width", s.width());
        context.xmlWriter().addAttribute("svg:height", s.height());
    }

    // The position is implicitly stored in the transformation matrix
    // if the transformation is saved as well
    if ((attributes & OdfPosition) && !(attributes & OdfTransformation)) {
        const QPointF p(position() * context.shapeOffset(this));
        context.xmlWriter().addAttribute("svg:x", p.x());
        context.xmlWriter().addAttribute("svg:y", p.y());
    }

    if (attributes & OdfTransformation) {
        QTransform matrix = absoluteTransformation() * context.shapeOffset(this);
        if (! matrix.isIdentity()) {
            if (qAbs(matrix.m11() - 1) < 1E-5           // 1
                    && qAbs(matrix.m12()) < 1E-5        // 0
                    && qAbs(matrix.m21()) < 1E-5        // 0
                    && qAbs(matrix.m22() - 1) < 1E-5) { // 1
                context.xmlWriter().addAttribute("svg:x", matrix.dx());
                context.xmlWriter().addAttribute("svg:y", matrix.dy());
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
                context.xmlWriter().addAttribute("svg:x", cp.value().position.x());
                context.xmlWriter().addAttribute("svg:y", cp.value().position.y());
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

    debugFlake << "shape saves contour-polygon";
    if (d->clipPath && !d->clipPath->clipPathShapes().isEmpty()) {
        // This will loose data as odf can only save one set of contour whereas
        // svg loading and at least karbon editing can produce more than one
        // TODO, FIXME see if we can save more than one clipshape to odf
        d->clipPath->clipPathShapes().first()->saveContourOdf(context, originalSize);
    }
}

// end loading & saving methods

KisHandlePainterHelper KoShape::createHandlePainterHelperView(QPainter *painter, KoShape *shape, const KoViewConverter &converter, qreal handleRadius)
{
    const QTransform originalPainterTransform = painter->transform();

    painter->setTransform(shape->absoluteTransformation() *
                          converter.documentToView() *
                          painter->transform());

    // move c-tor
    return KisHandlePainterHelper(painter, originalPainterTransform, handleRadius);
}

KisHandlePainterHelper KoShape::createHandlePainterHelperDocument(QPainter *painter, KoShape *shape, qreal handleRadius)
{
    const QTransform originalPainterTransform = painter->transform();

    painter->setTransform(shape->absoluteTransformation() *
                          painter->transform());

    // move c-tor
    return KisHandlePainterHelper(painter, originalPainterTransform, handleRadius);
}


QPointF KoShape::shapeToDocument(const QPointF &point) const
{
    return absoluteTransformation().map(point);
}

QRectF KoShape::shapeToDocument(const QRectF &rect) const
{
    return absoluteTransformation().mapRect(rect);
}

QPointF KoShape::documentToShape(const QPointF &point) const
{
    return absoluteTransformation().inverted().map(point);
}

QRectF KoShape::documentToShape(const QRectF &rect) const
{
    return absoluteTransformation().inverted().mapRect(rect);
}

bool KoShape::addDependee(KoShape *shape)
{
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
    int index = d->dependees.indexOf(shape);
    if (index >= 0)
        d->dependees.removeAt(index);
}

bool KoShape::hasDependee(KoShape *shape) const
{
    return d->dependees.contains(shape);
}

QList<KoShape*> KoShape::dependees() const
{
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
    d->additionalAttributes.insert(name, value);
}

void KoShape::removeAdditionalAttribute(const QString &name)
{
    d->additionalAttributes.remove(name);
}

bool KoShape::hasAdditionalAttribute(const QString &name) const
{
    return d->additionalAttributes.contains(name);
}

QString KoShape::additionalAttribute(const QString &name) const
{
    return d->additionalAttributes.value(name);
}

void KoShape::setAdditionalStyleAttribute(const char *name, const QString &value)
{
    d->additionalStyleAttributes.insert(name, value);
}

void KoShape::removeAdditionalStyleAttribute(const char *name)
{
    d->additionalStyleAttributes.remove(name);
}

KoFilterEffectStack *KoShape::filterEffectStack() const
{
    return d->filterEffectStack;
}

void KoShape::setFilterEffectStack(KoFilterEffectStack *filterEffectStack)
{
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
    return d->toolDelegates;
}

void KoShape::setToolDelegates(const QSet<KoShape*> &delegates)
{
    d->toolDelegates = delegates;
}

QString KoShape::hyperLink () const
{
    return d->hyperLink;
}

void KoShape::setHyperLink(const QString &hyperLink)
{
    d->hyperLink = hyperLink;
}

KoShape::ShapeChangeListener::~ShapeChangeListener()
{
    Q_FOREACH(KoShape *shape, m_registeredShapes) {
        shape->removeShapeChangeListener(this);
    }
}

void KoShape::ShapeChangeListener::registerShape(KoShape *shape)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(!m_registeredShapes.contains(shape));
    m_registeredShapes.append(shape);
}

void KoShape::ShapeChangeListener::unregisterShape(KoShape *shape)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_registeredShapes.contains(shape));
    m_registeredShapes.removeAll(shape);
}

void KoShape::ShapeChangeListener::notifyShapeChangedImpl(KoShape::ChangeType type, KoShape *shape)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_registeredShapes.contains(shape));

    notifyShapeChanged(type, shape);

    if (type == KoShape::Deleted) {
        unregisterShape(shape);
    }
}

void KoShape::addShapeChangeListener(KoShape::ShapeChangeListener *listener)
{

    KIS_SAFE_ASSERT_RECOVER_RETURN(!d->listeners.contains(listener));
    listener->registerShape(this);
    d->listeners.append(listener);
}

void KoShape::removeShapeChangeListener(KoShape::ShapeChangeListener *listener)
{

    KIS_SAFE_ASSERT_RECOVER_RETURN(d->listeners.contains(listener));
    d->listeners.removeAll(listener);
    listener->unregisterShape(this);
}

QList<KoShape::ShapeChangeListener *> KoShape::listeners() const
{
    return d->listeners;
}

QList<KoShape *> KoShape::linearizeSubtree(const QList<KoShape *> &shapes)
{
    QList<KoShape *> result;

    Q_FOREACH (KoShape *shape, shapes) {
        result << shape;

        KoShapeContainer *container = dynamic_cast<KoShapeContainer*>(shape);
        if (container) {
            result << linearizeSubtree(container->shapes());
        }
    }

    return result;
}
