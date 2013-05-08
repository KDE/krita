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
#include <kis_config.h>
#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_canvas_controller.h>


KisInfinityManager::KisInfinityManager(KisView2 *view, KisCanvas2 *canvas)
    : KisCanvasDecoration(INFINITY_DECORATION_ID, i18n("Expand into Infinity Decoration"), view),
      m_filterInstalled(false),
      m_cursorSwitched(false)
{
    connect(canvas, SIGNAL(documentOffsetUpdateFinished()), SLOT(imagePositionChanged()));
}

inline void KisInfinityManager::addDecoration(const QRect &areaRect, const QPointF &handlePoint, qreal angle)
{
    QTransform t;
    t.rotate(angle);
    t = t * QTransform::fromTranslate(handlePoint.x(), handlePoint.y());
    m_handleTransform << t;

    m_decorationPath.addRect(areaRect);
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

    bool visible = false;

    if (imageRect.x() <= -xThreshold) {
        QRect areaRect(widgetRect.adjusted(xCut, 0, 0, 0));
        QPointF pt = areaRect.center() + QPointF(-0.1 * stripeWidth, 0);
        addDecoration(areaRect, pt, 0);
        visible = true;
    }

    if (imageRect.y() <= -yThreshold) {
        QRect areaRect(widgetRect.adjusted(0, yCut, 0, 0));
        QPointF pt = areaRect.center() + QPointF(0, -0.1 * stripeWidth);
        addDecoration(areaRect, pt, 90);
        visible = true;
    }

    if (imageRect.right() > widgetRect.width() + xThreshold) {
        QRect areaRect(widgetRect.adjusted(0, 0, -xCut, 0));
        QPointF pt = areaRect.center() + QPointF(0.1 * stripeWidth, 0);
        addDecoration(areaRect, pt, 180);
        visible = true;
    }

    if (imageRect.bottom() > widgetRect.height() + yThreshold) {
        QRect areaRect(widgetRect.adjusted(0, 0, 0, -yCut));
        QPointF pt = areaRect.center() + QPointF(0, 0.1 * stripeWidth);
        addDecoration(areaRect, pt, 270);
        visible = true;
    }

    setVisible(visible);

    if (visible && !m_filterInstalled) {
        view()->canvasBase()->canvasWidget()->installEventFilter(this);
    }

    if (!visible && m_filterInstalled) {
        view()->canvasBase()->canvasWidget()->removeEventFilter(this);
    }
}

void KisInfinityManager::drawDecoration(QPainter& gc, const QRectF& updateArea, const KisCoordinatesConverter *converter)
{
    Q_UNUSED(updateArea);
    Q_UNUSED(converter);

    gc.save();
    gc.setTransform(QTransform(), false);

    KisConfig cfg;
    QColor color = cfg.canvasBorderColor();
    gc.fillPath(m_decorationPath, color.darker(115));

    QPainterPath p;

    p.moveTo(5, 2);
    p.lineTo(-3, 8);
    p.lineTo(-5, 5);
    p.lineTo( 2, 0);
    p.lineTo(-5,-5);
    p.lineTo(-3,-8);
    p.lineTo( 5,-2);
    p.arcTo(QRectF(3, -2, 4, 4), 90, -180);

    foreach (const QTransform &t, m_handleTransform) {
        gc.fillPath(t.map(p), color);
    }

    gc.restore();
}

bool KisInfinityManager::eventFilter(QObject *obj, QEvent *event)
{
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
            const KisCoordinatesConverter *converter = view()->canvasBase()->coordinatesConverter();
            QRect widgetRect = converter->widgetToImage(view()->canvas()->rect()).toAlignedRect();
            KisImageWSP image = view()->document()->image();
            QRect cropRect = widgetRect | image->bounds();
            image->resizeImage(cropRect);
        }
        break;
    }
    default:
        break;
    }

    return !retval ? KisCanvasDecoration::eventFilter(obj, event) : true;
}
