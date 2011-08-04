/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Roopesh Chander <roop@forwardbias.in>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
 * Copyright (C) 2011 Pierre Ducroquet <pinaraf@pinaraf.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoTextLayoutCellHelper.h"
#include <KoTableBorderStyle.h>
#include <QPainter>

KoTextLayoutCellHelper::KoTextLayoutCellHelper(const KoTableCellStyle &cellStyle, QObject *parent)
    : QObject(parent), m_cellStyle(cellStyle)
{

}

bool isSpeciallyDrawn(KoBorder::BorderStyle style)
{
    if (style == KoBorder::BorderWave)
        return true;
    if (style == KoBorder::BorderDoubleWave)
        return true;
    if (style == KoBorder::BorderSlash)
        return true;
    return false;
}

void KoTextLayoutCellHelper::drawHorizontalWave(KoBorder::BorderStyle style, QPainter &painter, qreal x, qreal w, qreal t) const
{
    QPen pen = painter.pen();
    const qreal linewidth = pen.width();
    const qreal penwidth = linewidth/6;
    pen.setWidth(penwidth);
    painter.setPen(pen);
    if (style == KoBorder::BorderSlash) {
        for (qreal sx=x; sx<x+w-linewidth; sx+=linewidth*0.5) {
            painter.drawLine(QLineF(sx, t-penwidth*2, sx+linewidth, t+penwidth*2));
        }
    } else {
        for (qreal sx=x; sx<x+w-2*linewidth; sx+=linewidth) {
            painter.drawLine(QLineF(sx, t-penwidth*2, sx+linewidth, t+penwidth*2));
            sx+=linewidth;
            painter.drawLine(QLineF(sx, t+penwidth*2, sx+linewidth, t-penwidth*2));
        }
    }
}

void KoTextLayoutCellHelper::drawVerticalWave(KoBorder::BorderStyle style, QPainter &painter, qreal y, qreal h, qreal t) const
{
    QPen pen = painter.pen();
    const qreal linewidth = pen.width();
    const qreal penwidth = linewidth/6;
    pen.setWidth(penwidth);
    painter.setPen(pen);
    if (style == KoBorder::BorderSlash) {
        for (qreal sy=y; sy<y+h-linewidth; sy+=linewidth*0.5) {
            painter.drawLine(QLineF(t-penwidth*2, sy, t+penwidth*2, sy+linewidth));
        }
    } else {
        for (qreal sy=y; sy<y+h-2*linewidth; sy+=linewidth) {
            painter.drawLine(QLineF(t-penwidth*2, sy, t+penwidth*2, sy+linewidth));
            sy+=linewidth;
            painter.drawLine(QLineF(t+penwidth*2, sy, t-penwidth*2, sy+linewidth));
        }
    }
}


void KoTextLayoutCellHelper::paintBorders(QPainter &painter, const QRectF &bounds, QVector<QLineF> *accumulatedBlankBorders) const
{
    QRectF innerBounds = bounds;

    // outer lines
    QPen topOuterPen = m_cellStyle.getEdge(KoTableBorderStyle::Top).outerPen;
    QPen bottomOuterPen = m_cellStyle.getEdge(KoTableBorderStyle::Bottom).outerPen;
    QPen leftOuterPen = m_cellStyle.getEdge(KoTableBorderStyle::Left).outerPen;
    QPen rightOuterPen = m_cellStyle.getEdge(KoTableBorderStyle::Right).outerPen;

    if (topOuterPen.widthF() > 0) {
        painter.setPen(topOuterPen);
        const qreal t = bounds.top() + topOuterPen.widthF() / 2.0;
        innerBounds.setTop(bounds.top() + m_cellStyle.getEdge(KoTableBorderStyle::Top).spacing + topOuterPen.widthF());
        painter.drawLine(QLineF(bounds.left(), t, bounds.right(), t));
    } else if (accumulatedBlankBorders) {
        // No border but we'd like to draw one for user convenience when on screen
        accumulatedBlankBorders->append(QLineF(bounds.left() + leftOuterPen.widthF() + m_cellStyle.getEdge(KoTableBorderStyle::Left).spacing,
                                               bounds.top() + topOuterPen.widthF() + m_cellStyle.getEdge(KoTableBorderStyle::Top).spacing,
                                               bounds.right() - rightOuterPen.widthF() - m_cellStyle.getEdge(KoTableBorderStyle::Right).spacing,
                                               bounds.top() + topOuterPen.widthF() + m_cellStyle.getEdge(KoTableBorderStyle::Top).spacing));
    }

    if (bottomOuterPen.widthF() > 0) {
        painter.setPen(bottomOuterPen);
        const qreal b = bounds.bottom() - bottomOuterPen.widthF() / 2.0;
        innerBounds.setBottom(bounds.bottom() - m_cellStyle.getEdge(KoTableBorderStyle::Bottom).spacing - bottomOuterPen.widthF());
        painter.drawLine(QLineF(bounds.left(), b, bounds.right(), b));
    } else if (accumulatedBlankBorders) {
        // No border but we'd like to draw one for user convenience when on screen
        accumulatedBlankBorders->append(QLineF(bounds.left() + leftOuterPen.widthF() + m_cellStyle.getEdge(KoTableBorderStyle::Left).spacing,
                                               bounds.bottom() - bottomOuterPen.widthF() - m_cellStyle.getEdge(KoTableBorderStyle::Bottom).spacing,
                                               bounds.right() - rightOuterPen.widthF() - m_cellStyle.getEdge(KoTableBorderStyle::Right).spacing,
                                               bounds.bottom() - bottomOuterPen.widthF() - m_cellStyle.getEdge(KoTableBorderStyle::Bottom).spacing));
    }

    if (leftOuterPen.widthF() > 0) {
        painter.setPen(leftOuterPen);
        const qreal l = bounds.left() + leftOuterPen.widthF() / 2.0;
        innerBounds.setLeft(bounds.left() + m_cellStyle.getEdge(KoTableBorderStyle::Left).spacing + leftOuterPen.widthF());
        painter.drawLine(QLineF(l, bounds.top() + m_cellStyle.getEdge(KoTableBorderStyle::Top).outerPen.widthF(), l, bounds.bottom() - m_cellStyle.getEdge(KoTableBorderStyle::Bottom).outerPen.widthF()));
    } else if (accumulatedBlankBorders) {
        // No border but we'd like to draw one for user convenience when on screen
        accumulatedBlankBorders->append(QLineF(bounds.left() + leftOuterPen.widthF() + m_cellStyle.getEdge(KoTableBorderStyle::Left).spacing,
                                               bounds.top() + topOuterPen.widthF() + m_cellStyle.getEdge(KoTableBorderStyle::Top).spacing,
                                               bounds.left() + leftOuterPen.widthF() + m_cellStyle.getEdge(KoTableBorderStyle::Left).spacing,
                                               bounds.bottom() - bottomOuterPen.widthF() - m_cellStyle.getEdge(KoTableBorderStyle::Bottom).spacing));
    }

    if (m_cellStyle.getEdge(KoTableBorderStyle::Right).outerPen.widthF() > 0) {
        painter.setPen(rightOuterPen);
        const qreal r = bounds.right() - rightOuterPen.widthF() / 2.0;
        innerBounds.setRight(bounds.right() - m_cellStyle.getEdge(KoTableBorderStyle::Right).spacing - rightOuterPen.widthF());
        painter.drawLine(QLineF(r, bounds.top() + m_cellStyle.getEdge(KoTableBorderStyle::Top).outerPen.widthF(), r, bounds.bottom() - bottomOuterPen.widthF()));
    } else if (accumulatedBlankBorders) {
        // No border but we'd like to draw one for user convenience when on screen
        accumulatedBlankBorders->append(QLineF(bounds.right() - rightOuterPen.widthF() - m_cellStyle.getEdge(KoTableBorderStyle::Right).spacing,
                                               bounds.top() + topOuterPen.widthF() + m_cellStyle.getEdge(KoTableBorderStyle::Top).spacing,
                                               bounds.right() - rightOuterPen.widthF() - m_cellStyle.getEdge(KoTableBorderStyle::Right).spacing,
                                               bounds.bottom() - bottomOuterPen.widthF() - m_cellStyle.getEdge(KoTableBorderStyle::Bottom).spacing));
    }

    paintDiagonalBorders(painter, bounds);

    // inner lines
    if (m_cellStyle.getEdge(KoTableBorderStyle::Top).innerPen.widthF() > 0) {
        QPen pen = m_cellStyle.getEdge(KoTableBorderStyle::Top).innerPen;
        painter.setPen(pen);
        const qreal t = innerBounds.top() + pen.widthF() / 2.0;
        painter.drawLine(QLineF(innerBounds.left(), t, innerBounds.right(), t));
    }
    if (m_cellStyle.getEdge(KoTableBorderStyle::Bottom).innerPen.widthF() > 0) {
        QPen pen = m_cellStyle.getEdge(KoTableBorderStyle::Bottom).innerPen;
        painter.setPen(pen);
        const qreal b = innerBounds.bottom() - pen.widthF() / 2.0;
        painter.drawLine(QLineF(innerBounds.left(), b, innerBounds.right(), b));
    }
    if (m_cellStyle.getEdge(KoTableBorderStyle::Left).innerPen.widthF() > 0) {
        QPen pen = m_cellStyle.getEdge(KoTableBorderStyle::Left).innerPen;
        painter.setPen(pen);
        const qreal l = innerBounds.left() + pen.widthF() / 2.0;
        painter.drawLine(QLineF(l, innerBounds.top() + m_cellStyle.getEdge(KoTableBorderStyle::Top).innerPen.widthF(), l, innerBounds.bottom() - m_cellStyle.getEdge(KoTableBorderStyle::Bottom).innerPen.widthF()));
    }
    if (m_cellStyle.getEdge(KoTableBorderStyle::Right).innerPen.widthF() > 0) {
        QPen pen = m_cellStyle.getEdge(KoTableBorderStyle::Right).innerPen;
        painter.setPen(pen);
        const qreal r = innerBounds.right() - pen.widthF() / 2.0;
        painter.drawLine(QLineF(r, innerBounds.top() + m_cellStyle.getEdge(KoTableBorderStyle::Top).innerPen.widthF(), r, innerBounds.bottom() - m_cellStyle.getEdge(KoTableBorderStyle::Bottom).innerPen.widthF()));
    }
}

void KoTextLayoutCellHelper::paintDiagonalBorders(QPainter &painter, const QRectF &bounds) const
{
    if (m_cellStyle.getEdge(KoTableBorderStyle::TopLeftToBottomRight).outerPen.widthF() > 0) {
        QPen diagonalPen = m_cellStyle.getEdge(KoTableBorderStyle::TopLeftToBottomRight).outerPen;
        painter.setPen(diagonalPen);

        QPen topPen = m_cellStyle.getEdge(KoTableBorderStyle::Top).outerPen;
        const qreal top = bounds.top() + topPen.widthF() / 2.0;
        QPen leftPen = m_cellStyle.getEdge(KoTableBorderStyle::Left).outerPen;
        const qreal left = bounds.left() + leftPen.widthF() / 2.0;
        QPen bottomPen = m_cellStyle.getEdge(KoTableBorderStyle::Bottom).outerPen;
        const qreal bottom = bounds.bottom() - bottomPen.widthF() / 2.0;
        QPen rightPen = m_cellStyle.getEdge(KoTableBorderStyle::Right).outerPen;
        const qreal right = bounds.right() - rightPen.widthF() / 2.0;

        painter.drawLine(QLineF(left, top, right, bottom));
    }
    if (m_cellStyle.getEdge(KoTableBorderStyle::BottomLeftToTopRight).outerPen.widthF() > 0) {
        QPen pen = m_cellStyle.getEdge(KoTableBorderStyle::BottomLeftToTopRight).outerPen;
        painter.setPen(pen);

        QPen topPen = m_cellStyle.getEdge(KoTableBorderStyle::Top).outerPen;
        const qreal top = bounds.top() + topPen.widthF() / 2.0;
        QPen leftPen = m_cellStyle.getEdge(KoTableBorderStyle::Left).outerPen;
        const qreal left = bounds.left() + leftPen.widthF() / 2.0;
        QPen bottomPen = m_cellStyle.getEdge(KoTableBorderStyle::Bottom).outerPen;
        const qreal bottom = bounds.bottom() - bottomPen.widthF() / 2.0;
        QPen rightPen = m_cellStyle.getEdge(KoTableBorderStyle::Right).outerPen;
        const qreal right = bounds.right() - rightPen.widthF() / 2.0;

        painter.drawLine(QLineF(left, bottom, right, top));
    }
}

void KoTextLayoutCellHelper::drawTopHorizontalBorder(QPainter &painter, qreal x, qreal y, qreal w, QVector<QLineF> *accumulatedBlankBorders) const
{
    qreal t=y;
    if (m_cellStyle.getEdge(KoTableBorderStyle::Top).outerPen.widthF() > 0) {
        QPen pen = m_cellStyle.getEdge(KoTableBorderStyle::Top).outerPen;

        painter.setPen(pen);
        t += pen.widthF() / 2.0;
        if(isSpeciallyDrawn(m_cellStyle.getBorderStyle(KoTableBorderStyle::Top))) {
            drawHorizontalWave(m_cellStyle.getBorderStyle(KoTableBorderStyle::Top), painter,x,w,t);
        } else {
            painter.drawLine(QLineF(x, t, x+w, t));
        }
        t = y + m_cellStyle.getEdge(KoTableBorderStyle::Top).spacing + pen.widthF();
    } else if (accumulatedBlankBorders) {
        // No border but we'd like to draw one for user convenience when on screen
        accumulatedBlankBorders->append(QLineF(x, t, x+w, t));
    }

    // inner line
    if (m_cellStyle.getEdge(KoTableBorderStyle::Top).innerPen.widthF() > 0) {
        QPen pen = m_cellStyle.getEdge(KoTableBorderStyle::Top).innerPen;
        painter.setPen(pen);
        t += pen.widthF() / 2.0;
        if(isSpeciallyDrawn(m_cellStyle.getBorderStyle(KoTableBorderStyle::Top))) {
            drawHorizontalWave(m_cellStyle.getBorderStyle(KoTableBorderStyle::Top), painter,x,w,t);
        } else {
            painter.drawLine(QLineF(x, t, x+w, t));
        }
    }
}

void KoTextLayoutCellHelper::drawSharedHorizontalBorder(QPainter &painter, const KoTableCellStyle &styleBelow,  qreal x, qreal y, qreal w, QVector<QLineF> *accumulatedBlankBorders) const
{
    bool paintThis = true;
    if (m_cellStyle.getBorderStyle(KoTableBorderStyle::Bottom) == KoBorder::BorderNone) {
        if (styleBelow.getBorderStyle(KoTableBorderStyle::Top) == KoBorder::BorderNone) {
            if (accumulatedBlankBorders) {
                accumulatedBlankBorders->append(QLineF(x, y, x+w, y));
            }
            return;
        }
        paintThis = false;
    }
    else {
        if (styleBelow.getBorderStyle(KoTableBorderStyle::Top) != KoBorder::BorderNone) {
            qreal thisWidth = m_cellStyle.getEdge(KoTableBorderStyle::Bottom).outerPen.widthF() + m_cellStyle.getEdge(KoTableBorderStyle::Bottom).spacing + m_cellStyle.getEdge(KoTableBorderStyle::Bottom).innerPen.widthF();
            qreal thatWidth = styleBelow.getEdge(KoTableBorderStyle::Top).outerPen.widthF() + styleBelow.getEdge(KoTableBorderStyle::Top).spacing
                            + styleBelow.getEdge(KoTableBorderStyle::Top).innerPen.widthF();
            paintThis = thisWidth >= thatWidth;
        }
    }

    const KoTableBorderStyle::Edge &edge = paintThis ? m_cellStyle.getEdge(KoTableBorderStyle::Bottom) : styleBelow.getEdge(KoTableBorderStyle::Top);
    const KoBorder::BorderStyle borderStyle = paintThis ? m_cellStyle.getBorderStyle(KoTableBorderStyle::Bottom): m_cellStyle.getBorderStyle(KoTableBorderStyle::Top);
    qreal t=y;

    if (edge.outerPen.widthF() > 0) {
        QPen pen = edge.outerPen;
        const qreal linewidth = pen.widthF();

        painter.setPen(pen);
        t += linewidth / 2.0;
        if(isSpeciallyDrawn(borderStyle)) {
            drawHorizontalWave(borderStyle, painter,x,w,t);
        } else {
            painter.drawLine(QLineF(x, t, x+w, t));
        }
        t = y + edge.spacing + linewidth;
    }
    // inner line
    if (edge.innerPen.widthF() > 0) {
        QPen pen = edge.innerPen;
        painter.setPen(pen);
        t += pen.widthF() / 2.0;
        if(isSpeciallyDrawn(borderStyle)) {
            drawHorizontalWave(borderStyle, painter,x,w,t);
        } else {
            painter.drawLine(QLineF(x, t, x+w, t));
        }
    }
}

void KoTextLayoutCellHelper::drawBottomHorizontalBorder(QPainter &painter, qreal x, qreal y, qreal w, QVector<QLineF> *accumulatedBlankBorders) const
{
    qreal t=y;
    if (m_cellStyle.getEdge(KoTableBorderStyle::Bottom).outerPen.widthF() > 0) {
        QPen pen = m_cellStyle.getEdge(KoTableBorderStyle::Bottom).outerPen;

        painter.setPen(pen);
        t += pen.widthF() / 2.0;
        if(isSpeciallyDrawn(m_cellStyle.getBorderStyle(KoTableBorderStyle::Bottom))) {
            drawHorizontalWave(m_cellStyle.getBorderStyle(KoTableBorderStyle::Bottom), painter,x,w,t);
        } else {
            painter.drawLine(QLineF(x, t, x+w, t));
        }
        t = y - m_cellStyle.getEdge(KoTableBorderStyle::Bottom).spacing - pen.widthF();
    } else if (accumulatedBlankBorders) {
        // No border but we'd like to draw one for user convenience when on screen
        accumulatedBlankBorders->append(QLineF(x, t, x+w, t));

    }

    // inner line
    if (m_cellStyle.getEdge(KoTableBorderStyle::Bottom).innerPen.widthF() > 0) {
        QPen pen = m_cellStyle.getEdge(KoTableBorderStyle::Bottom).innerPen;
        painter.setPen(pen);
        t -= pen.widthF() / 2.0;
        if(isSpeciallyDrawn(m_cellStyle.getBorderStyle(KoTableBorderStyle::Bottom))) {
            drawHorizontalWave(m_cellStyle.getBorderStyle(KoTableBorderStyle::Bottom), painter,x,w,t);
        } else {
            painter.drawLine(QLineF(x, t, x+w, t));
        }
    }
}

void KoTextLayoutCellHelper::drawLeftmostVerticalBorder(QPainter &painter, qreal x, qreal y, qreal h, QVector<QLineF> *accumulatedBlankBorders) const
{
    qreal thisWidth = m_cellStyle.getEdge(KoTableBorderStyle::Left).outerPen.widthF() + m_cellStyle.getEdge(KoTableBorderStyle::Left).spacing + m_cellStyle.getEdge(KoTableBorderStyle::Left).innerPen.widthF();
    qreal l = x - thisWidth / 2.0;

    if (m_cellStyle.getEdge(KoTableBorderStyle::Left).outerPen.widthF() > 0) {
        QPen pen = m_cellStyle.getEdge(KoTableBorderStyle::Left).outerPen;

        painter.setPen(pen);
        l += pen.widthF() / 2.0;
        if(isSpeciallyDrawn(m_cellStyle.getBorderStyle(KoTableBorderStyle::Left))) {
            drawVerticalWave(m_cellStyle.getBorderStyle(KoTableBorderStyle::Left), painter,y,h,l);
        } else {
            painter.drawLine(QLineF(l, y, l, y+h));
        }
        l += m_cellStyle.getEdge(KoTableBorderStyle::Left).spacing + pen.widthF() / 2.0;
    } else if (accumulatedBlankBorders) {
        // No border but we'd like to draw one for user convenience when on screen
        accumulatedBlankBorders->append(QLineF(l, y, l, y+h));

    }

    // inner line
    if (m_cellStyle.getEdge(KoTableBorderStyle::Left).innerPen.widthF() > 0) {
        QPen pen = m_cellStyle.getEdge(KoTableBorderStyle::Left).innerPen;
        painter.setPen(pen);
        l += pen.widthF() / 2.0;
        if(isSpeciallyDrawn(m_cellStyle.getBorderStyle(KoTableBorderStyle::Left))) {
            drawVerticalWave(m_cellStyle.getBorderStyle(KoTableBorderStyle::Left), painter,y,h,l);
        } else {
            painter.drawLine(QLineF(l, y, l, y+h));
        }
    }
}

void KoTextLayoutCellHelper::drawSharedVerticalBorder(QPainter &painter, const KoTableCellStyle &styleRight,  qreal x, qreal y, qreal h, QVector<QLineF> *accumulatedBlankBorders) const
{
    // First determine which style "wins" by comparing total width
    qreal thisWidth = m_cellStyle.getEdge(KoTableBorderStyle::Right).outerPen.widthF() + m_cellStyle.getEdge(KoTableBorderStyle::Right).spacing + m_cellStyle.getEdge(KoTableBorderStyle::Right).innerPen.widthF();
    qreal thatWidth = styleRight.getEdge(KoTableBorderStyle::Left).outerPen.widthF() + styleRight.getEdge(KoTableBorderStyle::Left).spacing
                                    + styleRight.getEdge(KoTableBorderStyle::Left).innerPen.widthF();

    qreal l=x;

    if(thisWidth >= thatWidth) {
        // left style wins
        l -= thisWidth / 2.0;
        if (m_cellStyle.getEdge(KoTableBorderStyle::Right).outerPen.widthF() > 0) {
            QPen pen = m_cellStyle.getEdge(KoTableBorderStyle::Right).outerPen;

            painter.setPen(pen);
            l += pen.widthF() / 2.0;
            if(isSpeciallyDrawn(m_cellStyle.getBorderStyle(KoTableBorderStyle::Right))) {
                drawVerticalWave(m_cellStyle.getBorderStyle(KoTableBorderStyle::Right), painter,y,h,l);
            } else {
                painter.drawLine(QLineF(l, y, l, y+h));
            }
            l += m_cellStyle.getEdge(KoTableBorderStyle::Right).spacing + pen.widthF() / 2.0;
        } else if (accumulatedBlankBorders) {
            // No border but we'd like to draw one for user convenience when on screen
            accumulatedBlankBorders->append(QLineF(l, y, l, y+h));

        }

        // inner line
        if (m_cellStyle.getEdge(KoTableBorderStyle::Right).innerPen.widthF() > 0) {
            QPen pen = m_cellStyle.getEdge(KoTableBorderStyle::Right).innerPen;
            painter.setPen(pen);
            l += pen.widthF() / 2.0;
            if(isSpeciallyDrawn(m_cellStyle.getBorderStyle(KoTableBorderStyle::Right))) {
                drawVerticalWave(m_cellStyle.getBorderStyle(KoTableBorderStyle::Right), painter,y,h,l);
            } else {
                painter.drawLine(QLineF(l, y, l, y+h));
            }
        }
    } else {
        // right style wins
        l -= thatWidth/2.0;
        if (styleRight.getEdge(KoTableBorderStyle::Left).outerPen.widthF() > 0) {
            QPen pen = styleRight.getEdge(KoTableBorderStyle::Left).outerPen;

            painter.setPen(pen);
            l += pen.widthF() / 2.0;
            if(isSpeciallyDrawn(m_cellStyle.getBorderStyle(KoTableBorderStyle::Left))) {
                drawVerticalWave(m_cellStyle.getBorderStyle(KoTableBorderStyle::Left), painter,y,h,l);
            } else {
                painter.drawLine(QLineF(l, y, l, y+h));
            }
            l += styleRight.getEdge(KoTableBorderStyle::Left).spacing + pen.widthF() / 2.0;
        }
        // inner line
        if (styleRight.getEdge(KoTableBorderStyle::Left).innerPen.widthF() > 0) {
            QPen pen = styleRight.getEdge(KoTableBorderStyle::Left).innerPen;
            painter.setPen(pen);
            l += pen.widthF() / 2.0;
            if(isSpeciallyDrawn(m_cellStyle.getBorderStyle(KoTableBorderStyle::Left))) {
                drawVerticalWave(m_cellStyle.getBorderStyle(KoTableBorderStyle::Left), painter,y,h,l);
            } else {
                painter.drawLine(QLineF(l, y, l, y+h));
            }
        }
    }
}

void KoTextLayoutCellHelper::drawRightmostVerticalBorder(QPainter &painter, qreal x, qreal y, qreal h, QVector<QLineF> *accumulatedBlankBorders) const
{
    qreal thisWidth = m_cellStyle.getEdge(KoTableBorderStyle::Right).outerPen.widthF() + m_cellStyle.getEdge(KoTableBorderStyle::Right).spacing + m_cellStyle.getEdge(KoTableBorderStyle::Right).innerPen.widthF();
    qreal l = x - thisWidth / 2.0;

    if (m_cellStyle.getEdge(KoTableBorderStyle::Right).outerPen.widthF() > 0) {
        QPen pen = m_cellStyle.getEdge(KoTableBorderStyle::Right).outerPen;

        painter.setPen(pen);
        l += pen.widthF() / 2.0;
        if(isSpeciallyDrawn(m_cellStyle.getBorderStyle(KoTableBorderStyle::Right))) {
            drawVerticalWave(m_cellStyle.getBorderStyle(KoTableBorderStyle::Right), painter,y,h,l);
        } else {
            painter.drawLine(QLineF(l, y, l, y+h));
        }
        l += m_cellStyle.getEdge(KoTableBorderStyle::Right).spacing - pen.widthF() / 2.0;
    } else if (accumulatedBlankBorders) {
        // No border but we'd like to draw one for user convenience when on screen
        accumulatedBlankBorders->append(QLineF(l, y, l, y+h));
    }

    // inner line
    if (m_cellStyle.getEdge(KoTableBorderStyle::Right).innerPen.widthF() > 0) {
        QPen pen = m_cellStyle.getEdge(KoTableBorderStyle::Right).innerPen;
        painter.setPen(pen);
        l += pen.widthF() / 2.0;
        if(isSpeciallyDrawn(m_cellStyle.getBorderStyle(KoTableBorderStyle::Right))) {
            drawVerticalWave(m_cellStyle.getBorderStyle(KoTableBorderStyle::Right), painter,y,h,l);
        } else {
            painter.drawLine(QLineF(l, y, l, y+h));
        }
    }
}
