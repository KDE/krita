/*
 *  kis_tool_bezier.h -- part of Krita
 *
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

#ifndef KIS_TOOL_BEZIER_H_
#define KIS_TOOL_BEZIER_H_

#include <QPointF>

#include "kis_tool_factory.h"
#include "kis_tool_curve.h"

class CurvePoint;
class QPointF;
class KisCanvas;
class KisCurve;
class KisPainter;

const int BEZIERENDHINT = 0x0010;
const int BEZIERPREVCONTROLHINT = 0x0020;
const int BEZIERNEXTCONTROLHINT = 0x0040;

const int SYMMETRICALCONTROLSOPTION = ALTOPTION;
const int PREFERCONTROLSOPTION = SHIFTOPTION;

class KisCurveBezier : public KisCurve {

    typedef KisCurve super;

    void recursiveCurve (const QPointF& P1, const QPointF& P2, const QPointF& P3,
                         const QPointF& P4, int level, iterator it);
    QPointF midpoint (const QPointF&, const QPointF&);

    int m_maxLevel;
    
public:

    KisCurveBezier() : super() {m_maxLevel = 5;}

    ~KisCurveBezier() {}

    virtual void calculateCurve(iterator, iterator, iterator);
    virtual iterator pushPivot(const QPointF&);
    virtual iterator movePivot(iterator, const QPointF&);
    virtual void deletePivot(iterator);

public:

    iterator groupEndpoint (iterator) const;
    iterator groupPrevControl (iterator) const;
    iterator groupNextControl (iterator) const;

    bool groupSelected (iterator) const;

    iterator nextGroupEndpoint (iterator) const;
    iterator prevGroupEndpoint (iterator) const;

};

class KisToolBezier : public KisToolCurve {

    typedef KisToolCurve super;
    Q_OBJECT

public:
    KisToolBezier(const QString&);
    virtual ~KisToolBezier();

protected:

    virtual KisCurve::iterator handleUnderMouse(const QPoint& pos);
    virtual void drawPivotHandle(QPainter& gc, KisCurve::iterator point);
    virtual KisCurve::iterator drawPoint(QPainter& gc, KisCurve::iterator point);

protected:

    KisCurveBezier *m_derivated;

};

#endif //__KIS_TOOL_BEZIER_H__
