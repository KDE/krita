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
#ifndef _KIS_CURVE_WIDGET_P_H_
#define _KIS_CURVE_WIDGET_P_H_
#include <kis_cubic_curve.h>
#include <QApplication>
#include <QPalette>
#include <KisSpinBoxSplineUnitConverter.h>

enum enumState {
    ST_NORMAL,
    ST_DRAG
};

/**
 * Private members for KisCurveWidget class
 */
class Q_DECL_HIDDEN KisCurveWidget::Private
{

    KisCurveWidget *m_curveWidget;
    KisSpinBoxSplineUnitConverter unitConverter;


public:
    Private(KisCurveWidget *parent);

    /* Dragging variables */
    int m_grab_point_index;
    double m_grabOffsetX;
    double m_grabOffsetY;
    double m_grabOriginalX;
    double m_grabOriginalY;
    QPointF m_draggedAwayPoint;
    int m_draggedAwayPointIndex;

    bool m_readOnlyMode;
    bool m_guideVisible;
    QColor m_colorGuide;


    /* The curve itself */
    bool    m_splineDirty;
    KisCubicCurve m_curve;

    QPixmap m_pix;
    QPixmap m_pixmapBase;
    bool m_pixmapDirty;
    QPixmap *m_pixmapCache;

    /* In/Out controls */
    QSpinBox *m_intIn;
    QSpinBox *m_intOut;

    /* Working range of them */
    int m_inMin;
    int m_inMax;
    int m_outMin;
    int m_outMax;

    /**
     * State functions.
     * At the moment used only for dragging.
     */
    enumState m_state;

    inline void setState(enumState st);
    inline enumState state() const;

    /**
     * Compresses the modified() signals
     */
    KisThreadSafeSignalCompressor m_modifiedSignalsCompressor;


    /*** Internal routines ***/

    /**
     * Common update routines
     */
    void setCurveModified(bool rewriteSpinBoxesValues);
    void setCurveRepaint();


    /**
     * Convert working range of
     * In/Out controls to normalized
     * range of spline (and reverse)
     * See notes on KisSpinBoxSplineUnitConverter
     */
    double io2sp(int x, int min, int max);
    int sp2io(double x, int min, int max);


    /**
     * Check whether newly created/moved point @p pt doesn't overlap
     * with any of existing ones from @p m_points and adjusts its coordinates.
     * @p skipIndex is the index of the point, that shouldn't be taken
     * into account during the search
     * (e.g. because it's @p pt itself)
     *
     * Returns false in case the point can't be placed anywhere
     * without overlapping
     */
    bool jumpOverExistingPoints(QPointF &pt, int skipIndex);


    /**
     * Synchronize In/Out spinboxes with the curve
     */
    void syncIOControls();

    /**
     * Find the nearest point to @p pt from m_points
     */
    int nearestPointInRange(QPointF pt, int wWidth, int wHeight) const;

    /**
     * Nothing to be said! =)
     */
    inline
    void drawGrid(QPainter &p, int wWidth, int wHeight);

};

KisCurveWidget::Private::Private(KisCurveWidget *parent)
    : m_modifiedSignalsCompressor(100, KisSignalCompressor::Mode::FIRST_INACTIVE)
{
    m_curveWidget = parent;
}

double KisCurveWidget::Private::io2sp(int x, int min, int max)
{
    return unitConverter.io2sp(x, min, max);
}

int KisCurveWidget::Private::sp2io(double x, int min, int max)
{
    return unitConverter.sp2io(x, min, max);
}


bool KisCurveWidget::Private::jumpOverExistingPoints(QPointF &pt, int skipIndex)
{
    Q_FOREACH (const QPointF &it, m_curve.points()) {
        if (m_curve.points().indexOf(it) == skipIndex)
            continue;
        if (fabs(it.x() - pt.x()) < POINT_AREA) {
            pt.rx() = pt.x() >= it.x() ?
                      it.x() + POINT_AREA : it.x() - POINT_AREA;
        }
    }
    return (pt.x() >= 0 && pt.x() <= 1.);
}

int KisCurveWidget::Private::nearestPointInRange(QPointF pt, int wWidth, int wHeight) const
{
    double nearestDistanceSquared = 1000;
    int nearestIndex = -1;
    int i = 0;

    Q_FOREACH (const QPointF & point, m_curve.points()) {
        double distanceSquared = (pt.x() - point.x()) *
                                 (pt.x() - point.x()) +
                                 (pt.y() - point.y()) *
                                 (pt.y() - point.y());

        if (distanceSquared < nearestDistanceSquared) {
            nearestIndex = i;
            nearestDistanceSquared = distanceSquared;
        }
        ++i;
    }

    if (nearestIndex >= 0) {
        if (fabs(pt.x() - m_curve.points()[nearestIndex].x()) *(wWidth - 1) < 5 &&
                fabs(pt.y() - m_curve.points()[nearestIndex].y()) *(wHeight - 1) < 5) {
            return nearestIndex;
        }
    }

    return -1;
}


#define div2_round(x) (((x)+1)>>1)
#define div4_round(x) (((x)+2)>>2)

void KisCurveWidget::Private::drawGrid(QPainter &p, int wWidth, int wHeight)
{
    /**
     * Hint: widget size should conform
     * formula 4n+5 to draw grid correctly
     * without curious shifts between
     * spline and it caused by rounding
     *
     * That is not mandatory but desirable
     */

    QPalette appPalette = QApplication::palette();

    p.setPen(QPen(appPalette.color(QPalette::Background), 1, Qt::SolidLine));
    p.drawLine(div4_round(wWidth), 0, div4_round(wWidth), wHeight);
    p.drawLine(div2_round(wWidth), 0, div2_round(wWidth), wHeight);
    p.drawLine(div4_round(3*wWidth), 0, div4_round(3*wWidth), wHeight);

    p.drawLine(0, div4_round(wHeight), wWidth, div4_round(wHeight));
    p.drawLine(0, div2_round(wHeight), wWidth, div2_round(wHeight));
    p.drawLine(0, div4_round(3*wHeight), wWidth, div4_round(3*wHeight));

}

void KisCurveWidget::Private::syncIOControls()
{
    if (!m_intIn || !m_intOut)
        return;

    bool somethingSelected = (m_grab_point_index >= 0);

    m_intIn->setEnabled(somethingSelected);
    m_intOut->setEnabled(somethingSelected);

    if (m_grab_point_index >= 0) {
        m_intIn->blockSignals(true);
        m_intOut->blockSignals(true);

        m_intIn->setValue(sp2io(m_curve.points()[m_grab_point_index].x(), m_inMin, m_inMax));
        m_intOut->setValue(sp2io(m_curve.points()[m_grab_point_index].y(), m_outMin, m_outMax));

        m_intIn->blockSignals(false);
        m_intOut->blockSignals(false);
    } else {
        /*FIXME: Ideally, these controls should hide away now */
    }
}

void KisCurveWidget::Private::setCurveModified(bool rewriteSpinBoxesValues = true)
{
    if (rewriteSpinBoxesValues) {
        syncIOControls();
    }
    m_splineDirty = true;
    m_curveWidget->update();
    m_curveWidget->emit compressorShouldEmitModified();
}

void KisCurveWidget::Private::setCurveRepaint()
{
    m_curveWidget->update();
}

void KisCurveWidget::Private::setState(enumState st)
{
    m_state = st;
}


enumState KisCurveWidget::Private::state() const
{
    return m_state;
}


#endif /* _KIS_CURVE_WIDGET_P_H_ */
