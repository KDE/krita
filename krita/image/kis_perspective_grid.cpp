/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <kdebug.h>

#include "kis_perspective_grid.h"

int KisSubPerspectiveGrid::s_lastIndex = 0;

KisSubPerspectiveGrid::KisSubPerspectiveGrid(KisPerspectiveGridNodeSP topLeft, KisPerspectiveGridNodeSP topRight, KisPerspectiveGridNodeSP bottomRight, KisPerspectiveGridNodeSP bottomLeft) : m_topLeft(topLeft), m_topRight(topRight), m_bottomRight(bottomRight), m_bottomLeft(bottomLeft), m_subdivisions(5), m_leftGrid(0), m_rightGrid(0), m_topGrid(0), m_bottomGrid(0), m_index(++s_lastIndex)
{
    
}

bool KisSubPerspectiveGrid::contains(const QPointF p) const
{
    return true;
#if 0
    KisPerspectiveMath::LineEquation d1 = KisPerspectiveMath::computeLineEquation( topLeft().data(), topRight().data() );
//     kdDebug() << p.y() << " " << (p.x() * d1.a + d1.b) << endl;
    if( p.y() >= p.x() * d1.a + d1.b)
    {
        d1 = KisPerspectiveMath::computeLineEquation( topRight().data(), bottomRight().data() );
        kdDebug() << p.y() << " " << (p.x() * d1.a + d1.b) << endl;
        if( p.y() >= p.x() * d1.a + d1.b)
        {
            d1 = KisPerspectiveMath::computeLineEquation( bottomRight().data(), bottomLeft().data() );
            kdDebug() << p.y() << " " << (p.x() * d1.a + d1.b) << endl;
            if( p.y() <= p.x() * d1.a + d1.b)
            {
                d1 = KisPerspectiveMath::computeLineEquation( bottomLeft().data(), topLeft().data() );
                kdDebug() << p.y() << " " << (p.x() * d1.a + d1.b) << endl;
                if( p.y() <= p.x() * d1.a + d1.b)
                {
                    return true;
                }
            }
        }
    }
    return false;
#endif
}


KisPerspectiveGrid::KisPerspectiveGrid()
{
}


KisPerspectiveGrid::~KisPerspectiveGrid()
{
    clearSubGrids( );
}

bool KisPerspectiveGrid::addNewSubGrid( KisSubPerspectiveGrid* ng )
{
    if(hasSubGrids() && !ng->topGrid() && !ng->bottomGrid() && !ng->leftGrid() && !ng->rightGrid() )
    {
        kError() << "sub grids need a neighbourgh if they are not the first grid to be added" << endl;
        return false;
    }
    m_subGrids.push_back(ng);
    return true;
}


void KisPerspectiveGrid::clearSubGrids( )
{
    for( QList<KisSubPerspectiveGrid*>::const_iterator it = begin(); it != end(); ++it)
    {
        delete *it;
    }
    m_subGrids.clear();
}

KisSubPerspectiveGrid* KisPerspectiveGrid::gridAt(QPointF p)
{
    for( QList<KisSubPerspectiveGrid*>::const_iterator it = begin(); it != end(); ++it)
    {
        if( (*it)->contains(p) )
        {
            return *it;
        }
    }
    return 0;
}

