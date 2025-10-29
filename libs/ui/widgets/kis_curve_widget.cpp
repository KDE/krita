/*
 *  SPDX-FileCopyrightText: 2005 C. Boemann <cbo@boemann.dk>
 *  SPDX-FileCopyrightText: 2009 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


// C++ includes.

#include <cmath>
#include <cstdlib>

// Qt includes.

#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QPoint>
#include <QPen>
#include <QEvent>
#include <QFont>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QApplication>

#include <QSpinBox>

// KDE includes.

#include <kis_debug.h>
#include <kis_config.h>
#include <klocalizedstring.h>

#include <kis_signal_compressor.h>
#include <kis_thread_safe_signal_compressor.h>


// Local includes.

#include "widgets/kis_curve_widget.h"


#define bounds(x,a,b) (x<a ? a : (x>b ? b :x))
#define MOUSE_AWAY_THRES 15
#define POINT_AREA       1E-4
#define CURVE_AREA       1E-4

#include "kis_curve_widget_p.h"

KisCurveWidget::KisCurveWidget(QWidget *parent, Qt::WindowFlags f)
        : QWidget(parent, f), d(new KisCurveWidget::Private(this))
{
    setObjectName("KisCurveWidget");

    connect(&d->m_modifiedSignalsCompressor, SIGNAL(timeout()), SLOT(notifyModified()));
    connect(this, SIGNAL(compressorShouldEmitModified()), SLOT(slotCompressorShouldEmitModified()));

    setMouseTracking(true);
    setAutoFillBackground(false);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setMinimumSize(150, 50);
    setMaximumSize(250, 250);

    // Curves widgets don't have context menus. Setting this prevents any
    // long-presses from delaying inputs, see KisLongPressEventFilter.cpp.
    setContextMenuPolicy(Qt::PreventContextMenu);

    setFocusPolicy(Qt::StrongFocus);
}

KisCurveWidget::~KisCurveWidget()
{
    delete d->m_pixmapCache;
    delete d;
}

bool KisCurveWidget::setCurrentPoint(const QPointF &position, bool setAsCorner)
{
    Q_ASSERT(d->m_grab_point_index >= 0);

    bool needResyncControls = true;
    bool isCorner;

    if (d->m_globalPointConstrain == PointConstrain_None) {
        d->m_curve.setPointAsCorner(d->m_grab_point_index, setAsCorner);
        isCorner = setAsCorner;
    } else {
        isCorner = d->m_globalPointConstrain == PointConstrain_AlwaysCorner;
    }

    QPointF newPosition(position);

    if (d->jumpOverExistingPoints(newPosition, d->m_grab_point_index)) {
        needResyncControls = false;
        d->m_curve.setPointPosition(d->m_grab_point_index, newPosition);
        d->m_grab_point_index = d->m_curve.curvePoints().indexOf(
            KisCubicCurvePoint(newPosition, isCorner)
        );
        Q_EMIT pointSelectedChanged();
    }

    d->setCurveModified(false);
    return needResyncControls;
}

bool KisCurveWidget::setCurrentPointPosition(const QPointF &position)
{
    Q_ASSERT(d->m_grab_point_index >= 0);

    return setCurrentPoint(position, d->m_curve.curvePoints()[d->m_grab_point_index].isSetAsCorner());
}

bool KisCurveWidget::setCurrentPointAsCorner(bool setAsCorner)
{
    Q_ASSERT(d->m_grab_point_index >= 0);

    // Setting the point corner flag is not allowed if there is some global constrain set
    if (d->m_globalPointConstrain != PointConstrain_None) {
        return false;
    }

    if (setAsCorner != d->m_curve.curvePoints()[d->m_grab_point_index].isSetAsCorner()) {
        d->m_curve.setPointAsCorner(d->m_grab_point_index, setAsCorner);
        d->setCurveModified(false);
    }

    return false;
}

void KisCurveWidget::setGlobalPointConstrain(PointConstrain constrain)
{
    if (d->m_globalPointConstrain == constrain) {
        return;
    }
    
    d->m_globalPointConstrain = constrain;
    d->applyGlobalPointConstrain();

    d->setCurveModified(false);
}

std::optional<KisCubicCurvePoint> KisCurveWidget::currentPoint() const
{
    return d->m_grab_point_index >= 0 && d->m_grab_point_index < d->m_curve.curvePoints().count()
            ? std::make_optional(d->m_curve.curvePoints()[d->m_grab_point_index])
            : std::nullopt;
}

std::optional<QPointF> KisCurveWidget::currentPointPosition() const
{
    return d->m_grab_point_index >= 0 && d->m_grab_point_index < d->m_curve.curvePoints().count()
            ? std::make_optional(d->m_curve.curvePoints()[d->m_grab_point_index].position())
            : std::nullopt;
}

std::optional<bool> KisCurveWidget::isCurrentPointSetAsCorner() const
{
    return d->m_grab_point_index >= 0 && d->m_grab_point_index < d->m_curve.curvePoints().count()
            ? std::make_optional(d->m_curve.curvePoints()[d->m_grab_point_index].isSetAsCorner())
            : std::nullopt;
}

KisCurveWidget::PointConstrain KisCurveWidget::globalPointConstrain() const
{
    return d->m_globalPointConstrain;
}

void KisCurveWidget::reset(void)
{
    d->m_grab_point_index = -1;
    Q_EMIT pointSelectedChanged();

    //remove total - 2 points.
    while (d->m_curve.curvePoints().count() - 2 ) {
        d->m_curve.removePoint(d->m_curve.curvePoints().count() - 2);
    }

    d->setCurveModified();
}

void KisCurveWidget::setPixmap(const QPixmap & pix)
{
    d->m_pix = pix;
    d->m_pixmapDirty = true;
    d->setCurveRepaint();
}

QPixmap KisCurveWidget::getPixmap()
{
    return d->m_pix;
}

bool KisCurveWidget::pointSelected() const
{
    return d->m_grab_point_index > 0 && d->m_grab_point_index < d->m_curve.curvePoints().count();
}

void KisCurveWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Delete || e->key() == Qt::Key_Backspace) {
        if (d->m_grab_point_index > 0 && d->m_grab_point_index < d->m_curve.curvePoints().count() - 1) {
            //x() find closest point to get focus afterwards
            double grab_point_x = d->m_curve.curvePoints()[d->m_grab_point_index].x();

            int left_of_grab_point_index = d->m_grab_point_index - 1;
            int right_of_grab_point_index = d->m_grab_point_index + 1;
            int new_grab_point_index;

            if (fabs(d->m_curve.curvePoints()[left_of_grab_point_index].x() - grab_point_x) <
                    fabs(d->m_curve.curvePoints()[right_of_grab_point_index].x() - grab_point_x)) {
                new_grab_point_index = left_of_grab_point_index;
            } else {
                new_grab_point_index = d->m_grab_point_index;
            }
            d->m_curve.removePoint(d->m_grab_point_index);
            d->m_grab_point_index = new_grab_point_index;
            Q_EMIT pointSelectedChanged();
            setCursor(Qt::ArrowCursor);
            d->setState(ST_NORMAL);
        }
        e->accept();
        d->setCurveModified();
    } else if (e->key() == Qt::Key_Escape && d->state() != ST_NORMAL) {
        d->m_curve.setPointPosition(d->m_grab_point_index, QPointF(d->m_grabOriginalX, d->m_grabOriginalY) );
        setCursor(Qt::ArrowCursor);
        d->setState(ST_NORMAL);

        e->accept();
        d->setCurveModified();
    } else if ((e->key() == Qt::Key_A || e->key() == Qt::Key_Insert) && d->state() == ST_NORMAL) {
        /* FIXME: Lets user choose the hotkeys */
        addPointInTheMiddle();
        e->accept();
    } else if (e->key() == Qt::Key_S &&
               d->m_globalPointConstrain == PointConstrain_None &&
               pointSelected() &&
               d->state() == ST_NORMAL) {
        /* FIXME: Lets user choose the hotkeys */
        setCurrentPointAsCorner(!*isCurrentPointSetAsCorner());
        e->accept();
    } else
        QWidget::keyPressEvent(e);
}

void KisCurveWidget::addPointInTheMiddle()
{
    QPointF position(0.5, d->m_curve.value(0.5));

    if (!d->jumpOverExistingPoints(position, -1))
        return;

    const bool setAsCorner = d->m_globalPointConstrain == PointConstrain_AlwaysCorner;

    d->m_grab_point_index = d->m_curve.addPoint(position, setAsCorner);
    Q_EMIT pointSelectedChanged();

    Q_EMIT shouldFocusIOControls();
    d->setCurveModified();
}

void KisCurveWidget::resizeEvent(QResizeEvent *e)
{
    d->m_pixmapDirty = true;
    QWidget::resizeEvent(e);
}

void KisCurveWidget::paintEvent(QPaintEvent *)
{
    int    wWidth = width() - 1;
    int    wHeight = height() - 1;


    QPainter p(this);

    // Antialiasing is not a good idea here, because
    // the grid will drift one pixel to any side due to rounding of int
    // FIXME: let's user tell the last word (in config)
    //p.setRenderHint(QPainter::Antialiasing);
     QPalette appPalette = QApplication::palette();
     p.fillRect(rect(), appPalette.color(QPalette::Base)); // clear out previous paint call results

     // make the entire widget grayed out if it is disabled
     if (!this->isEnabled()) {
        p.setOpacity(0.2);
     }



    //  draw background
    if (!d->m_pix.isNull()) {
        if (d->m_pixmapDirty || !d->m_pixmapCache) {
            delete d->m_pixmapCache;
            d->m_pixmapCache = new QPixmap(width(), height());
            QPainter cachePainter(d->m_pixmapCache);

            cachePainter.scale(1.0*width() / d->m_pix.width(), 1.0*height() / d->m_pix.height());
            cachePainter.drawPixmap(0, 0, d->m_pix);
            d->m_pixmapDirty = false;
        }
        p.drawPixmap(0, 0, *d->m_pixmapCache);
    }

    d->drawGrid(p, wWidth, wHeight);

    KisConfig cfg(true);
    if (cfg.antialiasCurves()) {
        p.setRenderHint(QPainter::Antialiasing);
    }

    // Draw curve.
    double curY;
    double normalizedX;
    int x;

    QPolygonF poly;

    p.setPen(QPen(appPalette.color(QPalette::Text), 2, Qt::SolidLine));
    for (x = 0 ; x < wWidth ; x++) {
        normalizedX = double(x) / wWidth;
        curY = wHeight - d->m_curve.value(normalizedX) * wHeight;

        /**
         * Keep in mind that QLineF rounds doubles
         * to ints mathematically, not just rounds down
         * like in C
         */
        poly.append(QPointF(x, curY));
    }
    poly.append(QPointF(x, wHeight - d->m_curve.value(1.0) * wHeight));
    p.drawPolyline(poly);

    QPainterPath fillCurvePath;
    QPolygonF fillPoly = poly;
    fillPoly.append(QPoint(rect().width(), rect().height()));
    fillPoly.append(QPointF(0,rect().height()));

    // add a couple points to the edges so it fills in below always

    QColor fillColor = appPalette.color(QPalette::Text);
    fillColor.setAlphaF(0.2);

    fillCurvePath.addPolygon(fillPoly);
    p.fillPath(fillCurvePath, fillColor);



    // Drawing curve handles.
    if (!d->m_readOnlyMode) {
        const qreal halfHandleSize = d->m_handleSize * 0.5;

        for (int i = 0; i < d->m_curve.curvePoints().count(); ++i) {
            const KisCubicCurvePoint &point = d->m_curve.points().at(i);

            if (i == d->m_grab_point_index) {
                // active point is slightly more "bold"
                p.setPen(QPen(appPalette.color(QPalette::Text), 4, Qt::SolidLine));
            } else {
                p.setPen(QPen(appPalette.color(QPalette::Text), 2, Qt::SolidLine));
            }

            const QPointF handleCenter(point.x() * wWidth, wHeight - point.y() * wHeight);

            if (point.isSetAsCorner()) {
                QPolygonF rhombusHandle;
                rhombusHandle.append(QPointF(handleCenter.x() - halfHandleSize, handleCenter.y()));
                rhombusHandle.append(QPointF(handleCenter.x(), handleCenter.y() - halfHandleSize));
                rhombusHandle.append(QPointF(handleCenter.x() + halfHandleSize, handleCenter.y()));
                rhombusHandle.append(QPointF(handleCenter.x(), handleCenter.y() + halfHandleSize));
                p.drawPolygon(rhombusHandle);
            } else {
                p.drawEllipse(handleCenter, halfHandleSize, halfHandleSize);
            }
        }
    }

    // add border around widget to help contain everything
    QPainterPath widgetBoundsPath;
    widgetBoundsPath.addRect(rect());
    p.strokePath(widgetBoundsPath, appPalette.color(QPalette::Text));


    p.setOpacity(1.0); // reset to 1.0 in case we were drawing a disabled widget before
}

void KisCurveWidget::mousePressEvent(QMouseEvent * e)
{
    if (d->m_readOnlyMode) return;

    if (e->button() != Qt::LeftButton)
        return;

    double x = e->pos().x() / (double)(width() - 1);
    double y = 1.0 - e->pos().y() / (double)(height() - 1);



    int closest_point_index = d->nearestPointInRange(QPointF(x, y), width(), height());
    if (closest_point_index < 0) {
        QPointF newPoint(x, y);
        if (!d->jumpOverExistingPoints(newPoint, -1))
            return;

        const bool setAsCorner = d->m_globalPointConstrain == PointConstrain_AlwaysCorner;
        d->m_grab_point_index = d->m_curve.addPoint(newPoint, setAsCorner);
        Q_EMIT pointSelectedChanged();
    } else {
        d->m_grab_point_index = closest_point_index;
        Q_EMIT pointSelectedChanged();
    }

    const KisCubicCurvePoint &currentPoint = d->m_curve.curvePoints()[d->m_grab_point_index];
    
    d->m_grabOriginalX = currentPoint.x();
    d->m_grabOriginalY = currentPoint.y();
    d->m_grabOffsetX = currentPoint.x() - x;
    d->m_grabOffsetY = currentPoint.y() - y;
    if (e->modifiers().testFlag(Qt::ControlModifier) && d->m_globalPointConstrain == PointConstrain_None) {
        d->m_curve.setPointAsCorner(d->m_grab_point_index, !currentPoint.isSetAsCorner());
    }
    d->m_curve.setPointPosition(d->m_grab_point_index, QPointF(x + d->m_grabOffsetX, y + d->m_grabOffsetY));

    d->m_draggedAwayPointIndex = -1;
    d->setState(ST_DRAG);


    d->setCurveModified();
}


void KisCurveWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (d->m_readOnlyMode) return;

    if (e->button() != Qt::LeftButton)
        return;

    setCursor(Qt::ArrowCursor);
    d->setState(ST_NORMAL);

    d->setCurveModified();
}


void KisCurveWidget::mouseMoveEvent(QMouseEvent * e)
{
    if (d->m_readOnlyMode) return;

    double x = e->pos().x() / (double)(width() - 1);
    double y = 1.0 - e->pos().y() / (double)(height() - 1);

    if (d->state() == ST_NORMAL) { // If no point is selected set the cursor shape if on top
        int nearestPointIndex = d->nearestPointInRange(QPointF(x, y), width(), height());

        if (nearestPointIndex < 0)
            setCursor(Qt::ArrowCursor);
        else
            setCursor(Qt::CrossCursor);
    } else { // Else, drag the selected point
        bool crossedHoriz = e->pos().x() - width() > MOUSE_AWAY_THRES ||
                            e->pos().x() < -MOUSE_AWAY_THRES;
        bool crossedVert =  e->pos().y() - height() > MOUSE_AWAY_THRES ||
                            e->pos().y() < -MOUSE_AWAY_THRES;

        bool removePoint = (crossedHoriz || crossedVert);

        if (!removePoint && d->m_draggedAwayPointIndex >= 0) {
            // point is no longer dragged away so reinsert it
            KisCubicCurvePoint newPoint(d->m_draggedAwayPoint);
            d->m_grab_point_index = d->m_curve.addPoint(newPoint);
            d->m_draggedAwayPointIndex = -1;
        }

        if (removePoint &&
                (d->m_draggedAwayPointIndex >= 0))
            return;


        setCursor(Qt::CrossCursor);

        x += d->m_grabOffsetX;
        y += d->m_grabOffsetY;

        double leftX;
        double rightX;
        if (d->m_grab_point_index == 0) {
            leftX = 0.0;
            if (d->m_curve.curvePoints().count() > 1)
                rightX = d->m_curve.curvePoints()[d->m_grab_point_index + 1].x() - POINT_AREA;
            else
                rightX = 1.0;
        } else if (d->m_grab_point_index == d->m_curve.curvePoints().count() - 1) {
            leftX = d->m_curve.curvePoints()[d->m_grab_point_index - 1].x() + POINT_AREA;
            rightX = 1.0;
        } else {
            Q_ASSERT(d->m_grab_point_index > 0 && d->m_grab_point_index < d->m_curve.curvePoints().count() - 1);

            // the 1E-4 addition so we can grab the dot later.
            leftX = d->m_curve.curvePoints()[d->m_grab_point_index - 1].x() + POINT_AREA;
            rightX = d->m_curve.curvePoints()[d->m_grab_point_index + 1].x() - POINT_AREA;
        }

        x = bounds(x, leftX, rightX);
        y = bounds(y, 0., 1.);

        d->m_curve.setPointPosition(d->m_grab_point_index, QPointF(x, y));

        if (removePoint && d->m_curve.curvePoints().count() > 2) {
            d->m_draggedAwayPoint = d->m_curve.curvePoints()[d->m_grab_point_index];
            d->m_draggedAwayPointIndex = d->m_grab_point_index;
            d->m_curve.removePoint(d->m_grab_point_index);
            d->m_grab_point_index = bounds(d->m_grab_point_index, 0, d->m_curve.curvePoints().count() - 1);
            Q_EMIT pointSelectedChanged();
        }

        d->setCurveModified();
    }
}

KisCubicCurve KisCurveWidget::curve()
{
    return d->m_curve;
}

void KisCurveWidget::setCurve(KisCubicCurve inlist)
{
    d->m_curve = inlist;
    d->m_grab_point_index = qBound(0, d->m_grab_point_index, d->m_curve.curvePoints().count() - 1);
    d->applyGlobalPointConstrain();
    d->setCurveModified();
    Q_EMIT pointSelectedChanged();
}

void KisCurveWidget::leaveEvent(QEvent *)
{
}

void KisCurveWidget::notifyModified()
{
    Q_EMIT modified();
    Q_EMIT curveChanged(d->m_curve);
}

void KisCurveWidget::slotCompressorShouldEmitModified()
{
    d->m_modifiedSignalsCompressor.start();
}
