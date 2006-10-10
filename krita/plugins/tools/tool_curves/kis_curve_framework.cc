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
 
#include <QRect>
#include <QPointF>

#include <kdebug.h>
#include "kis_curve_framework.h"

using namespace std;

/* **************************** *
 * KisCurve methods definitions *
 * **************************** */

KisCurve::iterator KisCurve::addPivot (KisCurve::iterator it, const QPointF& point)
{
    return iterator(*this,m_curve.insert(it.position(), CurvePoint(point,true,false,NOHINTS)));
}

KisCurve::iterator KisCurve::pushPivot (const QPointF& point)
{
    return selectPivot(iterator(*this,m_curve.insert(m_curve.end(),CurvePoint(point,true,false,NOHINTS))),true);
}

KisCurve::iterator KisCurve::addPoint (KisCurve::iterator it, const QPointF& point, bool pivot, bool selected, int hint)
{
    return iterator(*this,m_curve.insert(it.position(), CurvePoint(point,pivot,selected, hint)));
}

KisCurve::iterator KisCurve::addPoint (KisCurve::iterator it, const CurvePoint& point)
{
    return iterator(*this,m_curve.insert(it.position(), point));
}

KisCurve::iterator KisCurve::pushPoint (const QPointF& point, bool pivot, bool selected,int hint)
{
    return iterator(*this,m_curve.insert(m_curve.end(),CurvePoint(point,pivot,selected,hint)));
}

KisCurve::iterator KisCurve::pushPoint (const CurvePoint& point)
{
    return iterator(*this,m_curve.insert(m_curve.end(),point));
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

KisCurve KisCurve::subCurve(const QPointF& tend)
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

KisCurve KisCurve::subCurve(const QPointF& tstart, const QPointF& tend)
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
    if (!m_curve.empty()) {
        m_curve.erase(m_curve.begin());
        while (m_curve.size() > 1 && !first().isPivot())
            m_curve.erase(m_curve.begin());
    }
}

void KisCurve::deleteLastPivot ()
{
    if (!m_curve.empty()) {
        m_curve.pop_back();
        while (m_curve.size() > 1 && !last().isPivot())
            m_curve.pop_back();
    }
}

KisCurve::iterator KisCurve::deleteCurve (const QPointF& pos1, const QPointF& pos2)
{
    return deleteCurve (CurvePoint(pos1),CurvePoint(pos2));
}

KisCurve::iterator KisCurve::deleteCurve (const CurvePoint& pos1, const CurvePoint& pos2)
{
    return deleteCurve (find(pos1),find(pos2));
}

KisCurve::iterator KisCurve::deleteCurve (KisCurve::iterator pos1, KisCurve::iterator pos2)
{
    if (pos1 == pos2)
        return end();
    iterator pos = pos1;
    pos++;
    while (pos != pos2 && pos != end()) {
        pos = m_curve.erase(pos.position());
    }
    return pos;
}

KisCurve::iterator KisCurve::selectPivot(const QPointF& pt, bool isSelected)
{
    return selectPivot(find(CurvePoint(pt,true)),isSelected);
}

KisCurve::iterator KisCurve::selectPivot(const CurvePoint& pt, bool isSelected)
{
    return selectPivot(find(pt),isSelected);
}

KisCurve::iterator KisCurve::selectPivot(KisCurve::iterator it, bool isSelected)
{
    bool sel = false;
    if (m_standardkeepselected) {
        if (m_actionOptions & KEEPSELECTEDOPTION)
            sel = true;
    }
    KisCurve selected = pivots();
    for (iterator i = selected.begin(); i != selected.end(); i++)
        (*find((*i))).setSelected(sel);
    (*it).setSelected(isSelected);

    return it;
}

KisCurve::iterator KisCurve::movePivot(const QPointF& oldPt, const QPointF& newPt)
{
    return movePivot(CurvePoint(oldPt,true), newPt);
}

KisCurve::iterator KisCurve::movePivot(const CurvePoint& oldPt, const QPointF& newPt)
{
    return movePivot(find(oldPt), newPt);
}

KisCurve::iterator KisCurve::movePivot(KisCurve::iterator it, const QPointF& newPt)
{
    if (!(*it).isPivot())
        return end();

    (*it).setPoint(newPt);

    if ((*it) != first()) {
        deleteCurve (it.previousPivot(), it);
        calculateCurve (it.previousPivot(), it, it);
    }
    if ((*it) != last()) {
        deleteCurve (it, it.nextPivot());
        calculateCurve (it, it.nextPivot(), it.nextPivot());
    }

    return it;
}

void KisCurve::deletePivot (const QPointF& pt)
{
    deletePivot(CurvePoint(pt));
}

void KisCurve::deletePivot (const CurvePoint& pt)
{
    deletePivot(find(pt));
}

void KisCurve::deletePivot (KisCurve::iterator it)
{
    if (!(*it).isPivot())
        return;

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
}

// Probably it can be optimized - it is smooth though.
void KisCurve::moveSelected (const QPointF& trans)
{
    QPointF p;
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

void KisCurve::selectAll(bool sel)
{
    for (iterator i = begin(); i != end(); i = i.nextPivot())
        (*i).setSelected(sel);
}
