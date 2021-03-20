/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoShapeGroup.h"
#include "KoShapeContainerModel.h"
#include "KoShapeContainer_p.h"
#include "KoShapeLayer.h"
#include "SimpleShapeContainerModel.h"
#include "KoShapeSavingContext.h"
#include "KoShapeLoadingContext.h"
#include "KoXmlWriter.h"
#include "KoXmlReader.h"
#include "KoShapeRegistry.h"
#include "KoShapeStrokeModel.h"
#include "KoShapeShadow.h"
#include "KoInsets.h"

#include <FlakeDebug.h>

#include <QPainter>

class ShapeGroupContainerModel : public SimpleShapeContainerModel
{
public:
    ShapeGroupContainerModel(KoShapeGroup *group) : m_group(group) {}
    ~ShapeGroupContainerModel() override {}

    ShapeGroupContainerModel(const ShapeGroupContainerModel &rhs, KoShapeGroup *group)
        : SimpleShapeContainerModel(rhs),
          m_group(group)
    {
    }

    void add(KoShape *child) override
    {
        SimpleShapeContainerModel::add(child);
        m_group->invalidateSizeCache();
    }

    void remove(KoShape *child) override
    {
        SimpleShapeContainerModel::remove(child);
        m_group->invalidateSizeCache();
    }

    void childChanged(KoShape *shape, KoShape::ChangeType type) override
    {
        SimpleShapeContainerModel::childChanged(shape, type);
        //debugFlake << type;
        switch (type) {
        case KoShape::PositionChanged:
        case KoShape::RotationChanged:
        case KoShape::ScaleChanged:
        case KoShape::ShearChanged:
        case KoShape::SizeChanged:
        case KoShape::GenericMatrixChange:
        case KoShape::ParameterChanged:
        case KoShape::ClipPathChanged :
        case KoShape::ClipMaskChanged :
            m_group->invalidateSizeCache();
            break;
        default:
            break;
        }
    }

private: // members
    KoShapeGroup * m_group;
};

class KoShapeGroup::Private
{
public:
    Private() {}

    Private(const Private &) {}

    virtual ~Private() = default;

    mutable QRectF savedOutlineRect;
    mutable bool sizeCached = false;

};

KoShapeGroup::KoShapeGroup()
    : KoShapeContainer()
    , d(new Private)
{
    setModelInit(new ShapeGroupContainerModel(this));
}

KoShapeGroup::KoShapeGroup(const KoShapeGroup &rhs)
    : KoShapeContainer(rhs)
    , d(new Private(*rhs.d))
{
    ShapeGroupContainerModel *otherModel = dynamic_cast<ShapeGroupContainerModel*>(rhs.model());
    KIS_ASSERT_RECOVER_RETURN(otherModel);
    setModelInit(new ShapeGroupContainerModel(*otherModel, this));
}

KoShapeGroup::~KoShapeGroup()
{
    /**
     * HACK alert: model will use KoShapeGroup::invalidateSizeCache(), which uses
     * KoShapeGroup's d-pointer. We have to manually remove child shapes from the
     * model in the destructor of KoShapeGroup as the instance d is no longer accessible
     * since ~KoShapeGroup() is executed
     */
    model()->deleteOwnedShapes();
}

KoShape *KoShapeGroup::cloneShape() const
{
    return new KoShapeGroup(*this);
}

void KoShapeGroup::paintComponent(QPainter &painter, KoShapePaintingContext &) const
{
    Q_UNUSED(painter);
}

bool KoShapeGroup::hitTest(const QPointF &position) const
{
    Q_UNUSED(position);
    return false;
}

void KoShapeGroup::tryUpdateCachedSize() const
{
    if (!d->sizeCached) {
        QRectF bound;
        Q_FOREACH (KoShape *shape, shapes()) {
            bound |= shape->transformation().mapRect(shape->outlineRect());
        }
        d->savedOutlineRect = bound;
        KoShape::setSizeImpl(bound.size());
        d->sizeCached = true;
    }
}

QSizeF KoShapeGroup::size() const
{
    tryUpdateCachedSize();
    return KoShape::size();
}

void KoShapeGroup::setSize(const QSizeF &size)
{
    QSizeF oldSize = this->size();
    if (!shapeCount() || oldSize.isNull()) return;

    const QTransform scale =
        QTransform::fromScale(size.width() / oldSize.width(), size.height() / oldSize.height());

    setTransformation(scale * transformation());

    KoShapeContainer::setSize(size);
}

QRectF KoShapeGroup::outlineRect() const
{
    tryUpdateCachedSize();
    return d->savedOutlineRect;
}

QRectF KoShapeGroup::boundingRect() const
{
    QRectF groupBound = KoShape::boundingRect(shapes());

    if (shadow()) {
        KoInsets insets;
        shadow()->insets(insets);
        groupBound.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
    }
    return groupBound;
}

void KoShapeGroup::shapeChanged(ChangeType type, KoShape *shape)
{
    Q_UNUSED(shape);
    KoShapeContainer::shapeChanged(type, shape);
    switch (type) {
    case KoShape::StrokeChanged:
        break;
    default:
        break;
    }

    invalidateSizeCache();
}

void KoShapeGroup::invalidateSizeCache()
{
    d->sizeCached = false;
}
