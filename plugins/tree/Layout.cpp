/* This file is part of the KDE project

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
#include "Tree.h"

#include "KoShape.h"
#include <KoShapeContainer.h>
#include "KoConnectionShape.h"
#include "kdebug.h"

Layout::Layout(KoShapeContainer *container)
    : m_container(container)
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
    kDebug() << "children: " << m_children.contains(shape);
    Q_ASSERT(!m_children.contains(shape));
    KoConnectionShape *connector = dynamic_cast<KoConnectionShape*>(shape);
    if (connector){
        kDebug() << "connectors: " << m_connectors.contains(connector);
        Q_ASSERT(!m_connectors.contains(connector));
        m_connectors.append(shape);
    }
    else m_children.append(shape);
    scheduleRelayout();
}

void Layout::add(KoShape *shape, uint pos)
{
    Q_ASSERT(!m_children.contains(shape));
    m_children.insert(pos,shape);
    scheduleRelayout();
}

void Layout::attachConnector(KoShape* shape, KoConnectionShape *connector)
{
    //Q_ASSERT(m_children.contains(shape));
    //Q_ASSERT(m_connectors.contains(connector));
    m_bonds[shape] = connector;
    //scheduleRelayout();
}

void Layout::setRoot(KoShape *shape)
{
    m_root = shape;
    m_container->setSize(m_root->size());
    m_lastWidth = m_container->size().width();

    // shape was added to children by setParent() method
    m_children.removeOne(m_root);
}

KoShape* Layout::root() const
{
    return m_root;
}

void Layout::remove(KoShape *shape)
{
    if (m_children.removeOne(shape)) {
        scheduleRelayout();
    }
    else if (m_connectors.removeOne(shape)) {
        scheduleRelayout();
    }
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
    kDebug() << "";
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

void Layout::setPosition(const KoShape *shape, uint pos)
{
    int index =  m_children.indexOf(const_cast<KoShape*>(shape));
    if (index != -1) {
        m_children.swap(index, pos);
        scheduleRelayout();
    }
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
    case KoShape::PositionChanged:
    case KoShape::SizeChanged:
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
    kDebug() << "start";
    Q_ASSERT(!m_doingLayout);

    if (!m_relayoutScheduled)
        return;

    m_doingLayout = true;

    if (m_children.isEmpty()){
        m_container->setPosition(m_container->position()+m_root->position());
        m_container->setSize(m_root->size());
        m_doingLayout = false;
        m_relayoutScheduled = false;
        return;
    }
    qreal width=0, maxHeight=0;
    foreach(KoShape *child, m_children){
        QSizeF s = child->size();
        width += s.width();
        if (s.height()>maxHeight)
            maxHeight = s.height();
    }
    kDebug() << width;
    //spacing
    width += 5*(m_children.count()-1);
    if (m_root->size().width()>width)
        width = m_root->size().width();

    QSizeF s = m_container->size();
    s.setHeight(m_root->size().height()+70+maxHeight);
    s.setWidth(width);
    m_container->setSize(s);

    QPointF offset((width-m_root->size().width())/2, 0);
    m_root->setPosition(offset);
    offset = QPointF((width-m_lastWidth)/2, 0);
    m_container->setPosition(m_container->position()-offset);
    m_lastWidth = width;

    qreal x = 0;
    foreach (KoShape *child, m_children){
        child->setPosition(QPointF(x, 70+m_root->size().height()));
        KoConnectionShape *connector = m_bonds[child];
        connector->connectFirst(m_root,2);
        connector->connectSecond(child,0);
        connector->updateConnections();
        x += child->size().width() + 5;
    }

    m_doingLayout = false;
    m_relayoutScheduled = false;
    kDebug() << "end";
}
