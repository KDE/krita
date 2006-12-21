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

#include "KoLayerShape.h"

KoLayerShape::KoLayerShape()
: KoShapeContainer(new LayerMembers())
{
}

bool KoLayerShape::hitTest( const QPointF &position ) const
{
    Q_UNUSED(position);
    return false;
}

QRectF KoLayerShape::boundingRect() const
{
	QRectF bb;

	foreach( KoShape* shape, iterator() )
	{
		if( bb.isEmpty() )
			bb = shape->boundingRect();
		else
			bb = bb.unite( shape->boundingRect() );
	}

	return bb;
}

//  ############# LayerMembers #############
KoLayerShape::LayerMembers::LayerMembers()
{
}

KoLayerShape::LayerMembers::~LayerMembers()
{
}

void KoLayerShape::LayerMembers::add(KoShape *child)
{
    if(m_layerMembers.contains(child))
        return;
    m_layerMembers.append(child);
}

void KoLayerShape::LayerMembers::remove(KoShape *child)
{
    m_layerMembers.removeAll(child);
}

int KoLayerShape::LayerMembers::count() const
{
    return m_layerMembers.count();
}

QList<KoShape*> KoLayerShape::LayerMembers::iterator() const
{
    return QList<KoShape*>(m_layerMembers);
}

void KoLayerShape::LayerMembers::containerChanged(KoShapeContainer *container)
{
    Q_UNUSED(container);
}

void KoLayerShape::LayerMembers::setClipping(const KoShape *child, bool clipping)
{
    Q_UNUSED(child);
    Q_UNUSED(clipping);
}

bool KoLayerShape::LayerMembers::childClipped(const KoShape *child) const
{
    Q_UNUSED(child);
    return false;
}
