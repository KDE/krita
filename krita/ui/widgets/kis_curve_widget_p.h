/*
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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

#include <QVector>

template <typename T>
class KisTridiagonalSystem
{
    /*
     * e.g.
     *      |b0 c0  0   0   0| |x0| |f0|
     *      |a0 b1 c1   0   0| |x1| |f1|
     *      |0  a1 b2  c2   0|*|x2|=|f2|
     *      |0   0 a2  b3  c3| |x3| |f3|
     *      |0   0  0  a3  b4| |x4| |f4|
     */

public:

    /**
     * @return - vector that is storing x[]
     */
    static
    QVector<T> calculate(QVector<T> &a,
                         QVector<T> &b,
                         QVector<T> &c,
                         QVector<T> &f) {
        QVector<T> x;
        QVector<T> alpha;
        QVector<T> beta;

        int i;
        int size = b.size();

        Q_ASSERT(a.size() == size - 1 &&
                 c.size() == size - 1 &&
                 f.size() == size);

        x.resize(size);

        /**
         * Check for special case when
         * order of the matrix is equal to 1
         */
        if (size == 1) {
            x[0] = f[0] / b[0];
            return x;
        }

        /**
         * Common case
         */

        alpha.resize(size);
        beta.resize(size);


        alpha[1] = -c[0] / b[0];
        beta[1] =  f[0] / b[0];

        for (i = 1; i < size - 1; i++) {
            alpha[i+1] = -c[i] /
                         (a[i-1] * alpha[i] + b[i]);

            beta[i+1] = (f[i] - a[i-1] * beta[i])
                        /
                        (a[i-1] * alpha[i] + b[i]);
        }

        x.last() = (f.last() - a.last() * beta.last())
                   /
                   (b.last() + a.last() * alpha.last());

        for (i = size - 2; i >= 0; i--)
            x[i] = alpha[i+1] * x[i+1] + beta[i+1];

        return x;
    }
};

template <typename T_point, typename T>
class KisCubicSpline
{
    /**
     *  s[i](x)=a[i] +
     *          b[i] * (x-x[i]) +
     *    1/2 * c[i] * (x-x[i])^2 +
     *    1/6 * d[i] * (x-x[i])^3
     *
     *  h[i]=x[i+1]-x[i]
     *
     */

protected:
    QVector<T> m_a;
    QVector<T> m_b;
    QVector<T> m_c;
    QVector<T> m_d;

    QVector<T> m_h;
    T m_begin;
    T m_end;
    int m_intervals;

public:
    KisCubicSpline() {}
    KisCubicSpline(const QVector<T_point> &a) {
        createSpline(a);
    }

    /**
     * Create new spline and precalculate some values
     * for future
     *
     * @a - base points of the spline
     */
    void createSpline(const QVector<T_point> &a) {
        int intervals = m_intervals = a.size() - 1;
        int i;
        m_begin = a.first().x();
        m_end = a.last().x();

        m_a.clear();
        m_b.resize(intervals);
        m_c.clear();
        m_d.resize(intervals);
        m_h.resize(intervals);

        for (i = 0; i < intervals; i++) {
            m_h[i] = a[i+1].x() - a[i].x();
            m_a.append(a[i].y());
        }
        m_a.append(a.last().y());


        QVector<T> tri_b;
        QVector<T> tri_f;
        QVector<T> tri_a; /* equals to @tri_c */

        for (i = 0; i < intervals - 1; i++) {
            tri_b.append(2.*(m_h[i] + m_h[i+1]));

            tri_f.append(6.*((m_a[i+2] - m_a[i+1]) / m_h[i+1] - (m_a[i+1] - m_a[i]) / m_h[i]));
        }
        for (i = 1; i < intervals - 1; i++)
            tri_a.append(m_h[i]);

        if (intervals > 1) {
            KisTridiagonalSystem<T> tridia;
            m_c = tridia.calculate(tri_a, tri_b, tri_a, tri_f);
        }
        m_c.prepend(0);
        m_c.append(0);

        for (i = 0; i < intervals; i++)
            m_d[i] = (m_c[i+1] - m_c[i]) / m_h[i];

        for (i = 0; i < intervals; i++)
            m_b[i] = -0.5 * (m_c[i] * m_h[i])  - (1 / 6.0) * (m_d[i] * m_h[i] * m_h[i]) + (m_a[i+1] - m_a[i]) / m_h[i];
    }

    /**
     * Get value of precalculated spline in the point @x
     */
    T getValue(T x) const {
        T x0;
        int i = findRegion(x, x0);
        /* TODO: check for asm equivalent */
        return m_a[i] +
               m_b[i] *(x - x0) +
               0.5 * m_c[i] *(x - x0) *(x - x0) +
               (1 / 6.0)* m_d[i] *(x - x0) *(x - x0) *(x - x0);
    }

    T begin() const {
        return m_begin;
    }

    T end() const {
        return m_end;
    }

protected:

    /**
     * findRegion - Searches for the region containing @x
     * @x0 - out parameter, containing beginning of the region
     * @return - index of the region
     */
    int findRegion(T x, T &x0) const {
        int i;
        x0 = m_begin;
        for (i = 0; i < m_intervals; i++) {
            if (x >= x0 && x < x0 + m_h[i])
                return i;
            x0 += m_h[i];
        }
        if (x >= m_end) {
            x0 -= m_h[m_intervals-1];
            return m_intervals - 1;
        }

        qDebug("X value: %f\n", x);
        qDebug("m_end  : %f\n", m_end);
        Q_ASSERT_X(0, "findRegion", "X value is outside regions");
        /* **never reached** */
        return -1;
    }
};


class KisCurveWidget;


enum enumState {
    ST_NORMAL,
    ST_DRAG
};



/**
 * Private members for KisCurveWidget class
 */
class KisCurveWidget::Private
{

    KisCurveWidget *m_curveWidget;


public:
    Private(KisCurveWidget *parent);
    virtual ~Private();

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
    KisCubicSpline<QPointF, double> m_spline;

    QList<QPointF> m_points;
    QPixmap m_pix;
    bool m_pixmapDirty;
    QPixmap *m_pixmapCache;

    /* In/Out controls */
    QSpinBox *m_intIn;
    QSpinBox *m_intOut;

    /* Working range of them */
    int m_inOutMin;
    int m_inOutMax;

    /**
     * State functions.
     * At the moment used only for dragging.
     */
    enumState m_state;

    inline void setState(enumState st);
    inline enumState state() const;



    /*** Internal routins ***/

    /**
     * Common update routins
     */
    void setCurveModified();
    void setCurveRepaint();


    /**
     * Convert working range of
     * In/Out controls to normalized
     * range of spline (and reverse)
     */
    double io2sp(int x);
    int sp2io(double x);


    /**
     * Check whether newly created/moved point @pt doesn't overlap
     * with any of existing ones from @m_points and adjusts its coordinates.
     * @skipIndex is the index of the point, that shouldn't be taken
     * into account during the search
     * (e.g. beacuse it's @pt itself)
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
     * Find the nearest point to @pt from m_points
     */
    int nearestPointInRange(QPointF pt, int wWidth, int wHeight) const;

    /**
     * Used in getCurveValue() to automatically
     * extend non-existing parts of the curve
     * (e.g. before the first point)
     * and to cut off big y-values
     */
    static inline
    double checkBounds(const KisCubicSpline<QPointF, double> &spline, double x);

    /**
     * Nothing to be said! =)
     */
    inline
    void drawGrid(QPainter &p, int wWidth, int wHeight);

};

KisCurveWidget::Private::Private(KisCurveWidget *parent)
{
    m_curveWidget = parent;
}

KisCurveWidget::Private::~Private()
{
}


double KisCurveWidget::Private::io2sp(int x)
{
    int rangeLen = m_inOutMax - m_inOutMin;
    return double(x - m_inOutMin) / rangeLen;
}

int KisCurveWidget::Private::sp2io(double x)
{
    int rangeLen = m_inOutMax - m_inOutMin;
    return int(x*rangeLen + 0.5) + m_inOutMin;
}


bool KisCurveWidget::Private::jumpOverExistingPoints(QPointF &pt, int skipIndex)
{
    foreach(const QPointF &it, m_points) {
        if (m_points.indexOf(it) == skipIndex)
            continue;
        if (fabs(it.x() - pt.x()) < POINT_AREA)
            pt.rx() = pt.x() >= it.x() ?
                      it.x() + POINT_AREA : it.x() - POINT_AREA;
    }
    return (pt.x() >= 0 && pt.x() <= 1.);
}

int KisCurveWidget::Private::nearestPointInRange(QPointF pt, int wWidth, int wHeight) const
{
    double nearestDistanceSquared = 1000;
    int nearestIndex = -1;
    int i = 0;

    foreach(const QPointF & point, m_points) {
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
        if (fabs(pt.x() - m_points[nearestIndex].x()) *(wWidth - 1) < 5 &&
                fabs(pt.y() - m_points[nearestIndex].y()) *(wHeight - 1) < 5) {
            return nearestIndex;
        }
    }

    return -1;
}


double KisCurveWidget::Private::checkBounds(const KisCubicSpline<QPointF, double> &spline, double x)
{
    double y;
    x = bounds(x, spline.begin(), spline.end());
    y = spline.getValue(x);
    y = bounds(y, 0., 1.);
    return y;
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

    p.setPen(QPen::QPen(Qt::gray, 1, Qt::SolidLine));
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

        m_intIn->setValue(sp2io(m_points[m_grab_point_index].x()));
        m_intOut->setValue(sp2io(m_points[m_grab_point_index].y()));

        m_intIn->blockSignals(false);
        m_intOut->blockSignals(false);
    } else {
        /*FIXME: Ideally, these controls should hide away now */
    }
}

void KisCurveWidget::Private::setCurveModified()
{
    syncIOControls();
    m_splineDirty = true;
    m_curveWidget->update();
    m_curveWidget->emit modified();
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
