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
    : m_pivot(0), m_selected(0)
{

}
    
inline CurvePoint::CurvePoint (KisPoint &pt, bool p = false, bool s = false)
    : m_pivot(p), m_selected((p) ? s : false)
{
    m_point = pt;
}

inline CurvePoint::CurvePoint (double x, double y, bool p = false, bool s = false)
    : m_pivot(p), m_selected((p) ? s : false)
{
    KisPoint tmp(x,y);
    m_point = tmp;
}

inline CurvePoint::CurvePoint (QPoint& pt, bool p = false, bool s = false)
    : m_pivot(p), m_selected((p) ? s : false)
{
    KisPoint tmp(pt);
    m_point = tmp;
}
    
inline CurvePoint::CurvePoint (KoPoint& pt, bool p = false, bool s = false)
    : m_pivot(p), m_selected((p) ? s : false)
{
    KisPoint tmp(pt);
    m_point = tmp;
}


inline void CurvePoint::setPoint(KisPoint p)
{
    m_point = p;
}

inline void CurvePoint::setPoint(double x, double y)
{
    KisPoint tmp(x,y);
    m_point = tmp;
}

inline void CurvePoint::setPoint(QPoint &pt)
{
    KisPoint tmp(pt);
    m_point = tmp;
}

inline void CurvePoint::setPoint(KoPoint &pt)
{
    KisPoint tmp(pt);
    m_point = tmp;
}


/* **************************** *
 * KisCurve methods definitions *
 * **************************** */

void KisCurve::addPivot (CurvePoint point, int index)
{
    point.setPivot(true);
    add(point,index);
}

void KisCurve::addPivot (KisPoint point, bool selected, int index)
{
    add(point, true, selected, index);
}

void KisCurve::add (CurvePoint point, int index)
{
    if (index < 0) {
        m_curve.append (point);
    } else {
        PointList::iterator it = &m_curve.at(index);
        m_curve.insert (it, point);
    }
}

void KisCurve::add (KisPoint point, bool pivot, bool selected, int index) {
    CurvePoint temp (point, pivot, selected);

    add (temp, index);
}

void KisCurve::setPivot (CurvePoint pivot) {
    PointList::iterator it = qFind(m_curve.begin(),m_curve.end(),pivot);
    if (it != m_curve.end())
        it->setPivot(true);
}

void KisCurve::setPivot (int index) {
    PointList::iterator it = &m_curve.at(index);
    if (it)
        it->setPivot(true);
}

void KisCurve::deleteLastPivot () {
    if (!m_curve.isEmpty()) {
        m_curve.pop_back();
        while (m_curve.count() > 1 && !m_curve.last().isPivot())
            m_curve.pop_back();
    }
}
