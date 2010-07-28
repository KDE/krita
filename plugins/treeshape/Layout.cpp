/* This file is part of the KDE project

   Copyright (c) 2010 Cyril Oblikov <munknex@gmail.com>

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

#include <QSizeF>
#include <QPointF>
#include <QMap>

#include "Layout.h"
#include "TreeShape.h"

#include "KoShape.h"
#include <KoShapeContainer.h>
#include "KoConnectionShape.h"
#include "kdebug.h"

Layout::Layout(KoShapeContainer *container)
    : m_container(container)
    , m_structure(TreeShape::OrgDown)
    , m_doingLayout(false)
    , m_relayoutScheduled(false)
    , m_root(0)
{
}

Layout::~Layout()
{
}

void Layout::add(KoShape *shape)
{
    Q_ASSERT(!m_children.contains(shape));
    TreeShape *tree = dynamic_cast<TreeShape*>(shape);
    if (tree) {
        tree->setZIndex(m_container->zIndex()+1);
        if (tree->nextShape()) {
            Q_ASSERT(m_children.contains(tree->nextShape()));
            int pos = m_children.indexOf(tree->nextShape());
            if (pos != 0) {
                TreeShape *prev = dynamic_cast<TreeShape*>(m_children[pos-1]);
                prev->setNextShape(shape);
            }
            m_children.insert(pos, shape);
        } else {
            if (!m_children.isEmpty()) {
                TreeShape *prev = dynamic_cast<TreeShape*>(m_children.last());
                prev->setNextShape(shape);
            }
            m_children.append(shape);
        }
    }

    KoConnectionShape *connector = dynamic_cast<KoConnectionShape*>(shape);
    if (connector) {
        Q_ASSERT(!m_connectors.contains(connector));
        m_connectors.append(shape);
    }

    scheduleRelayout();
}

void Layout::attachConnector(KoShape* shape, KoConnectionShape *connector)
{
    Q_ASSERT(m_children.contains(shape));
    Q_ASSERT(m_connectors.contains(connector));
    m_bonds[shape] = connector;
    //scheduleRelayout();
}

void Layout::setRoot(KoShape *shape)
{
    m_root = shape;
    m_container->setSize(m_root->size());
    m_lastWidth = m_container->size().width();
    m_lastHeight = m_container->size().height();

    // shape was added to children by setParent() method
    m_children.removeOne(m_root);
}

KoShape* Layout::root() const
{
    return m_root;
}

void Layout::setStructure(TreeShape::TreeType structure)
{
    m_structure = structure;
    scheduleRelayout();
}

TreeShape::TreeType Layout::structure() const
{
    return m_structure;
}

void Layout::remove(KoShape *shape)
{
    int pos = m_children.indexOf(shape);
    if (pos != -1) {
        if (pos != 0) {
            TreeShape *prev = dynamic_cast<TreeShape*>(m_children[pos-1]);
            KoShape *next = (shape==m_children.last()) ? 0 :m_children[pos+1];
            prev->setNextShape(next);
        }
        m_children.removeOne(shape);
        kDebug() << "child removed" << shape->shapeId();
        scheduleRelayout();
    }

    else if (m_connectors.removeOne(shape)) {
        kDebug() << "connector removed";
        scheduleRelayout();
    }
}

KoShape* Layout::connector(KoShape *shape)
{
    Q_ASSERT(m_children.contains(shape));
    return dynamic_cast<KoShape*>(m_bonds[shape]);
}

KoShape* Layout::proposePosition(KoShape* shape)
{
    if (m_children.isEmpty())
        return 0;

    KoShape *nextShape = 0;

    if (m_structure == TreeShape::OrgDown) {
        QPointF pos = shape->absoluteTransformation(0).map(shape->connectionPoints()[0]);
        pos = m_container->documentToShape(pos);

        nextShape = m_children.first();
        foreach(KoShape* child, m_children)
            if (child->position().x()>pos.x()) {
                nextShape = child;
                break;
            }
        if (nextShape->position().x()<pos.x())
            nextShape = 0;
    }

    return nextShape;
}

void Layout::setClipped(const KoShape *shape, bool clipping)
{
    Q_UNUSED(shape);
    Q_UNUSED(clipping);
}

bool Layout::isClipped(const KoShape *shape) const
{
    Q_UNUSED(shape);
    return false;
}

void Layout::setInheritsTransform(const KoShape *shape, bool inherit)
{
    Q_UNUSED(shape);
    Q_UNUSED(inherit);
}

bool Layout::inheritsTransform(const KoShape *shape) const
{
    Q_UNUSED(shape);
    return true;
}

int Layout::count() const
{
    return m_children.size()+m_connectors.size()+1;
}

QList<KoShape*> Layout::shapes() const
{
    //kDebug() << "";
    QList<KoShape*> all;
    all.append(m_children);
    all.append(m_connectors);
    all.append(m_root);
    return all;
}

void Layout::containerChanged(KoShapeContainer *container, KoShape::ChangeType type)
{
    Q_UNUSED(container);
    switch(type) {
    case KoShape::SizeChanged:
        scheduleRelayout();
        break;
    default:
        break;
    }
}

bool Layout::isChildLocked(const KoShape *shape) const
{
    return shape->isGeometryProtected();
}

void Layout::childChanged(KoShape *shape, KoShape::ChangeType type)
{
    Q_UNUSED(shape);

    // Do not relayout again if we're currently in the process of a relayout.
    // Repositioning a layout item or resizing it will result in a cull of this method.
    if (m_doingLayout)
        return;

    // This can be fine-tuned, but right now, simply everything will be re-layouted.
    switch (type) {
    //case KoShape::PositionChanged:
    case KoShape::SizeChanged:
        kDebug() << "Child SizeChanged";
        scheduleRelayout();
    // FIXME: There's some cases that would require relayouting but that don't make sense
    // for chart items, e.g. ShearChanged. How should these changes be avoided or handled?
    default:
        break;
    }
}

void Layout::scheduleRelayout()
{
    m_relayoutScheduled = true;
}

void Layout::layout()
{
    //kDebug() << "start";
    Q_ASSERT(!m_doingLayout);

    if (!m_relayoutScheduled)
        return;

    m_doingLayout = true;

    if (m_children.isEmpty()) {
        m_container->setPosition(m_container->position()+m_root->position());
        m_container->setSize(m_root->size());
        m_doingLayout = false;
        m_relayoutScheduled = false;
        return;
    }

    switch (m_structure) {
        case TreeShape::OrgDown:
            kDebug() << "d";
            buildOrgDown();
            break;
        case TreeShape::OrgUp:
            kDebug() << "u";
            buildOrgUp();
            break;
        case TreeShape::OrgLeft:
            kDebug() << "l";
            buildOrgLeft();
            break;
        case TreeShape::OrgRight:
            kDebug() << "r";
            buildOrgRight();
            break;
        default:
            kDebug() << "def";
            buildOrgDown();
            break;
    }

    m_container->update();
    m_doingLayout = false;
    m_relayoutScheduled = false;
    kDebug() << "end";
}

void Layout::buildOrgUp()
{
    qreal fromChildToChild = 5;
    qreal fromParentToChild = 50;

    // calculating size of container
    qreal width=0, height=0, maxHeight=0;
    foreach(KoShape *child, m_children) {
        QSizeF s = child->size();
        width += s.width();
        maxHeight = qMax(s.height(), maxHeight);
    }

    // do not forget about spacing
    width += fromChildToChild*(m_children.count()-1);

    width = qMax(m_root->size().width(), width);
    height = m_root->size().height()+fromParentToChild+maxHeight;
    m_container->setSize(QSizeF(width, height));

    QPointF offset((width-m_root->size().width())/2,
                   fromParentToChild+maxHeight);
    m_root->setPosition(offset);
    offset = QPointF((width-m_lastWidth)/2, height-m_lastHeight);
    m_container->setPosition(m_container->position()-offset);
    m_lastWidth = width;
    m_lastHeight = height;

    qreal x = 0;
    foreach (KoShape *child, m_children) {
        qreal y = height-m_root->size().height()
                    -child->size().height()
                    -fromParentToChild;
        child->setPosition(QPointF(x, y));
        KoConnectionShape *connector = m_bonds[child];
        connector->connectFirst(m_root,0);
        TreeShape *tree = dynamic_cast<TreeShape*>(child);
        connector->connectSecond(tree->root(),2);
        connector->updateConnections();
        x += child->size().width() + fromChildToChild;
    }
}

void Layout::buildOrgDown()
{
    qreal fromChildToChild = 5;
    qreal fromParentToChild = 50;

    // calculating size of container
    qreal width=0, height=0, maxHeight=0;
    foreach(KoShape *child, m_children) {
        QSizeF s = child->size();
        width += s.width();
        maxHeight = qMax(s.height(), maxHeight);
    }

    // do not forget about spacing
    width += fromChildToChild*(m_children.count()-1);

    width = qMax(m_root->size().width(), width);
    height = m_root->size().height()+fromParentToChild+maxHeight;
    m_container->setSize(QSizeF(width, height));

    QPointF offset((width-m_root->size().width())/2, 0);
    m_root->setPosition(offset);
    offset = QPointF((width-m_lastWidth)/2, 0);
    m_container->setPosition(m_container->position()-offset);
    m_lastWidth = width;

    qreal x = 0;
    foreach (KoShape *child, m_children) {
        qreal y = fromParentToChild+m_root->size().height();
        child->setPosition(QPointF(x, y));
        KoConnectionShape *connector = m_bonds[child];
        connector->connectFirst(m_root,2);
        TreeShape *tree = dynamic_cast<TreeShape*>(child);
        connector->connectSecond(tree->root(),0);
        connector->updateConnections();
        x += child->size().width() + fromChildToChild;
    }
}

void Layout::buildOrgLeft()
{
    qreal fromChildToChild = 5;
    qreal fromParentToChild = 50;

    // calculating size of container
    qreal width=0, height=0, maxWidth=0;
    foreach(KoShape *child, m_children) {
        QSizeF s = child->size();
        height += s.height();
        maxWidth = qMax(s.width(), maxWidth);
    }

    // do not forget about spacing
    height += fromChildToChild*(m_children.count()-1);

    height = qMax(m_root->size().height(), height);
    width = m_root->size().width()+fromParentToChild+maxWidth;
    m_container->setSize(QSizeF(width, height));

    QPointF offset(maxWidth+fromParentToChild,
                   (height-m_root->size().height())/2);
    m_root->setPosition(offset);
    offset = QPointF(width-m_lastWidth, (height-m_lastHeight)/2);
    m_container->setPosition(m_container->position()-offset);
    m_lastHeight = height;
    m_lastWidth = width;

    qreal y = 0;
    foreach (KoShape *child, m_children) {
        qreal x = width-m_root->size().width()
                   -child->size().width()
                   -fromParentToChild;
        child->setPosition(QPointF(x, y));
        KoConnectionShape *connector = m_bonds[child];
        connector->connectFirst(m_root,3);
        TreeShape *tree = dynamic_cast<TreeShape*>(child);
        connector->connectSecond(tree->root(),1);
        connector->updateConnections();
        y += child->size().height() + fromChildToChild;
    }
}

void Layout::buildOrgRight()
{
    qreal fromChildToChild = 5;
    qreal fromParentToChild = 50;

    // calculating size of container
    qreal width=0, height=0, maxWidth=0;
    foreach(KoShape *child, m_children) {
        QSizeF s = child->size();
        height += s.height();
        maxWidth = qMax(s.width(), maxWidth);
    }

    // do not forget about spacing
    height += fromChildToChild*(m_children.count()-1);

    height = qMax(m_root->size().height(), height);
    width = m_root->size().width()+fromParentToChild+maxWidth;
    m_container->setSize(QSizeF(width, height));

    QPointF offset(0, (height-m_root->size().height())/2);
    m_root->setPosition(offset);
    offset = QPointF(0, (height-m_lastHeight)/2);
    m_container->setPosition(m_container->position()-offset);
    m_lastHeight = height;

    qreal y = 0;
    foreach (KoShape *child, m_children) {
        qreal x = fromParentToChild+m_root->size().width();
        child->setPosition(QPointF(x, y));
        KoConnectionShape *connector = m_bonds[child];
        connector->connectFirst(m_root,1);
        TreeShape *tree = dynamic_cast<TreeShape*>(child);
        connector->connectSecond(tree->root(),3);
        connector->updateConnections();
        y += child->size().height() + fromChildToChild;
    }
}