/*
 *  Copyright (c) 2006 Emanuele Tamponi <emanuele@valinor.it>
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
 
/* Initial Commit for the Curves Framework. E.T. */

#include <qvaluevector.h>
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_point.h"

#include "kis_curves_framework.h"

/* ****************************** *
 * CurvePoint methods definitions *
 * ****************************** */

inline CurvePoint::CurvePoint ()
    : m_point(0), m_pivot(0), m_selected(0)
{

}
    
inline CurvePoint::CurvePoint (KisPoint &pt, bool p = false, bool s = false)
    : m_pivot(p), m_selected((p) ? s : false)
{
    m_point = new KisPoint;
    *m_point = pt;
}

inline CurvePoint::CurvePoint (double x, double y, bool p = false, bool s = false)
    : m_pivot(p), m_selected((p) ? s : false)
{
    m_point = new KisPoint(x,y);
}

inline CurvePoint::CurvePoint (QPoint& pt, bool p = false, bool s = false)
    : m_pivot(p), m_selected((p) ? s : false)
{
    m_point = new KisPoint(pt);
}
    
inline CurvePoint::CurvePoint (KoPoint& pt, bool p = false, bool s = false)
    : m_pivot(p), m_selected((p) ? s : false)
{
    m_point = new KisPoint(pt);
}


inline bool CurvePoint::setPoint(KisPoint p)
{
    if (!m_point)
        m_point = new KisPoint();
    *m_point = p;
    
    return ((m_point) ? true : false);
}

inline bool CurvePoint::setPoint(double x, double y)
{
    if (m_point)
        delete m_point;
    m_point = new KisPoint(x,y);
    
    return ((m_point) ? true : false);
}

inline bool CurvePoint::setPoint(QPoint &pt)
{
    if (m_point)
        delete m_point;
    m_point = new KisPoint(pt);
    
    return ((m_point) ? true : false);
}

inline bool CurvePoint::setPoint(KoPoint &pt)
{
    if (m_point)
        delete m_point;
    m_point = new KisPoint(pt);
    
    return ((m_point) ? true : false);
}


/* **************************** *
 * KisCurve methods definitions *
 * **************************** */


int KisCurve::add (CurvePoint point, int index = -1)
{
    if (index < 0) {
        m_curve.append (point);
        return m_curve.count()-1;
    } else {
        PointList::iterator it = &m_curve.at(index);
        m_curve.insert (it, point);
        return index;
    } 
}

int KisCurve::add (KisPoint point, bool pivot, bool selection, int index = -1) {
    CurvePoint temp (point, pivot, selection);

    return add (temp, index);
}

bool KisCurve::setPivot (CurvePoint pivot) {
    PointList::iterator it = qFind(m_curve.begin(),m_curve.end(),pivot);
    if (it != m_curve.end())
        it->setPivot(true);
    else
        return false;

    return true;
}

bool KisCurve::setPivot (int index) {
    PointList::iterator it = &m_curve.at(index);
    if (it)
        it->setPivot(true);
    else
        return false;

    return true;
}

bool KisCurve::deleteLastPivot () {
    m_curve.erase(m_curve.end());
    for (PointList::iterator it = m_curve.end();(!it->isPivot());it--)
        m_curve.erase(it);
    return true;
}
