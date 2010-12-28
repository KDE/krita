#include "KoTosContainerModel.h"

KoTosContainerModel::KoTosContainerModel()
: m_textShape(0)
{
}

KoTosContainerModel::~KoTosContainerModel()
{
}

void KoTosContainerModel::add(KoShape *shape)
{
    // make sure shape is a text shape
    KoTextShapeDataBase *shapeData = qobject_cast<KoTextShapeDataBase*>(shape->userData());
    Q_ASSERT(shapeData != 0);
    if (shapeData) {
        m_textShape = shape;
    }
}

void KoTosContainerModel::remove(KoShape *shape)
{
    Q_ASSERT(shape == m_textShape);
    if (shape == m_textShape) {
        m_textShape = 0;
    }
}

void KoTosContainerModel::setClipped(const KoShape *shape, bool clipping)
{
    Q_UNUSED(shape);
    Q_UNUSED(clipping);
}

bool KoTosContainerModel::isClipped(const KoShape *shape) const
{
    return false;
}

void KoTosContainerModel::setInheritsTransform(const KoShape *shape, bool inherit)
{
    Q_UNUSED(shape);
    Q_UNUSED(inherit);
}

bool KoTosContainerModel::inheritsTransform(const KoShape *shape) const
{
    return true;
}

bool KoTosContainerModel::isChildLocked(const KoShape *child) const
{
    Q_ASSERT(child == m_textShape);
    Q_ASSERT(child->parent());
    // TODO do we need to guard this?
    return child->isGeometryProtected() || child->parent()->isGeometryProtected();
}

int KoTosContainerModel::count() const
{
    return m_textShape != 0 ? 1 : 0;
}

QList<KoShape*> KoTosContainerModel::shapes() const
{
    QList<KoShape*> shapes;
    shapes << m_textShape;
    return shapes;
}

void KoTosContainerModel::containerChanged(KoShapeContainer *container, KoShape::ChangeType type)
{
}
