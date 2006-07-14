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

#include "kis_curve_framework.h"

/* **************************** *
 * KisCurve methods definitions *
 * **************************** */

KisCurve::iterator KisCurve::addPivot (KisCurve::iterator it, const CurvePoint& point)
{
    return iterator(*this,m_curve.insert(it.position(), CurvePoint(point.point(),true,point.isSelected(),point.hint())));
}

KisCurve::iterator KisCurve::addPivot (KisCurve::iterator it, const KisPoint& point, bool selected, int hint)
{
    return iterator(*this,m_curve.insert(it.position(), CurvePoint(point,true,selected,hint)));
}

KisCurve::iterator KisCurve::addPoint (KisCurve::iterator it, const KisPoint& point, bool pivot, bool selected, int hint)
{
    return iterator(*this,m_curve.insert(it.position(), CurvePoint(point,pivot,selected, hint)));
}

KisCurve::iterator KisCurve::addPoint (KisCurve::iterator it, const CurvePoint& point)
{
    return iterator(*this,m_curve.insert(it.position(), point));
}

KisCurve::iterator KisCurve::pushPivot (const CurvePoint& point)
{
    return iterator(*this,m_curve.append(CurvePoint(point.point(),true,point.isSelected(),point.hint())));
}

KisCurve::iterator KisCurve::pushPivot (const KisPoint& point, bool selected, int hint)
{
    return iterator(*this,m_curve.append(CurvePoint(point,true,selected,hint)));
}

KisCurve::iterator KisCurve::pushPoint (const KisPoint& point, bool pivot, bool selected,int hint)
{
    return iterator(*this,m_curve.append(CurvePoint(point,pivot,selected,hint)));
}

KisCurve::iterator KisCurve::pushPoint (const CurvePoint& point)
{
    return iterator(*this,m_curve.append(point));
}

KisCurve KisCurve::pivots()
{
    KisCurve temp;

    for (iterator it = begin(); it != end(); it = it.nextPivot())
        temp.pushPivot((*it));

    return temp;
}

KisCurve KisCurve::selectedPivots(bool selected)
{
    KisCurve temp;

    for (iterator it = begin(); it != end(); it = it.nextPivot())
        if ((*it).isSelected() == selected)
            temp.pushPivot((*it));

    return temp;
}

KisCurve KisCurve::subCurve(const KisPoint& tend)
{
    return subCurve(find(tend).previousPivot(),find(tend));
}

KisCurve KisCurve::subCurve(const CurvePoint& tend)
{
    return subCurve(find(tend).previousPivot(),find(tend));
}

KisCurve KisCurve::subCurve(iterator tend)
{
    return subCurve(tend.previousPivot(),tend);
}

KisCurve KisCurve::subCurve(const KisPoint& tstart, const KisPoint& tend)
{
    return subCurve(find(tstart),find(tend));
}

KisCurve KisCurve::subCurve(const CurvePoint& tstart, const CurvePoint& tend)
{
    return subCurve(find(tstart),find(tend));
}

KisCurve KisCurve::subCurve(iterator tstart, iterator tend)
{
    KisCurve temp;

    while (tstart != tend && tstart != m_curve.end())
        temp.pushPoint((*++tstart));

    return temp;
}

void KisCurve::deleteFirstPivot ()
{
    if (!m_curve.isEmpty()) {
        m_curve.pop_front();
        while (m_curve.count() > 1 && !first().isPivot())
            m_curve.pop_front();
    }
}

void KisCurve::deleteLastPivot ()
{
    if (!m_curve.isEmpty()) {
        m_curve.pop_back();
        while (m_curve.count() > 1 && !last().isPivot())
            m_curve.pop_back();
    }
}

void KisCurve::deleteCurve (const KisPoint& pos1, const KisPoint& pos2)
{
    deleteCurve (CurvePoint(pos1),CurvePoint(pos2));
}

void KisCurve::deleteCurve (const CurvePoint& pos1, const CurvePoint& pos2)
{
    deleteCurve(find(pos1),find(pos2));
}

void KisCurve::deleteCurve (KisCurve::iterator pos1, KisCurve::iterator pos2)
{
    if (pos1 == pos2)
        return;
    KisCurve::iterator pos = pos1;
    pos++;
    while (pos != pos2 && pos != end()) {
        pos = m_curve.erase(pos.position());
    }
}

void KisCurve::selectPivot(const KisPoint& pt, bool isSelected)
{
    selectPivot(CurvePoint(pt,true),isSelected);
}

void KisCurve::selectPivot(const CurvePoint& pt, bool isSelected)
{
    selectPivot(find(pt),isSelected);
}

void KisCurve::selectPivot(KisCurve::iterator it, bool isSelected)
{
    (*it).setSelected(isSelected);
}

KisCurve::iterator KisCurve::movePivot(const KisPoint& oldPt, const KisPoint& newPt)
{
    return movePivot(CurvePoint(oldPt,true), newPt);
}

KisCurve::iterator KisCurve::movePivot(const CurvePoint& oldPt, const KisPoint& newPt)
{
    return movePivot(find(oldPt), newPt);
}

KisCurve::iterator KisCurve::movePivot(KisCurve::iterator it, const KisPoint& newPt)
{
    if (!(*it).isPivot()) {
        kdDebug(0) << "Ma che ci divertiamo?" << endl;
        return end();
    }

    int hint = (*it).hint();
    KisCurve::iterator start = it.previousPivot();
    KisCurve::iterator end = it.nextPivot();
    KisCurve::iterator newPivot(*this);

    if (m_curve.count() == 1) {
        m_curve.clear();
        newPivot = pushPivot(newPt,false,hint);
    } else if (end == m_curve.end()) {
        deleteLastPivot();
        newPivot = pushPivot(newPt,false,hint);
        calculateCurve(start,newPivot,newPivot);
    } else if (start == it) {
        deleteFirstPivot();
        newPivot = addPivot(end,newPt,false,hint);
        calculateCurve(newPivot,end,end);
    } else {
        deleteCurve(start,end);
        newPivot = addPivot(end,newPt,false,hint);
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
    return deletePivot(find(pt));
}

bool KisCurve::deletePivot (KisCurve::iterator it)
{
    if (!(*it).isPivot())
        return false;

    KisCurve::iterator start = it.previousPivot();
    KisCurve::iterator end = it.nextPivot();

    if (end == m_curve.end())
        deleteLastPivot();
    else if (start == it)
        deleteFirstPivot();
    else {
        deleteCurve(start,end);
        /* This should add the points before iterator end */
        calculateCurve(start,end,end);
    }

    return true;
}
