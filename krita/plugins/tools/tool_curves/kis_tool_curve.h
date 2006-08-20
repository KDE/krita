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

#include "kis_selection.h"
#include "kis_tool_paint.h"
#include "kis_canvas_subject.h"
#include "kis_point.h"

#include "kis_curve_framework.h"

class QRect;
class KisPainter;
class KisSelectionOptions;

typedef QPair<KisCurve::iterator,bool> PointPair;

const double MAXDISTANCE = 2.5;
double pointToSegmentDistance(const KisPoint& p, const KisPoint& l0, const KisPoint& l1);

class KisToolCurve : public KisToolPaint {

    typedef KisToolPaint super;
    Q_OBJECT

public:
    KisToolCurve(const QString& UIName);
    virtual ~KisToolCurve();

    virtual void update (KisCanvasSubject *subject);
    virtual QWidget* createOptionWidget(QWidget* parent);

    virtual void buttonPress(KisButtonPressEvent *event);
    virtual void move(KisMoveEvent *event);
    virtual void buttonRelease(KisButtonReleaseEvent *event);
    virtual void doubleClick(KisDoubleClickEvent *event);
    virtual void keyPress(QKeyEvent *event);
    virtual void keyRelease(QKeyEvent *event);

public slots:

    virtual void deactivate();

protected:

    virtual void paint(KisCanvasPainter&);
    virtual void paint(KisCanvasPainter&, const QRect&);

    /* ********************** *
     * KisToolCurve interface *
     * ********************** */

    /*
     * This keep in sync the options of the tool with the options of the curve
     */
    virtual int updateOptions(int);

    virtual PointPair pointUnderMouse(const QPoint& pos);
    virtual KisCurve::iterator handleUnderMouse(const QPoint& pos);

    /*
     * Select the needed points; called after pointUnderMouse
     */
    virtual KisCurve::iterator selectByMouse(KisCurve::iterator it);

    /*
     * draw() initializes the KisCanvasPainter and then loop on the points of the curve for drawing them.
     */
    virtual void draw(bool = true, bool = false);
    virtual void draw(KisCurve::iterator inf, bool = false, bool = true);

    /*
     * Used by draw() to draw the current point of the curve. Can draw more than one point and then returns the last one
     */
    virtual KisCurve::iterator drawPoint(KisCanvasPainter& gc, KisCurve::iterator point);

    /*
     * Used by draw(), if a point is a pivot, this draw the handle around it (if m_drawPivots is set to true)
     */
    virtual void drawPivotHandle(KisCanvasPainter& gc, KisCurve::iterator point);

    /*
     * Methods for commiting the curve
     */

    /*
     * Called by selectCurve(), this convert m_curve to a vector of KisPoint in order to be used by paintPolygon()
     */
    virtual QValueVector<KisPoint> convertCurve();

    /*
     * Called by paintCurve(), it behaves essentially like drawPoint(), but this uses a KisPainter
     */
    virtual KisCurve::iterator paintPoint(KisPainter&, KisCurve::iterator);

    /*
     * Finish the curve: if the tool is a TOOL_SHAPE or TOOL_FREEHAND, calls paintCurve(), if it's a TOOL_SELECT, then selectCurve()
     */
    virtual void commitCurve();

    /*
     * Used by commitCurve() if the tool is a painting tool
     */
    virtual void paintCurve();

    /*
     * Used by commitCurve() if the tool is a selection tool
     */
    virtual void selectCurve();

    /*
     * Return the rect around a given point, assuming that that point is an unselected pivot
     */
    QRect pivotRect (const QPoint&);

    /*
     * Same as above for selected pivots
     */
    QRect selectedPivotRect (const QPoint&);

protected:

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
    bool m_supportMinimalDraw;

    QString m_transactionMessage;

private:

    QString m_UIName;


/* ********************************** *
 * Selection Tools specific functions *
 * ********************************** */

public:

    /*
     * This initializes our Option Widget (called by createOptionWidget())
     */
    virtual QWidget* createSelectionOptionWidget(QWidget* parent);
    
    /*
     * This return our internal KisSelectionOptions if toolType() returns TOOL_SELECT
     */
    virtual QWidget* optionWidget();

public slots:

    /*
     * Slot for createSelectionOptionWidget()
     */
    virtual void slotSetAction(int);

private:

    /*
     * Members used by slotSetAction() and selectCurve()
     */
    KisSelectionOptions* m_optWidget;
    enumSelectionMode m_selectAction;
};

#endif //__KIS_TOOL_CURVE_H_
