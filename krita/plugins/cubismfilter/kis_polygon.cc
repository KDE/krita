/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 * ported from Gimp, Copyright (C) 1997 Eiichi Takamori <taka@ma1.seikyou.ne.jp>
 * original pixelize.c for GIMP 0.54 by Tracy Scott
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <math.h>
#include <qvaluevector.h>

#include <kis_point.h>

#include "kis_polygon.h"

KisPolygon::KisPolygon()
{
        //create an empty KisPoint vector
        //typedef QValueVector<KisPoint> KisPointVector;
        m_data = new KisPointVector(4);
}

void KisPolygon::addPoint(double x, double y)
{
        KisPoint point(x, y);
        m_data->append(point);
}

void KisPolygon::rotate(double theta)
{
        double ct, st, ox, oy;
        
        ct = cos( theta );
        st = sin( theta );
        
        KisPointVector::iterator it;
        for( it = m_data->begin(); it != m_data->end(); ++it )
        {
                ox = (*it).x();
                oy = (*it).y();
                (*it).setX( ct * ox - st * oy );
                (*it).setY( st * ox + ct * oy );
        }
}

void KisPolygon::translate(double tx, double ty)
{
        KisPointVector::iterator it;
        
        for( it = m_data->begin(); it != m_data->end(); ++it )
        {
                (*it).setX( (*it).x() + tx );
                (*it).setY( (*it).y() + ty );
        }
}

void KisPolygon::reset()
{
        m_data->clear();
}

Q_INT32 KisPolygon::extents (double& x1, double& y1, double& x2, double& y2)
{
        if (m_data->empty())
        {
                return 0;     
        }
        x1 = x2 = m_data->front().x();
        y1 = y2 = m_data->front().y();

        KisPointVector::iterator it;
        
        for( it = m_data->begin(); it != m_data->end(); ++it )
        {
                if ((*it).x() < x1)
                {
                        x1 = (*it).x();
                }
                if ((*it).x() > x2)
                {
                        x2 = (*it).x();
                }
                if ((*it).y() < y1)
                {
                        y1 = (*it).y();
                }
                if ((*it).y() > y2)
                {
                        y2 = (*it).y();
                }
        }
        return 1;
}
