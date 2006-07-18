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
/*
KisCurve::iterator KisCurve::addPivot (KisCurve::iterator it, const CurvePoint& point)
{
    return iterator(*this,m_curve.insert(it.position(), CurvePoint(point.point(),true,point.isSelected(),point.hint())));
}
*/
KisCurve::iterator KisCurve::addPivot (KisCurve::iterator it, const KisPoint& point)
{
    return iterator(*this,m_curve.insert(it.position(), CurvePoint(point,true,true,NOHINTS)));
}

KisCurve::iterator KisCurve::addPoint (KisCurve::iterator it, const KisPoint& point, bool pivot, bool selected, int hint)
{
    return iterator(*this,m_curve.insert(it.position(), CurvePoint(point,pivot,selected, hint)));
}

KisCurve::iterator KisCurve::addPoint (KisCurve::iterator it, const CurvePoint& point)
{
    return iterator(*this,m_curve.insert(it.position(), point));
}
/*
KisCurve::iterator KisCurve::pushPivot (const CurvePoint& point)
{
    return iterator(*this,m_curve.append(CurvePoint(point.point(),true,point.isSelected(),point.hint())));
}
*/
KisCurve::iterator KisCurve::pushPivot (const KisPoint& point)
{
    return selectPivot(iterator(*this,m_curve.append(CurvePoint(point,true,false,NOHINTS))), true);
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
        temp.pushPoint((*it));

    return temp;
}

KisCurve KisCurve::selectedPivots(bool selected)
{
    KisCurve temp;

    for (iterator it = begin(); it != end(); it = it.nextPivot())
        if ((*it).isSelected() == selected)
            temp.pushPoint((*it));

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
    iterator pos = pos1;
    pos++;
    while (pos != pos2 && pos != end()) {
        pos = m_curve.erase(pos.position());
    }
}

KisCurve::iterator KisCurve::selectPivot(const KisPoint& pt, bool isSelected)
{
    return selectPivot(find(CurvePoint(pt,true)),isSelected);
}

KisCurve::iterator KisCurve::selectPivot(const CurvePoint& pt, bool isSelected)
{
    return selectPivot(find(pt),isSelected);
}

KisCurve::iterator KisCurve::selectPivot(KisCurve::iterator it, bool isSelected)
{
    if (m_actionOptions & KEEPSELECTEDOPTION) {
        if ((*it).isSelected())
            (*it).setSelected(false);
        else
            (*it).setSelected(true);
    } else {
        KisCurve selected = selectedPivots();
        for (iterator i = selected.begin(); i != selected.end(); i++)
            (*find((*i))).setSelected(false);
        (*it).setSelected(isSelected);
    }

    return it;
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

    (*it).setPoint(newPt);

    if ((*it) != first()) {
        deleteCurve (it.previousPivot(), it);
        calculateCurve (it.previousPivot(), it, it);
    }
    if ((*it) != last()) {
        deleteCurve (it, it.nextPivot());
        calculateCurve (it, it.nextPivot(), it);
    }

    return it;
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

    iterator start = it.previousPivot();
    iterator end = it.nextPivot();

    if (end == m_curve.end())
        deleteLastPivot();
    else if (start == it)
        deleteFirstPivot();
    else {
        deleteCurve(start,end);
        calculateCurve(start,end,end);
    }

    return true;
}

// Probably it can be optimized - it's is smooth however.
void KisCurve::moveSelected (const KisPoint& trans)
{
    KisPoint p;
    KisCurve sel = selectedPivots();

    for (iterator it = sel.begin(); it != sel.end(); it++) {
        p = (*it).point() + trans;
        movePivot((*it),p);
    }
}

void KisCurve::deleteSelected ()
{
    KisCurve sel = selectedPivots();
    for (iterator it = sel.begin(); it != sel.end(); it++)
        deletePivot((*it));
}
