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
 
#include <qvaluelist.h>
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
    
inline CurvePoint::CurvePoint (const KisPoint& pt, bool p, bool s)
    : m_pivot(p), m_selected((p) ? s : false)
{
    m_point = pt;
}

inline CurvePoint::CurvePoint (double x, double y, bool p, bool s)
    : m_pivot(p), m_selected((p) ? s : false)
{
    KisPoint tmp(x,y);
    m_point = tmp;
}
/*
inline CurvePoint::CurvePoint (QPoint pt, bool p, bool s)
    : m_pivot(p), m_selected((p) ? s : false)
{
    KisPoint tmp(pt);
    m_point = tmp;
}
    
inline CurvePoint::CurvePoint (KoPoint pt, bool p, bool s)
    : m_pivot(p), m_selected((p) ? s : false)
{
    KisPoint tmp(pt);
    m_point = tmp;
}
*/

inline void CurvePoint::setPoint(const KisPoint& p)
{
    m_point = p;
}

inline void CurvePoint::setPoint(double x, double y)
{
    KisPoint tmp(x,y);
    m_point = tmp;
}
/*
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
*/

/* **************************** *
 * KisCurve methods definitions *
 * **************************** */

CurveIterator KisCurve::addPivot (const CurvePoint& point, CurveIterator it)
{
    CurvePoint p = point;
    p.setPivot(true);
    return addPoint(p,it);
}

CurveIterator KisCurve::addPivot (const KisPoint& point, bool selected, CurveIterator it)
{
    CurvePoint temp (point, true, selected);
    
    return addPoint(temp, it);
}

CurveIterator KisCurve::addPoint (const CurvePoint& point, CurveIterator it)
{
    if (it == 0)
        return m_curve.append (point);
    else
        return m_curve.insert (it, point);
}

CurveIterator KisCurve::addPoint (const KisPoint& point, bool pivot, bool selected, CurveIterator it)
{
    CurvePoint temp (point, pivot, selected);
    return addPoint (temp, it);
}

CurveIterator KisCurve::previousPivot(const KisPoint& pt)
{
    return previousPivot(CurvePoint(pt));
}

CurveIterator KisCurve::previousPivot(const CurvePoint& pt)
{
    return previousPivot(m_curve.find(pt));
}

CurveIterator KisCurve::previousPivot(CurveIterator it)
{
    while (it != m_curve.begin()) {
        it--;
        if ((*it).isPivot())
            return it;
    }

    return m_curve.end();
}

CurveIterator KisCurve::nextPivot(const KisPoint& pt)
{
    return nextPivot(CurvePoint(pt));
}

CurveIterator KisCurve::nextPivot(const CurvePoint& pt)
{
    return nextPivot(m_curve.find(pt));
}

CurveIterator KisCurve::nextPivot(CurveIterator it)
{
    while (it != m_curve.end()) {
        it++;
        if ((*it).isPivot())
            return it;
    }

    return m_curve.end();
}

KisCurve KisCurve::pivots()
{
    KisCurve temp;

    for (CurveIterator it = m_curve.begin(); it != m_curve.end(); it = nextPivot(it))
        temp.addPivot((*it));

    return temp;
}

KisCurve KisCurve::selectedPivots(bool selected)
{
    KisCurve temp;

    for (CurveIterator it = m_curve.begin(); it != m_curve.end(); it = nextPivot(it))
        if ((*it).isSelected() == selected)
            temp.addPivot((*it));

    return temp;
}

void KisCurve::deleteFirstPivot ()
{
    if (!m_curve.isEmpty()) {
        m_curve.pop_front();
        while (m_curve.count() > 1 && !m_curve.first().isPivot())
            m_curve.pop_front();
    }
}

void KisCurve::deleteLastPivot ()
{
    if (!m_curve.isEmpty()) {
        m_curve.pop_back();
        while (m_curve.count() > 1 && !m_curve.last().isPivot())
            m_curve.pop_back();
    }
}

void KisCurve::deleteCurve (const KisPoint& pos1, const KisPoint& pos2)
{
    deleteCurve (CurvePoint(pos1),CurvePoint(pos2));
}

void KisCurve::deleteCurve (const CurvePoint& pos1, const CurvePoint& pos2)
{
    deleteCurve(m_curve.find(pos1),m_curve.find(pos2));
}

void KisCurve::deleteCurve (CurveIterator pos1, CurveIterator pos2)
{
    if (pos1 == pos2)
        return;
    CurveIterator pos = pos1;
    pos++;
    while (pos != pos2 && pos != m_curve.end()) {
        pos = m_curve.erase(pos);
    }
}

void KisCurve::setPivotSelected(const KisPoint& pt, bool isSelected)
{
    setPivotSelected(CurvePoint(pt,true),isSelected);
}

void KisCurve::setPivotSelected(const CurvePoint& pt, bool isSelected)
{
    setPivotSelected(m_curve.find(pt),isSelected);
}

void KisCurve::setPivotSelected(CurveIterator it, bool isSelected)
{
    (*it).setSelected(isSelected);
}

CurveIterator KisCurve::movePivot(const KisPoint& oldPt, const KisPoint& newPt)
{
    return movePivot(CurvePoint(oldPt,true), newPt);
}

CurveIterator KisCurve::movePivot(const CurvePoint& oldPt, const KisPoint& newPt)
{
    return movePivot(m_curve.find(oldPt), newPt);
}

CurveIterator KisCurve::movePivot(CurveIterator it, const KisPoint& newPt)
{
    if (!(*it).isPivot()) {
        kdDebug(0) << "Ma che ci divertiamo?" << endl;
        return m_curve.end();
    }

    CurveIterator start = previousPivot(it);
    CurveIterator end = nextPivot(it);
    CurveIterator newPivot;

    if (m_curve.count() == 1) {
        m_curve.clear();
        newPivot = addPivot(newPt);
    } else if (end == m_curve.end()) {
        deleteLastPivot();
        newPivot = addPivot(newPt);
        calculateCurve(start,newPivot,newPivot);
    } else if (start == m_curve.end()) {
        deleteFirstPivot();
        newPivot = addPivot(newPt,false,end);
        calculateCurve(newPivot,end,end);
    } else {
        deleteCurve(start,end);
        newPivot = addPivot(newPt,false,end);
        calculateCurve(start,newPivot,newPivot);
        calculateCurve(newPivot,end,end);
    }

    return newPivot;
}

bool KisCurve::deletePivot (const KisPoint& pt)
{
    return deletePivot(CurvePoint(pt));
}

bool KisCurve::deletePivot (const CurvePoint& pt)
{
    return deletePivot(m_curve.find(pt));
}

bool KisCurve::deletePivot (CurveIterator it)
{
    if (!(*it).isPivot())
        return false;

    CurveIterator start = previousPivot(it);
    CurveIterator end = nextPivot(it);

    if (end == m_curve.end())
        deleteLastPivot();
    else if (start == m_curve.end())
        deleteFirstPivot();
    else {
        deleteCurve(start,end);
        /* This should add the points before iterator end */
        calculateCurve(start,end,end);
    }

    return true;
}
