/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2000-2001 theKompany.com & Dave Marotti
 * Copyright (C) 2002 Patrick Julien <freak@codepimps.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include <qpainter.h>

#include "kdebug.h"

#include "kis_ruler.h"

#define MARKER_WIDTH 1
#define MARKER_HEIGHT RULER_THICKNESS

const char *KisRuler::m_nums[] = {
    "70 7 2 1",
    "  c Black",
    "X c None",
    "XX   XXXXXX XXXX   XXXX   XXXXXX XXX     XXXX  XXX     XXX   XXXX   XX",
    "X XXX XXXX  XXX XXX XX XXX XXXX  XXX XXXXXXX XXXXXXXXX XX XXX XX XXX X",
    "X XXX XXXXX XXXXXXX XXXXXX XXX X XXX XXXXXX XXXXXXXXX XXX XXX XX XXX X",
    "X XXX XXXXX XXXXX  XXXXX  XXX XX XXX    XXX    XXXXXX XXXX   XXXX    X",
    "X XXX XXXXX XXXX XXXXXXXXX XX     XXXXXX XX XXX XXXX XXXX XXX XXXXXX X",
    "X XXX XXXXX XXX XXXXXX XXX XXXXX XXXXXXX XX XXX XXXX XXXX XXX XXXXX XX",
    "XX   XXXXXX XXX     XXX   XXXXXX XXX    XXXX   XXXXX XXXXX   XXXX  XXX"
};

KisRuler::KisRuler(Qt::Orientation o, QWidget *parent, const char *name) : super(parent, name, WRepaintNoErase | WResizeNoErase), m_pixmapNums(m_nums)
{
    setBackgroundMode(NoBackground);
    setFrameStyle(Box | Sunken);
    setLineWidth(1);
    setMidLineWidth(0);
    m_orientation = o;
    m_unit = KoUnit::U_PT;
    m_zoom = 1.0;
    m_firstVisible = 0;
    m_pixmapBuffer = 0;
    m_currentPosition = -1;

    if (m_orientation == Qt::Horizontal) {
        setFixedHeight(RULER_THICKNESS);
        initMarker(MARKER_WIDTH, MARKER_HEIGHT);
    } else {
        setFixedWidth(RULER_THICKNESS);
        initMarker(MARKER_HEIGHT, MARKER_WIDTH);
    }
}

KisRuler::~KisRuler()
{
    delete m_pixmapBuffer;
}

void KisRuler::initMarker(Q_INT32 w, Q_INT32 h)
{
    QPainter p;

    m_pixmapMarker.resize(w, h);
    p.begin(&m_pixmapMarker);
    p.setPen(blue);
    p.eraseRect(0, 0, w, h);
    p.drawLine(0, 0, w - 1, h - 1);
    p.end();
}

void KisRuler::recalculateSize()
{
    Q_INT32 w;
    Q_INT32 h;

    if (m_pixmapBuffer) {
        delete m_pixmapBuffer;
        m_pixmapBuffer = 0;
    }

    if (m_orientation == Qt::Horizontal) {
        w = width();
        h = RULER_THICKNESS;
    } else {
        w = RULER_THICKNESS;
        h = height();
    }

    m_pixmapBuffer = new QPixmap(w, h);
    Q_CHECK_PTR(m_pixmapBuffer);

    drawRuler();
    updatePointer(m_currentPosition, m_currentPosition);
}

KoUnit::Unit KisRuler::unit() const
{
    return  m_unit;
}

void KisRuler::setUnit(KoUnit::Unit u)
{
    m_unit = u;
    drawRuler();
    updatePointer(m_currentPosition, m_currentPosition);
    update();
}

void KisRuler::setZoom(double zoom)
{
    m_zoom = zoom;
    recalculateSize();
    drawRuler();
    updatePointer(m_currentPosition, m_currentPosition);
    update();
}

void KisRuler::updatePointer(Q_INT32 x, Q_INT32 y)
{
    if (m_pixmapBuffer) {
        if (m_orientation == Qt::Horizontal) {
            if (m_currentPosition != -1)
                repaint(m_currentPosition, 1, MARKER_WIDTH, MARKER_HEIGHT);

            if (x != -1) {
                bitBlt(this, x, 1, &m_pixmapMarker, 0, 0, MARKER_WIDTH, MARKER_HEIGHT);
                m_currentPosition = x;
            }
        } else {
            if (m_currentPosition != -1)
                repaint(1, m_currentPosition, MARKER_HEIGHT, MARKER_WIDTH);

            if (y != -1) {
                bitBlt(this, 1, y, &m_pixmapMarker, 0, 0, MARKER_HEIGHT, MARKER_WIDTH);
                m_currentPosition = y;
            }
        }
    }
}

void KisRuler::updateVisibleArea(Q_INT32 xpos, Q_INT32 ypos)
{
    if (m_orientation == Qt::Horizontal)
        m_firstVisible = xpos;
    else
        m_firstVisible = ypos;

    drawRuler();
    update();
    updatePointer(m_currentPosition, m_currentPosition);
}

void KisRuler::paintEvent(QPaintEvent *e)
{
    if (m_pixmapBuffer) {
        const QRect& rect = e->rect();

        bitBlt(this, rect.topLeft(), m_pixmapBuffer, rect);
        super::paintEvent(e);
    }
}

void KisRuler::drawRuler()
{
    QPainter p;
    QString buf;
    Q_INT32 st1 = 0;
    Q_INT32 st2 = 0;
    Q_INT32 st3 = 0;
    Q_INT32 st4 = 0;

    if (!m_pixmapBuffer)
        return;

    p.begin(m_pixmapBuffer);
    p.setPen(colorGroup().text());
    p.setBackgroundColor(colorGroup().base());
    p.eraseRect(0, 0, m_pixmapBuffer->width(), m_pixmapBuffer->height());

    switch (m_unit) {
        case KoUnit::U_PT:
        case KoUnit::U_MM:
        case KoUnit::U_DD:
        case KoUnit::U_CC:
            st1 = 1;
            st2 = 5;
            st3 = 10;
            st4 = 25;
            break;
        case KoUnit::U_CM:
        case KoUnit::U_PI:
        case KoUnit::U_INCH:
        default:
            st1 = 1;
            st2 = 2;
            st3 = 5;
            st4 = 10;
    }

    bool s1 = KoUnit::fromUserValue(st1, m_unit) * m_zoom > 3.0;
    bool s2 = KoUnit::fromUserValue(st2, m_unit) * m_zoom > 3.0;
    bool s3 = KoUnit::fromUserValue(st3, m_unit) * m_zoom > 3.0;
    bool s4 = KoUnit::fromUserValue(st4, m_unit) * m_zoom > 3.0;

    float cx = KoUnit::fromUserValue(100, m_unit) / m_zoom;
    Q_INT32 step = qRound(cx);

    if (step < 5) {
        step = 1;
    } else if (step < 10) {
        step = 5;
    } else if (step < 25) {
        step = 10;
    } else if (step < 50) {
        step = 25;
    } else if (step < 100) {
        step = 50;
    } else if (step < 250) {
        step = 100;
    } else if (step < 500) {
        step = 250;
    } else if (step < 1000) {
        step = 500;
    } else if (step < 2500) {
        step = 1000;
    } else if (step < 5000) {
        step = 2500;
    } else if (step < 10000) {
        step = 5000;
    } else  if (step < 25000) {
        step = 10000;
    } else if (step < 50000) {
        step = 25000;
    } else if (step < 100000) {
        step = 50000;
    } else {
        step = 100000;
    }

    Q_INT32 start = (Q_INT32)(KoUnit::fromUserValue(m_firstVisible, m_unit) / m_zoom);
    Q_INT32 pos = 0;

    if (m_orientation == Qt::Horizontal) {
        do {
            pos = (Q_INT32)(KoUnit::fromUserValue(start, m_unit) * m_zoom - m_firstVisible);

            if (!s3 && s4 && start % st4 == 0)
                p.drawLine(pos, RULER_THICKNESS - 9, pos, RULER_THICKNESS);

            if (s3 && start % st3 == 0)
                p.drawLine(pos, RULER_THICKNESS - 9, pos, RULER_THICKNESS);

            if (s2 && start % st2 == 0)
                p.drawLine(pos, RULER_THICKNESS - 7, pos, RULER_THICKNESS);

            if (s1 && start % st1 == 0)
                p.drawLine(pos, RULER_THICKNESS - 5, pos, RULER_THICKNESS);

            if (start % step == 0) {
                buf.setNum(QABS(start));
                drawNums(&p, pos, 4, buf, true);
            }

            start++;
        } while (pos < m_pixmapBuffer->width());
    } else {
        do {
            pos = (Q_INT32)(KoUnit::fromUserValue(start, m_unit) * m_zoom - m_firstVisible);

            if (!s3 && s4 && start % st4 == 0)
                p.drawLine(RULER_THICKNESS - 9, pos, RULER_THICKNESS, pos);

            if (s3 && start % st3 == 0)
                p.drawLine(RULER_THICKNESS - 9, pos, RULER_THICKNESS, pos);

            if (s2 && start % st2 == 0)
                p.drawLine(RULER_THICKNESS - 7, pos, RULER_THICKNESS, pos);

            if (s1 && start % st1 == 0)
                p.drawLine(RULER_THICKNESS - 5, pos, RULER_THICKNESS, pos);

            if (start % step == 0) {
                buf.setNum(QABS(start));
                drawNums(&p, 4, pos, buf, false);
            }

            start++;
        } while (pos < m_pixmapBuffer->height());
    }

    p.end();
}

void KisRuler::resizeEvent(QResizeEvent *)
{
    recalculateSize();
}

void KisRuler::styleChange(QStyle& oldStyle)
{
    Q_UNUSED(oldStyle);
    updateGeometry();
    drawRuler();
}

void KisRuler::paletteChange(const QPalette& oldPalette)
{
    Q_UNUSED(oldPalette);
    drawRuler();
}

void KisRuler::show()
{
    if (m_orientation == Qt::Horizontal) {
        setFixedHeight(RULER_THICKNESS);
        initMarker(MARKER_WIDTH, MARKER_HEIGHT);
    } else {
        setFixedWidth(RULER_THICKNESS);
        initMarker(MARKER_HEIGHT, MARKER_WIDTH);
    }

    super::show();
}

void KisRuler::hide()
{
    /*
    if (m_orientation == Qt::Horizontal)
        setFixedHeight(1);
    else
        setFixedWidth(1);
        */
    super::hide();
}

void KisRuler::drawNums(QPainter *p, Q_INT32 x, Q_INT32 y, QString& num, bool orientationHoriz)
{
    if (orientationHoriz)
        x -= 7;
    else
        y -= 8;

    for (Q_UINT32 k = 0; k < num.length(); k++) {
        Q_INT32 st = num.at(k).digitValue() * 7;

        p->drawPixmap(x, y, m_pixmapNums, st, 0, 7, 7);

        if (orientationHoriz)
            x += 7;
        else
            y += 8;
    }
}

#include "kis_ruler.moc"

