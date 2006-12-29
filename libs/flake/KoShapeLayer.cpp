/* This file is part of the KDE project
   Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>

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

#include "KoShapeLayer.h"

KoShapeLayer::KoShapeLayer()
: KoShapeContainer(new LayerMembers())
{
    setSelectable(false);
}

bool KoShapeLayer::hitTest( const QPointF &position ) const
{
    Q_UNUSED(position);
    return false;
}

QRectF KoShapeLayer::boundingRect() const
{
    QRectF bb(0.0, 0.0, 1.0, 1.0);
    QRectF shapeRect;

    foreach( KoShape* shape, iterator() )
    {
        shapeRect.setTopLeft(shape->position());
        shapeRect.setSize(shape->size());

        bb = bb.unite( shapeRect );
    }

    return bb;
}

QSizeF KoShapeLayer::size() const
{
    return boundingRect().size();
}

QPointF KoShapeLayer::position() const
{
    return QPointF(0.0, 0.0);
}


//  ############# LayerMembers #############
KoShapeLayer::LayerMembers::LayerMembers()
{
}

KoShapeLayer::LayerMembers::~LayerMembers()
{
}

void KoShapeLayer::LayerMembers::add(KoShape *child)
{
    if(m_layerMembers.contains(child))
        return;
    m_layerMembers.append(child);
}

void KoShapeLayer::LayerMembers::remove(KoShape *child)
{
    m_layerMembers.removeAll(child);
}

int KoShapeLayer::LayerMembers::count() const
{
    return m_layerMembers.count();
}

QList<KoShape*> KoShapeLayer::LayerMembers::iterator() const
{
    return QList<KoShape*>(m_layerMembers);
}

void KoShapeLayer::LayerMembers::containerChanged(KoShapeContainer *container)
{
    Q_UNUSED(container);
}

void KoShapeLayer::LayerMembers::setClipping(const KoShape *child, bool clipping)
{
    Q_UNUSED(child);
    Q_UNUSED(clipping);
}

bool KoShapeLayer::LayerMembers::childClipped(const KoShape *child) const
{
    Q_UNUSED(child);
    return false;
}
