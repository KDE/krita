/*
 *  SPDX-FileCopyrightText: 2005 C. Boemann <cbo@boemann.dk>
 *  SPDX-FileCopyrightText: 2009 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _KIS_CURVE_WIDGET_P_H_
#define _KIS_CURVE_WIDGET_P_H_
#include <kis_cubic_curve.h>
#include <QApplication>
#include <QPalette>

enum enumState {
    ST_NORMAL,
    ST_DRAG
};

/**
 * Private members for KisCurveWidget class
 */
class Q_DECL_HIDDEN KisCurveWidget::Private
{

    KisCurveWidget *m_curveWidget {nullptr};

public:
    Private(KisCurveWidget *parent);

    /* Dragging variables */
    int m_grab_point_index {-1};
    double m_grabOffsetX {0.0};
    double m_grabOffsetY {0.0};
    double m_grabOriginalX {0.0};
    double m_grabOriginalY {0.0};
    QPointF m_draggedAwayPoint;
    int m_draggedAwayPointIndex {0};

    bool m_readOnlyMode {false};

    /* The curve itself */
    bool    m_splineDirty {false};
    KisCubicCurve m_curve;

    QPixmap m_pix;
    bool m_pixmapDirty {true};
    QPixmap *m_pixmapCache {nullptr};

    /* view-logic variables */
    int m_handleSize {12}; // size of the control points (diameter, in logical pixels) - both for painting and for detecting clicks

    /**
     * State functions.
     * At the moment used only for dragging.
     */
    enumState m_state {enumState::ST_NORMAL};

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
    void setCurveModified(bool rewriteSpinBoxesValues = true);
    void setCurveRepaint();



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

    // Important:
    // pt and points from the curve are in (0, 1) ranges
    // hence the usage of wWidth etc.

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

        // difference between points is in (0, 1) range as well (or rather, (-1,1))
        QPointF distanceVector = QPointF((pt.x() - m_curve.points()[nearestIndex].x()) *(wWidth - 1),
                                        (pt.y() - m_curve.points()[nearestIndex].y()) *(wHeight - 1));

        if (distanceVector.x() > m_handleSize || distanceVector.y() > m_handleSize) {
            // small performance optimization
            // distance is for sure bigger than m_handleSize now, no need to check
            return -1;
        }

        double distanceInPixels = QLineF(distanceVector, QPointF(0, 0)).length();

        if (distanceInPixels <= m_handleSize) {
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
    Q_EMIT m_curveWidget->shouldSyncIOControls();
}

void KisCurveWidget::Private::setCurveModified(bool rewriteSpinBoxesValues)
{
    if (rewriteSpinBoxesValues) {
        syncIOControls();
    }
    m_splineDirty = true;
    m_curveWidget->update();
    Q_EMIT m_curveWidget->compressorShouldEmitModified();
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
