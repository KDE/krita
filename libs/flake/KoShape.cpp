/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2006 C. Boemann Rasmussen <cbo@boemann.dk>
   SPDX-FileCopyrightText: 2006-2010 Thomas Zander <zander@kde.org>
   SPDX-FileCopyrightText: 2006-2010 Thorsten Zachmann <zachmann@kde.org>
   SPDX-FileCopyrightText: 2007-2009, 2011 Jan Hambrecht <jaham@gmx.net>
   SPDX-FileCopyrightText: 2010 Boudewijn Rempt <boud@valdyas.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
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
#include "KoFilterEffectStack.h"
#include <KoSnapData.h>

#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoUnit.h>

#include <QPainter>
#include <QVariant>
#include <QPainterPath>
#include <QList>
#include <QMap>
#include <QByteArray>
#include <FlakeDebug.h>

#include "kis_assert.h"

#include <KisHandlePainterHelper.h>

// KoShape::Private

KoShape::SharedData::SharedData()
    : QSharedData()
    , size(50, 50)
    , shadow(0)
    , filterEffectStack(0)
    , transparency(0.0)
    , zIndex(0)
    , runThrough(0)
    , visible(true)
    , printable(true)
    , geometryProtected(false)
    , keepAspect(false)
    , selectable(true)
    , protectContent(false)
    , textRunAroundSide(KoShape::BiggestRunAroundSide)
    , textRunAroundDistanceLeft(0.0)
    , textRunAroundDistanceTop(0.0)
    , textRunAroundDistanceRight(0.0)
    , textRunAroundDistanceBottom(0.0)
    , textRunAroundThreshold(0.0)
    , textRunAroundContour(KoShape::ContourFull)
{ }

KoShape::SharedData::SharedData(const SharedData &rhs)
    : QSharedData()
    , size(rhs.size)
    , shapeId(rhs.shapeId)
    , name(rhs.name)
    , localMatrix(rhs.localMatrix)
    , userData(rhs.userData ? rhs.userData->clone() : 0)
    , stroke(rhs.stroke)
    , fill(rhs.fill)
    , inheritBackground(rhs.inheritBackground)
    , inheritStroke(rhs.inheritStroke)
    , shadow(0) // WARNING: not implemented in Krita
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

KoShape::SharedData::~SharedData()
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

// ======== KoShape

const qint16 KoShape::maxZIndex = std::numeric_limits<qint16>::max();
const qint16 KoShape::minZIndex = std::numeric_limits<qint16>::min();

KoShape::KoShape()
    : d(new Private()),
      s(new SharedData),
      m_absolute(true),
      m_extraTransform(QTransform())
{
    notifyChanged();
}

KoShape::KoShape(const KoShape &rhs)
    : d(new Private()),
      s(rhs.s)
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

void KoShape::paintStroke(QPainter &painter, KoShapePaintingContext &paintcontext) const
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
    s->localMatrix = s->localMatrix * scaleMatrix;

    notifyChanged();
    shapeChangedPriv(ScaleChanged);
}

void KoShape::rotate(qreal angle)
{
    QPointF center = s->localMatrix.map(QPointF(0.5 * size().width(), 0.5 * size().height()));
    QTransform rotateMatrix;
    rotateMatrix.translate(center.x(), center.y());
    rotateMatrix.rotate(angle);
    rotateMatrix.translate(-center.x(), -center.y());
    s->localMatrix = s->localMatrix * rotateMatrix;

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
    s->localMatrix = s->localMatrix * shearMatrix;

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
    s->size = size;
}

void KoShape::setPosition(const QPointF &newPosition)
{
    QPointF currentPos = position();
    if (newPosition == currentPos)
        return;
    QTransform translateMatrix;
    translateMatrix.translate(newPosition.x() - currentPos.x(), newPosition.y() - currentPos.y());
    s->localMatrix = s->localMatrix * translateMatrix;

    notifyChanged();
    shapeChangedPriv(PositionChanged);
}

bool KoShape::hitTest(const QPointF &position) const
{
    if (d->parent && d->parent->isClipped(this) && !d->parent->hitTest(position))
        return false;

    QPointF point = absoluteTransformation().inverted().map(position);
    QRectF bb = outlineRect();

    if (s->stroke) {
        KoInsets insets;
        s->stroke->strokeInsets(this, insets);
        bb.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
    }
    if (bb.contains(point))
        return true;

    // if there is no shadow we can as well just leave
    if (! s->shadow)
        return false;

    // the shadow has an offset to the shape, so we simply
    // check if the position minus the shadow offset hits the shape
    point = absoluteTransformation().inverted().map(position - s->shadow->offset());

    return bb.contains(point);
}

QRectF KoShape::boundingRect() const
{

    QTransform transform = absoluteTransformation();
    QRectF bb = outlineRect();
    if (s->stroke) {
        KoInsets insets;
        s->stroke->strokeInsets(this, insets);
        bb.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
    }
    bb = transform.mapRect(bb);
    if (s->shadow) {
        KoInsets insets;
        s->shadow->insets(insets);
        bb.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
    }
    if (s->filterEffectStack) {
        QRectF clipRect = s->filterEffectStack->clipRectForBoundingRect(outlineRect());
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
    if(m_absolute) {
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
    }
    matrix *= m_extraTransform;

    return s->localMatrix * matrix;
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
    s->localMatrix = matrix * s->localMatrix;
    notifyChanged();
    shapeChangedPriv(GenericMatrixChange);
}

void KoShape::setTransformation(const QTransform &matrix)
{
    s->localMatrix = matrix;
    notifyChanged();
    shapeChangedPriv(GenericMatrixChange);
}

QTransform KoShape::transformation() const
{
    return s->localMatrix;
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
    return s->zIndex;
}

void KoShape::update() const
{

    if (!d->shapeManagers.empty()) {
        const QRectF rect(boundingRect());
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
    s->size = shape->size();
    s->zIndex = shape->zIndex();
    s->visible = shape->isVisible(false);

    // Ensure printable is true by default
    if (!s->visible)
        s->printable = true;
    else
        s->printable = shape->isPrintable();

    s->geometryProtected = shape->isGeometryProtected();
    s->protectContent = shape->isContentProtected();
    s->selectable = shape->isSelectable();
    s->keepAspect = shape->keepAspectRatio();
    s->localMatrix = shape->s->localMatrix;
}

void KoShape::notifyChanged()
{
    Q_FOREACH (KoShapeManager * manager, d->shapeManagers) {
        manager->notifyShapeChanged(this);
    }
}

void KoShape::setUserData(KoShapeUserData *userData)
{
    s->userData.reset(userData);
}

KoShapeUserData *KoShape::userData() const
{
    return s->userData.data();
}

bool KoShape::hasTransparency() const
{
    QSharedPointer<KoShapeBackground> bg = background();

    return !bg || bg->hasTransparency() || s->transparency > 0.0;
}

void KoShape::setTransparency(qreal transparency)
{
    s->transparency = qBound<qreal>(0.0, transparency, 1.0);

    shapeChangedPriv(TransparencyChanged);
    notifyChanged();
}

qreal KoShape::transparency(bool recursive) const
{
    if (!recursive || !parent()) {
        return s->transparency;
    } else {
        const qreal parentOpacity = 1.0-parent()->transparency(recursive);
        const qreal childOpacity = 1.0-s->transparency;
        return 1.0-(parentOpacity*childOpacity);
    }
}

KoInsets KoShape::strokeInsets() const
{
    KoInsets answer;
    if (s->stroke)
        s->stroke->strokeInsets(this, answer);
    return answer;
}

qreal KoShape::rotation() const
{
    // try to extract the rotation angle out of the local matrix
    // if it is a pure rotation matrix

    // check if the matrix has shearing mixed in
    if (fabs(fabs(s->localMatrix.m12()) - fabs(s->localMatrix.m21())) > 1e-10)
        return std::numeric_limits<qreal>::quiet_NaN();
    // check if the matrix has scaling mixed in
    if (fabs(s->localMatrix.m11() - s->localMatrix.m22()) > 1e-10)
        return std::numeric_limits<qreal>::quiet_NaN();

    // calculate the angle from the matrix elements
    qreal angle = atan2(-s->localMatrix.m21(), s->localMatrix.m11()) * 180.0 / M_PI;
    if (angle < 0.0)
        angle += 360.0;

    return angle;
}

QSizeF KoShape::size() const
{
    return s->size;
}

QPointF KoShape::position() const
{
    QPointF center = outlineRect().center();
    return s->localMatrix.map(center) - center;
}

KoShape::TextRunAroundSide KoShape::textRunAroundSide() const
{
    return s->textRunAroundSide;
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

    if ( s->textRunAroundSide == side) {
        return;
    }

    s->textRunAroundSide = side;
    notifyChanged();
    shapeChangedPriv(TextRunAroundChanged);
}

qreal KoShape::textRunAroundDistanceTop() const
{
    return s->textRunAroundDistanceTop;
}

void KoShape::setTextRunAroundDistanceTop(qreal distance)
{
    s->textRunAroundDistanceTop = distance;
}

qreal KoShape::textRunAroundDistanceLeft() const
{
    return s->textRunAroundDistanceLeft;
}

void KoShape::setTextRunAroundDistanceLeft(qreal distance)
{
    s->textRunAroundDistanceLeft = distance;
}

qreal KoShape::textRunAroundDistanceRight() const
{
    return s->textRunAroundDistanceRight;
}

void KoShape::setTextRunAroundDistanceRight(qreal distance)
{
    s->textRunAroundDistanceRight = distance;
}

qreal KoShape::textRunAroundDistanceBottom() const
{
    return s->textRunAroundDistanceBottom;
}

void KoShape::setTextRunAroundDistanceBottom(qreal distance)
{
    s->textRunAroundDistanceBottom = distance;
}

qreal KoShape::textRunAroundThreshold() const
{
    return s->textRunAroundThreshold;
}

void KoShape::setTextRunAroundThreshold(qreal threshold)
{
    s->textRunAroundThreshold = threshold;
}

KoShape::TextRunAroundContour KoShape::textRunAroundContour() const
{
    return s->textRunAroundContour;
}

void KoShape::setTextRunAroundContour(KoShape::TextRunAroundContour contour)
{
    s->textRunAroundContour = contour;
}

void KoShape::setBackground(QSharedPointer<KoShapeBackground> fill)
{
    s->inheritBackground = false;
    s->fill = fill;
    shapeChangedPriv(BackgroundChanged);
    notifyChanged();
}

QSharedPointer<KoShapeBackground> KoShape::background() const
{

    QSharedPointer<KoShapeBackground> bg;

    if (!s->inheritBackground) {
        bg = s->fill;
    } else if (parent()) {
        bg = parent()->background();
    }

    return bg;
}

void KoShape::setInheritBackground(bool value)
{

    s->inheritBackground = value;
    if (s->inheritBackground) {
        s->fill.clear();
    }
}

bool KoShape::inheritBackground() const
{
    return s->inheritBackground;
}

void KoShape::setZIndex(qint16 zIndex)
{
    if (s->zIndex == zIndex)
        return;
    s->zIndex = zIndex;
    notifyChanged();
}

int KoShape::runThrough() const
{
    return s->runThrough;
}

void KoShape::setRunThrough(short int runThrough)
{
    s->runThrough = runThrough;
}

void KoShape::setVisible(bool on)
{
    int _on = (on ? 1 : 0);
    if (s->visible == _on) return;
    s->visible = _on;
}

bool KoShape::isVisible(bool recursive) const
{
    if (!recursive)
        return s->visible;

    if (!s->visible)
        return false;

    KoShapeContainer * parentShape = parent();

    if (parentShape) {
        return parentShape->isVisible(true);
    }

    return true;
}

void KoShape::setPrintable(bool on)
{
    s->printable = on;
}

bool KoShape::isPrintable() const
{
    if (s->visible)
        return s->printable;
    else
        return false;
}

void KoShape::setSelectable(bool selectable)
{
    s->selectable = selectable;
}

bool KoShape::isSelectable() const
{
    return s->selectable;
}

void KoShape::setGeometryProtected(bool on)
{
    s->geometryProtected = on;
}

bool KoShape::isGeometryProtected() const
{
    return s->geometryProtected;
}

void KoShape::setContentProtected(bool protect)
{
    s->protectContent = protect;
}

bool KoShape::isContentProtected() const
{
    return s->protectContent;
}

KoShapeContainer *KoShape::parent() const
{
    return d->parent;
}

void KoShape::setKeepAspectRatio(bool keepAspect)
{
    s->keepAspect = keepAspect;

    shapeChangedPriv(KeepAspectRatioChange);
    notifyChanged();
}

bool KoShape::keepAspectRatio() const
{
    return s->keepAspect;
}

QString KoShape::shapeId() const
{
    return s->shapeId;
}

void KoShape::setShapeId(const QString &id)
{
    s->shapeId = id;
}

KoShapeStrokeModelSP KoShape::stroke() const
{

    KoShapeStrokeModelSP stroke;

    if (!s->inheritStroke) {
        stroke = s->stroke;
    } else if (parent()) {
        stroke = parent()->stroke();
    }

    return stroke;
}

void KoShape::setStroke(KoShapeStrokeModelSP stroke)
{

    s->inheritStroke = false;
    s->stroke = stroke;
    shapeChangedPriv(StrokeChanged);
    notifyChanged();
}

void KoShape::setInheritStroke(bool value)
{
    s->inheritStroke = value;
    if (s->inheritStroke) {
        s->stroke.clear();
    }
}

bool KoShape::inheritStroke() const
{
    return s->inheritStroke;
}

void KoShape::setShadow(KoShapeShadow *shadow)
{
    if (s->shadow)
        s->shadow->deref();
    s->shadow = shadow;
    if (s->shadow) {
        s->shadow->ref();
        // TODO update changed area
    }
    shapeChangedPriv(ShadowChanged);
    notifyChanged();
}

KoShapeShadow *KoShape::shadow() const
{
    return s->shadow;
}

void KoShape::setClipPath(KoClipPath *clipPath)
{
    s->clipPath.reset(clipPath);
    shapeChangedPriv(ClipPathChanged);
    notifyChanged();
}

KoClipPath * KoShape::clipPath() const
{
    return s->clipPath.data();
}

void KoShape::setClipMask(KoClipMask *clipMask)
{
    s->clipMask.reset(clipMask);
    shapeChangedPriv(ClipMaskChanged);
    notifyChanged();
}

KoClipMask* KoShape::clipMask() const
{
    return s->clipMask.data();
}

QTransform KoShape::transform() const
{
    return s->localMatrix;
}

QString KoShape::name() const
{
    return s->name;
}

void KoShape::setName(const QString &name)
{
    s->name = name;
}

void KoShape::waitUntilReady(bool asynchronous) const
{
    Q_UNUSED(asynchronous);
}

bool KoShape::isShapeEditable(bool recursive) const
{
    if (!s->visible || s->geometryProtected)
        return false;

    if (recursive && d->parent) {
        return d->parent->isShapeEditable(true);
    }

    return true;
}

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
    s->additionalAttributes.insert(name, value);
}

void KoShape::removeAdditionalAttribute(const QString &name)
{
    s->additionalAttributes.remove(name);
}

bool KoShape::hasAdditionalAttribute(const QString &name) const
{
    return s->additionalAttributes.contains(name);
}

QString KoShape::additionalAttribute(const QString &name) const
{
    return s->additionalAttributes.value(name);
}

void KoShape::setAdditionalStyleAttribute(const char *name, const QString &value)
{
    s->additionalStyleAttributes.insert(name, value);
}

void KoShape::removeAdditionalStyleAttribute(const char *name)
{
    s->additionalStyleAttributes.remove(name);
}

KoFilterEffectStack *KoShape::filterEffectStack() const
{
    return s->filterEffectStack;
}

void KoShape::setFilterEffectStack(KoFilterEffectStack *filterEffectStack)
{
    if (s->filterEffectStack)
        s->filterEffectStack->deref();
    s->filterEffectStack = filterEffectStack;
    if (s->filterEffectStack) {
        s->filterEffectStack->ref();
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
    return s->hyperLink;
}

void KoShape::setHyperLink(const QString &hyperLink)
{
    s->hyperLink = hyperLink;
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

QList<KoShape *> KoShape::linearizeSubtreeSorted(const QList<KoShape *> &shapes)
{
    QList<KoShape*> sortedShapes = shapes;
    std::sort(sortedShapes.begin(), sortedShapes.end(), KoShape::compareShapeZIndex);

    QList<KoShape *> result;

    Q_FOREACH (KoShape *shape, sortedShapes) {
        result << shape;

        KoShapeContainer *container = dynamic_cast<KoShapeContainer*>(shape);
        if (container) {
            result << linearizeSubtreeSorted(container->shapes());
        }
    }

    return result;
}

void KoShape::setAbsolute(bool value)
{
    m_absolute = value;
}

QTransform KoShape::extraTransform() const
{
    return m_extraTransform;
}

void KoShape::setExtraTransform(QTransform t)
{
    m_extraTransform = t;
    notifyChanged();
    shapeChangedPriv(GenericMatrixChange);
}
