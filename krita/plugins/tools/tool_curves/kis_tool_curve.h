/*
 *  kis_tool_curve.h -- part of Krita
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

#ifndef KIS_TOOL_CURVE_H_
#define KIS_TOOL_CURVE_H_

#include <qpen.h>

#include "kis_tool_paint.h"
#include "kis_curve_framework.h"
#include "kis_point.h"

class QRect;

class KisPoint;
class KisPainter;

class KisCurve;

// FIXME These can be passed to draw()
const bool DRAWING = true;
const bool ERASING = false;

class KisToolCurve : public KisToolPaint {

    typedef KisToolPaint super;
    Q_OBJECT

public:
    KisToolCurve(const QString& UIName);
    virtual ~KisToolCurve();

    virtual void update (KisCanvasSubject *subject);

    virtual void buttonPress(KisButtonPressEvent *event);
    virtual void move(KisMoveEvent *event);
    virtual void buttonRelease(KisButtonReleaseEvent *event);
    virtual void doubleClick(KisDoubleClickEvent *event);
    virtual void keyPress(QKeyEvent *event);
    virtual void keyRelease(QKeyEvent *event);

public slots:

    void deactivate();

protected:

    virtual void paint(KisCanvasPainter&);
    virtual void paint(KisCanvasPainter&, const QRect&);

    //
    // KisToolCurve interface
    //
    virtual int convertKeysToOptions(int);
    virtual KisCurve::iterator selectByHandle(const QPoint& pos);
    virtual void draw();
    virtual KisCurve::iterator drawPoint(KisCanvasPainter& gc, KisCurve::iterator point);
    virtual KisCurve::iterator drawPivot(KisCanvasPainter& gc, KisCurve::iterator point);
    virtual void paintCurve();
    virtual KisCurve::iterator paintPoint(KisPainter& painter, KisCurve::iterator point);

    QRect pivotRect (const QPoint&);
    QRect selectedPivotRect (const QPoint&);

protected:

    KisCurve *m_curve;
    KisImageSP m_currentImage;
    
    bool m_dragging;
    KisCurve::iterator m_current;
    KisCurve::iterator m_previous;

    bool m_drawPivots;
    QPen m_drawingPen;
    QPen m_pivotPen;
    QPen m_selectedPivotPen;
    int m_pivotRounding;
    int m_selectedPivotRounding;

    int m_pressedKeys;
};

#endif //__KIS_TOOL_CURVE_H_
