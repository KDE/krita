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
#include <qcursor.h>

#include "kis_tool.h"
#include "kis_canvas_subject.h"
#include "kis_point.h"

#include "kis_curve_framework.h"

class QRect;
class KisPainter;

const double MAXDISTANCE = 1.2; 

class KisToolCurve : public KisTool {

    typedef KisTool super;
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

    virtual QCursor cursor();
    virtual void setCursor(const QCursor&);
    

public slots:

    virtual void activate();
    virtual void deactivate();

protected:

    virtual void paint(KisCanvasPainter&);
    virtual void paint(KisCanvasPainter&, const QRect&);

    //
    // KisToolCurve interface
    //
    virtual int updateOptions(int);

    virtual KisCurve::iterator selectByMouse(const QPoint& pos);
    virtual KisCurve::iterator selectByHandle(const QPoint& pos);

    virtual void draw();
    virtual KisCurve::iterator drawPoint(KisCanvasPainter& gc, KisCurve::iterator point);
    virtual void drawPivotHandle(KisCanvasPainter& gc, KisCurve::iterator point);
    
    virtual void paintCurve() = 0;

    QRect pivotRect (const QPoint&);
    QRect selectedPivotRect (const QPoint&);

protected:

    KisCanvasSubject *m_subject;
    KisImageSP m_currentImage;

    KisCurve *m_curve;
    KisCurve::iterator m_current;
    KisCurve::iterator m_previous;
    KisPoint m_currentPoint;
    
    bool m_dragging;
    bool m_drawPivots;
    QPen m_drawingPen;
    QPen m_pivotPen;
    QPen m_selectedPivotPen;
    int m_pivotRounding;
    int m_selectedPivotRounding;

    int m_actionOptions;

    QString m_transactionMessage;

private:

    QCursor m_cursor;
};

#endif //__KIS_TOOL_CURVE_H_
