/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_infinity_manager.h"

#include <QPainter>

#include <klocale.h>

#include <KoCanvasController.h>

#include <kis_debug.h>
#include <kis_view2.h>
#include <kis_canvas2.h>
#include <input/kis_input_manager.h>
#include <kis_config.h>
#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_canvas_controller.h>
#include <kis_algebra_2d.h>



KisInfinityManager::KisInfinityManager(KisView2 *view, KisCanvas2 *canvas)
  : KisCanvasDecoration(INFINITY_DECORATION_ID, view, true),
    m_filteringEnabled(false),
    m_cursorSwitched(false),
    m_sideRects(NSides)
{
    connect(canvas, SIGNAL(documentOffsetUpdateFinished()), SLOT(imagePositionChanged()));
}

inline void KisInfinityManager::addDecoration(const QRect &areaRect, const QPointF &handlePoint, qreal angle, Side side)
{
    QTransform t;
    t.rotate(angle);
    t = t * QTransform::fromTranslate(handlePoint.x(), handlePoint.y());
    m_handleTransform << t;

    m_decorationPath.addRect(areaRect);
    m_sideRects[side] = areaRect;
}

void KisInfinityManager::imagePositionChanged()
{
    QRect imageRect = view()->canvasBase()->coordinatesConverter()->imageRectInWidgetPixels().toAlignedRect();
    QRect widgetRect = view()->canvasBase()->canvasWidget()->rect();

    KisConfig cfg;
    qreal vastScrolling = cfg.vastScrolling();

    int xReserve = vastScrolling * widgetRect.width();
    int yReserve = vastScrolling * widgetRect.height();

    int xThreshold = imageRect.width() - 0.4 * xReserve;
    int yThreshold = imageRect.height() - 0.4 * yReserve;

    const int stripeWidth = 48;

    int xCut = widgetRect.width() - stripeWidth;
    int yCut = widgetRect.height() - stripeWidth;

    m_decorationPath = QPainterPath();
    m_decorationPath.setFillRule(Qt::WindingFill);

    m_handleTransform.clear();

    m_sideRects.clear();
    m_sideRects.resize(NSides);

    bool visible = false;

    if (imageRect.x() <= -xThreshold) {
        QRect areaRect(widgetRect.adjusted(xCut, 0, 0, 0));
        QPointF pt = areaRect.center() + QPointF(-0.1 * stripeWidth, 0);
        addDecoration(areaRect, pt, 0, Right);
        visible = true;
    }

    if (imageRect.y() <= -yThreshold) {
        QRect areaRect(widgetRect.adjusted(0, yCut, 0, 0));
        QPointF pt = areaRect.center() + QPointF(0, -0.1 * stripeWidth);
        addDecoration(areaRect, pt, 90, Bottom);
        visible = true;
    }

    if (imageRect.right() > widgetRect.width() + xThreshold) {
        QRect areaRect(widgetRect.adjusted(0, 0, -xCut, 0));
        QPointF pt = areaRect.center() + QPointF(0.1 * stripeWidth, 0);
        addDecoration(areaRect, pt, 180, Left);
        visible = true;
    }

    if (imageRect.bottom() > widgetRect.height() + yThreshold) {
        QRect areaRect(widgetRect.adjusted(0, 0, 0, -yCut));
        QPointF pt = areaRect.center() + QPointF(0, 0.1 * stripeWidth);
        addDecoration(areaRect, pt, 270, Top);
        visible = true;
    }

    if (visible && !m_filteringEnabled && this->visible()) {
        view()->canvasBase()->inputManager()->attachPriorityEventFilter(this);
        m_filteringEnabled = true;
    }

    if (!visible && m_filteringEnabled || !this->visible()) {
        view()->canvasBase()->inputManager()->detachPriorityEventFilter(this);
        m_filteringEnabled = false;
    }
}

void KisInfinityManager::drawDecoration(QPainter& gc, const QRectF& updateArea, const KisCoordinatesConverter *converter, KisCanvas2 *canvas)
{
    Q_UNUSED(updateArea);
    Q_UNUSED(converter);
    Q_UNUSED(canvas);

    if (!m_filteringEnabled) return;

    gc.save();
    gc.setTransform(QTransform(), false);

    KisConfig cfg;
    QColor color = cfg.canvasBorderColor();
    gc.fillPath(m_decorationPath, color.darker(115));

    QPainterPath p = KisAlgebra2D::smallArrow();

    foreach (const QTransform &t, m_handleTransform) {
        gc.fillPath(t.map(p), color);
    }

    gc.restore();
}

inline int expandLeft(int x0, int x1, int maxExpand)
{
    return qMax(x0 - maxExpand, qMin(x0, x1));
}

inline int expandRight(int x0, int x1, int maxExpand)
{
    return qMin(x0 + maxExpand, qMax(x0, x1));
}

bool KisInfinityManager::eventFilter(QObject *obj, QEvent *event)
{
    KIS_ASSERT_RECOVER_NOOP(m_filteringEnabled);

    bool retval = false;

    switch (event->type()) {
    case QEvent::Enter:
    case QEvent::Leave:
    case QEvent::MouseMove: {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        if (m_decorationPath.contains(mouseEvent->pos())) {
            if (!m_cursorSwitched) {
                m_oldCursor = view()->canvas()->cursor();
                m_cursorSwitched = true;
            }
            view()->canvas()->setCursor(Qt::PointingHandCursor);
            retval = true;
        } else if (m_cursorSwitched) {
            view()->canvas()->setCursor(m_oldCursor);
            m_cursorSwitched = false;
        }
        break;
    }
    case QEvent::MouseButtonPress: {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        retval = mouseEvent->button() == Qt::LeftButton && m_cursorSwitched;

        if (mouseEvent->button() == Qt::RightButton) {
            imagePositionChanged();
        }

        break;
    }
    case QEvent::MouseButtonRelease: {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        retval = mouseEvent->button() == Qt::LeftButton && m_cursorSwitched;

        if (retval) {
            QPoint pos = mouseEvent->pos();

            const KisCoordinatesConverter *converter = view()->canvasBase()->coordinatesConverter();
            QRect widgetRect = converter->widgetToImage(view()->canvas()->rect()).toAlignedRect();
            KisImageWSP image = view()->document()->image();
            QRect cropRect = image->bounds();

            const int hLimit = cropRect.width();
            const int vLimit = cropRect.height();

            if (m_sideRects[Right].contains(pos)) {
                cropRect.setRight(expandRight(cropRect.right(), widgetRect.right(), hLimit));
            }
            if (m_sideRects[Bottom].contains(pos)) {
                cropRect.setBottom(expandRight(cropRect.bottom(), widgetRect.bottom(), vLimit));
            }
            if (m_sideRects[Left].contains(pos)) {
                cropRect.setLeft(expandLeft(cropRect.left(), widgetRect.left(), hLimit));
            }
            if (m_sideRects[Top].contains(pos)) {
                cropRect.setTop(expandLeft(cropRect.top(), widgetRect.top(), vLimit));
            }

            image->resizeImage(cropRect);
        }
        break;
    }
    default:
        break;
    }

    return !retval ? KisCanvasDecoration::eventFilter(obj, event) : true;
}
