/*
 *  Copyright (c) 2005 C. Boemann <cbo@boemann.dk>
 *  Copyright (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
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
#ifndef KIS_CURVE_WIDGET_H
#define KIS_CURVE_WIDGET_H

// Qt includes.

#include <QWidget>
#include <QColor>
#include <QPointF>
#include <QPixmap>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QPaintEvent>
#include <QList>

#include <kritaui_export.h>

class QSpinBox;
class KisCubicCurve;

/**
 * KisCurveWidget is a widget that shows a single curve that can be edited
 * by the user. The user can grab the curve and move it; this creates
 * a new control point. Control points can be deleted by selecting a point
 * and pressing the delete key.
 *
 * (From: http://techbase.kde.org/Projects/Widgets_and_Classes#KisCurveWidget)
 * KisCurveWidget allows editing of spline based y=f(x) curves. Handy for cases
 * where you want the user to control such things as tablet pressure
 * response, color transformations, acceleration by time, aeroplane lift
 *by angle of attack.
 */
class KRITAUI_EXPORT KisCurveWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool pointSelected READ pointSelected NOTIFY pointSelectedChanged);

public:
    friend class CurveEditorItem;
    /**
     * Create a new curve widget with a default curve, that is a straight
     * line from bottom-left to top-right.
     */
    KisCurveWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);

    ~KisCurveWidget() override;

    /**
     * Reset the curve to the default shape
     */
    void reset(void);

    /**
     * Enable the guide and set the guide color to the specified color.
     *
     * XXX: it seems that the guide feature isn't actually implemented yet?
     */
    void setCurveGuide(const QColor & color);


    /**
     * Set a background pixmap. The background pixmap will be drawn under
     * the grid and the curve.
     *
     * XXX: or is the pixmap what is drawn to the  left and bottom of the curve
     * itself?
     */
    void setPixmap(const QPixmap & pix);
    QPixmap getPixmap();
    
    void setBasePixmap(const QPixmap & pix);
    QPixmap getBasePixmap();

    /**
     * Whether or not there is a point selected
     * This does NOT include the first and last points
     */
    bool pointSelected() const;

Q_SIGNALS:

    /**
     * Emitted whenever a control point has changed position.
     */
    void modified(void);
    /**
     * Emitted whenever the status of whether a control point is selected or not changes
     */
    void pointSelectedChanged();

protected Q_SLOTS:
    void inOutChanged(int);


protected:

    void keyPressEvent(QKeyEvent *) override;
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent * e) override;
    void mouseReleaseEvent(QMouseEvent * e) override;
    void mouseMoveEvent(QMouseEvent * e) override;
    void leaveEvent(QEvent *) override;
    void resizeEvent(QResizeEvent *e) override;

public:

    /**
     * @return get a list with all defined points. If you want to know what the
     * y value for a given x is on the curve defined by these points, use getCurveValue().
     * @see getCurveValue
     */
    KisCubicCurve curve();

    /**
     * Replace the current curve with a curve specified by the curve defined by the control
     * points in @p inlist.
     */
    void setCurve(KisCubicCurve inlist);

    /**
     * Connect/disconnect external spinboxes to the curve
     * @p inMin / @p inMax - is the range for input values
     * @p outMin / @p outMax - is the range for output values
     */
    void setupInOutControls(QSpinBox *in, QSpinBox *out, int inMin, int inMax, int outMin, int outMax);
    void dropInOutControls();

    /**
     * Handy function that creates new point in the middle
     * of the curve and sets focus on the @p m_intIn field,
     * so the user can move this point anywhere in a moment
     */
    void addPointInTheMiddle();

private:

    class Private;
    Private * const d;

};


#endif /* KIS_CURVE_WIDGET_H */
