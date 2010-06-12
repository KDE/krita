/* This file is part of the KDE project
   Copyright (C) 2002 Lennart Kudling <kudling@kde.org>
   Copyright (C) 2002-2003 Rob Buis <buis@kde.org>
   Copyright (C) 2002,2003,2005 Tomislav Lukman <tomislav.lukman@ck.t-com.hr>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2010 Boudewijn Rempt <boud@valdyas.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "StylePreview.h"

#include <KoPathShape.h>
#include <KoShapeBorderModel.h>
#include <KoZoomHandler.h>
#include <KoGradientBackground.h>

#include <QtCore/QEvent>
#include <QtCore/QPoint>
#include <QtCore/QRect>
#include <QtGui/QPaintEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QColor>
#include <QtGui/QBrush>
#include <QtGui/QGradient>

#define PANEL_SIZEX 50.0
#define PANEL_SIZEY 50.0

StylePreview::StylePreview(QWidget * parent)
    : QFrame(parent), m_strokeWidget(false), m_background(0), m_stroke(0)
    , m_strokeRect(5.0, 5.0, 30.0, 30.0), m_fillRect(15.0, 15.0, 30.0, 30.0)
    , m_checkerPainter(10)
{
    setFocusPolicy(Qt::NoFocus);

    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setMaximumHeight(int(PANEL_SIZEY));

    installEventFilter(this);
    update(m_stroke, m_background);
}

StylePreview::~StylePreview()
{
    if (m_background && !m_background->deref())
        delete m_background;
    if (m_stroke && !m_stroke->deref())
        delete m_stroke;
}

void StylePreview::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setClipRect(event->rect());

    m_checkerPainter.paint(painter, rect());

    painter.translate(QPoint(int((width() - PANEL_SIZEX) / 2), int((height() - PANEL_SIZEY) / 2)));

    if (m_strokeWidget) {
        drawFill(painter, m_background);
        drawStroke(painter, m_stroke);
    }
    else {
        drawStroke(painter, m_stroke);
        drawFill(painter, m_background);
    }
    painter.end();

    QFrame::paintEvent(event);
}

QSize StylePreview::sizeHint() const
{
    return QSize(int(PANEL_SIZEX), int(PANEL_SIZEY));
}

QSize StylePreview::minimumSizeHint() const
{
    return QSize(int(PANEL_SIZEX), int(PANEL_SIZEY));
}

QSizePolicy StylePreview::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

bool StylePreview::eventFilter(QObject *, QEvent *event)
{
    if (event && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* e = static_cast<QMouseEvent *>(event);

        int ex = e->x() - int((width() - PANEL_SIZEX) / 2);
        int ey = e->y() - int((height() - PANEL_SIZEY) / 2);

        if (m_strokeWidget) {
            if (m_strokeRect.contains(QPointF(ex, ey))) {
                m_strokeWidget = true;
                emit strokeSelected();
            }
            else if (m_fillRect.contains(QPointF(ex, ey))) {
                m_strokeWidget = false;
                emit fillSelected();
            }
        }
        else {
            if (m_fillRect.contains(QPointF(ex, ey))) {
                m_strokeWidget = false;
                emit fillSelected();
            }
            else if (m_strokeRect.contains(QPointF(ex, ey))) {
                m_strokeWidget = true;
                emit strokeSelected();
            }
        }
        update(m_stroke, m_background);
    }

    return false;
}

void StylePreview::update(KoShapeBorderModel * stroke, KoShapeBackground * fill)
{
    bool updateNeeded = false;
    if (fill != m_background) {
        if (m_background && !m_background->deref())
            delete m_background;

        m_background = fill;

        if (m_background) {
            m_background->ref();
        }
        updateNeeded = true;
    }

    if (stroke != m_stroke) {
        if (m_stroke && !m_stroke->deref())
            delete m_stroke;

        m_stroke = stroke;

        if (m_stroke) {
            m_stroke->ref();
        }
        updateNeeded = true;
    }

    if (updateNeeded) {
        QFrame::update();
    }
}

void StylePreview::drawFill(QPainter & painter, const KoShapeBackground * fill)
{
    painter.save();

    if (fill) {
        const KoGradientBackground * gradientFill = dynamic_cast<const KoGradientBackground*>(fill);
        if (gradientFill) {
            const QGradient * gradient = gradientFill->gradient();
            QBrush brush(Qt::white);
            switch (gradient->type()) {
                case QGradient::LinearGradient:
                {
                    QLinearGradient g;
                    g.setStart(QPointF(30, 20));
                    g.setFinalStop(QPointF(30, 50));
                    g.setStops(gradient->stops());
                    brush = QBrush(g);
                    break;
                }
                case QGradient::RadialGradient:
                {
                    QRadialGradient g;
                    g.setCenter(m_fillRect.center());
                    g.setFocalPoint(m_fillRect.center());
                    g.setRadius(15.0);
                    g.setStops(gradient->stops());
                    brush = QBrush(g);
                    break;
                }
                case QGradient::ConicalGradient:
                {
                    QConicalGradient g;
                    g.setCenter(m_fillRect.center());
                    g.setAngle(0.0);
                    g.setStops(gradient->stops());
                    brush = QBrush(g);
                    break;
                }
                default:
                    break;
            }
            painter.setBrush(brush);
            painter.setPen(Qt::NoPen);
            painter.drawRect(m_fillRect);
        }
        else {
            // use the background to draw
            QPainterPath p;
            p.addRect(m_fillRect);
            fill->paint(painter, p);
        }
    }
    else {
        QBrush brush(Qt::white);
        painter.setBrush(brush);
        painter.setPen(Qt::NoPen);
        painter.drawRect(m_fillRect);
    }

    // show 3D outline of fill part
    painter.setBrush(Qt::NoBrush);

    painter.setPen(Qt::white);
    painter.drawLine(m_fillRect.topRight(), m_fillRect.topLeft());
    painter.drawLine(m_fillRect.topLeft(), m_fillRect.bottomLeft());

    painter.setPen(QColor(127, 127, 127));
    painter.drawLine(m_fillRect.topRight(), m_fillRect.bottomRight());
    painter.drawLine(m_fillRect.bottomRight(), m_fillRect.bottomLeft());

    if (! fill) {
        QPen pen(Qt::red);
        pen.setWidth(2);
        painter.setPen(pen);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.drawLine(m_fillRect.topRight(), m_fillRect.bottomLeft());
    }
    painter.restore();
}


void StylePreview::drawStroke(QPainter & painter, const KoShapeBorderModel * stroke)
{
    painter.save();

    QRectF innerRect = m_strokeRect.adjusted(5, 5, -5, -5);
    QRectF outerRect = m_strokeRect.adjusted(0, 0, 1, 1);

    QRegion clipRegion = QRegion(outerRect.toRect()).subtracted(QRegion(innerRect.toRect()));

    if (stroke) {
        KoPathShape path;

        QRectF middleRect = m_strokeRect.adjusted(2., 2., -2., -2.);
        KoZoomHandler zoomHandler;

        middleRect = zoomHandler.viewToDocument(middleRect);

        path.moveTo(middleRect.topLeft());
        path.lineTo(middleRect.bottomLeft());
        path.lineTo(middleRect.bottomRight());
        path.lineTo(middleRect.topRight());
        path.close();

        KoShapeBorderModel * border = const_cast<KoShapeBorderModel *>(stroke);
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setClipRegion(clipRegion);
        border->paint(&path, painter, zoomHandler);
        painter.restore();
    }
    else {
        painter.save();
        painter.setClipRegion(clipRegion);
        painter.setBrush(Qt::white);
        painter.setPen(Qt::NoPen);
        painter.drawRect(outerRect);
        painter.restore();
    }

    // only draw th 3D ouline when no stroke is set
    // which looks better for thin strokes
    if (! stroke) {
        // show 3D outline of stroke part
        painter.setBrush(Qt::NoBrush);

        painter.setPen(Qt::white);
        painter.drawLine(QPointF(m_strokeRect.right() + 1, m_strokeRect.top() - 1),
                        QPointF(m_strokeRect.left() - 1, m_strokeRect.top() - 1));
        painter.drawLine(QPointF(m_strokeRect.left() - 1, m_strokeRect.top() - 1),
                        QPointF(m_strokeRect.left() - 1, m_strokeRect.bottom() + 1));

        painter.setPen(QColor(127, 127, 127));
        painter.drawLine(QPointF(m_strokeRect.right() + 1, m_strokeRect.top() - 1),
                        QPointF(m_strokeRect.right() + 1, m_strokeRect.bottom() + 1));
        painter.drawLine(QPointF(m_strokeRect.right() + 1, m_strokeRect.bottom() + 1),
                        QPointF(m_strokeRect.left() - 1, m_strokeRect.bottom() + 1));

        painter.setPen(Qt::black);
        painter.drawLine(innerRect.topRight(), innerRect.topLeft());
        painter.drawLine(innerRect.topLeft(), innerRect.bottomLeft());

        painter.setPen(Qt::white);
        painter.drawLine(innerRect.topRight(), innerRect.bottomRight());
        painter.drawLine(innerRect.bottomRight(), innerRect.bottomLeft());
    }

    if (! stroke) {
        QPen pen(Qt::red);
        pen.setWidth(2);
        painter.setPen(pen);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.drawLine(m_strokeRect.topRight(), m_strokeRect.bottomLeft());
    }

    painter.restore();
}

#include <StylePreview.moc>
