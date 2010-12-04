/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008,2010 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Roopesh Chander <roop@forwardbias.in>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
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
#include "KoTableBorderStyle.h"

#include "KoTableBorderStyle_p.h"

#include <QPainter>

KoTableBorderStylePrivate::KoTableBorderStylePrivate()
{
    edges[KoTableBorderStyle::Top].spacing = 0;
    borderstyle[KoTableBorderStyle::Top] = KoTableBorderStyle::BorderNone;

    edges[KoTableBorderStyle::Left].spacing = 0;
    borderstyle[KoTableBorderStyle::Left] = KoTableBorderStyle::BorderNone;

    edges[KoTableBorderStyle::Bottom].spacing = 0;
    borderstyle[KoTableBorderStyle::Bottom] = KoTableBorderStyle::BorderNone;

    edges[KoTableBorderStyle::Right].spacing = 0;
    borderstyle[KoTableBorderStyle::Right] = KoTableBorderStyle::BorderNone;

    edges[KoTableBorderStyle::TopLeftToBottomRight].spacing = 0;
    borderstyle[KoTableBorderStyle::TopLeftToBottomRight] = KoTableBorderStyle::BorderNone;

    edges[KoTableBorderStyle::BottomLeftToTopRight].spacing = 0;
    borderstyle[KoTableBorderStyle::BottomLeftToTopRight] = KoTableBorderStyle::BorderNone;
}

KoTableBorderStylePrivate::~KoTableBorderStylePrivate()
{
}

KoTableBorderStyle::KoTableBorderStyle(QObject *parent)
    : QObject(parent)
    , d_ptr(new KoTableBorderStylePrivate())
{
}

KoTableBorderStyle::KoTableBorderStyle(const QTextTableCellFormat &format, QObject *parent)
    : QObject(parent)
    , d_ptr(new KoTableBorderStylePrivate())
{
    init(format);
}

KoTableBorderStyle::KoTableBorderStyle(KoTableBorderStylePrivate &dd, const QTextTableCellFormat &format, QObject *parent)
    : QObject(parent)
    , d_ptr(&dd)
{
    init(format);
}

KoTableBorderStyle::KoTableBorderStyle(KoTableBorderStylePrivate &dd, QObject *parent)
    : QObject(parent)
    , d_ptr(&dd)
{
}

void KoTableBorderStyle::init(const QTextTableCellFormat &format)
{
    Q_D(KoTableBorderStyle);

    d->edges[Top].outerPen = format.penProperty(TopBorderOuterPen);
    d->edges[Top].spacing = format.doubleProperty(TopBorderSpacing);
    d->edges[Top].innerPen = format.penProperty(TopBorderInnerPen);
    d->borderstyle[Top] = BorderStyle(format.intProperty(TopBorderStyle));

    d->edges[Left].outerPen = format.penProperty(LeftBorderOuterPen);
    d->edges[Left].spacing = format.doubleProperty(LeftBorderSpacing);
    d->edges[Left].innerPen = format.penProperty(LeftBorderInnerPen);
    d->borderstyle[Left] = BorderStyle(format.intProperty(LeftBorderStyle));

    d->edges[Bottom].outerPen =format.penProperty(BottomBorderOuterPen);
    d->edges[Bottom].spacing = format.doubleProperty(BottomBorderSpacing);
    d->edges[Bottom].innerPen = format.penProperty(BottomBorderInnerPen);
    d->borderstyle[Bottom] = BorderStyle(format.intProperty(BottomBorderStyle));

    d->edges[Right].outerPen = format.penProperty(RightBorderOuterPen);
    d->edges[Right].spacing = format.doubleProperty(RightBorderSpacing);
    d->edges[Right].innerPen = format.penProperty(RightBorderInnerPen);
    d->borderstyle[Right] = BorderStyle(format.intProperty(RightBorderStyle));

    d->edges[TopLeftToBottomRight].outerPen = format.penProperty(TopLeftToBottomRightBorderOuterPen);
    d->edges[TopLeftToBottomRight].spacing = format.doubleProperty(TopLeftToBottomRightBorderSpacing);
    d->edges[TopLeftToBottomRight].innerPen = format.penProperty(TopLeftToBottomRightBorderInnerPen);
    d->borderstyle[TopLeftToBottomRight] = BorderStyle(format.intProperty(TopLeftToBottomRightBorderStyle));

    d->edges[BottomLeftToTopRight].outerPen = format.penProperty(BottomLeftToTopRightBorderOuterPen);
    d->edges[BottomLeftToTopRight].spacing = format.doubleProperty(BottomLeftToTopRightBorderSpacing);
    d->edges[BottomLeftToTopRight].innerPen = format.penProperty(BottomLeftToTopRightBorderInnerPen);
    d->borderstyle[BottomLeftToTopRight] = BorderStyle(format.intProperty(BottomLeftToTopRightBorderStyle));
}

KoTableBorderStyle::~KoTableBorderStyle()
{
    delete d_ptr;
}


bool KoTableBorderStyle::isDrawn(BorderStyle style) const
{
    if (style == BorderWave)
        return true;
    if (style == BorderDoubleWave)
        return true;
    if (style == BorderSlash)
        return true;
    return false;
}


void KoTableBorderStyle::drawHorizontalWave(BorderStyle style, QPainter &painter, qreal x, qreal w, qreal t) const
{
    QPen pen = painter.pen();
    const qreal linewidth = pen.width();
    const qreal penwidth = linewidth/6;
    pen.setWidth(penwidth);
    painter.setPen(pen);
    if (style == BorderSlash) {
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


void KoTableBorderStyle::drawVerticalWave(BorderStyle style, QPainter &painter, qreal y, qreal h, qreal t) const
{
    QPen pen = painter.pen();
    const qreal linewidth = pen.width();
    const qreal penwidth = linewidth/6;
    pen.setWidth(penwidth);
    painter.setPen(pen);
    if (style == BorderSlash) {
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

void KoTableBorderStyle::setEdge(Side side, BorderStyle style, qreal width, QColor color)
{
    Q_D(KoTableBorderStyle);

    KoTableBorderStylePrivate::Edge edge;
    qreal innerWidth = 0;
    qreal middleWidth = 0;
    qreal space = 0;
    switch (style) {
    case BorderNone:
        width = 0.0;
        break;
    case BorderDouble:
        innerWidth = space = width/4; //some nice default look
        width -= (space + innerWidth);
        edge.outerPen.setStyle(Qt::SolidLine);
        break;
    case BorderDotted:
        edge.outerPen.setStyle(Qt::DotLine);
        break;
    case BorderDashed:
        edge.outerPen.setStyle(Qt::DashLine);
        break;
    case BorderDashedLong: {
        QVector<qreal> dashes;
        dashes << 6 << 6;
        edge.outerPen.setDashPattern(dashes);
        break;
    }
    case BorderTriple:
        innerWidth = middleWidth = space = width/6;
        width -= (space + innerWidth);
        edge.outerPen.setStyle(Qt::SolidLine);
        break;
    case BorderDashDot:
        edge.outerPen.setStyle(Qt::DashDotLine);
        break;
    case BorderDashDotDot:
        edge.outerPen.setStyle(Qt::DashDotDotLine);
        break;
    case BorderWave:
        edge.outerPen.setStyle(Qt::SolidLine);
        break;
    case BorderSlash:
        edge.outerPen.setStyle(Qt::SolidLine);
        break;
    case BorderDoubleWave:
        innerWidth = space = width/4; //some nice default look
        width -= (space + innerWidth);
        edge.outerPen.setStyle(Qt::SolidLine);
        break;
    default:
        edge.outerPen.setStyle(Qt::SolidLine);
        break;
    }
    edge.outerPen.setColor(color);
    edge.outerPen.setJoinStyle(Qt::MiterJoin);
    edge.outerPen.setCapStyle(Qt::FlatCap);
    edge.outerPen.setWidthF(width);

    edge.spacing = space;
    edge.innerPen = edge.outerPen;
    edge.innerPen.setWidthF(innerWidth);
    QPen middlePen;
    middlePen = edge.outerPen;
    middlePen.setWidthF(middleWidth);

    d->edges[side] = edge;
    d->borderstyle[side] = style;
}

void KoTableBorderStyle::setEdgeDoubleBorderValues(Side side, qreal innerWidth, qreal space)
{
    Q_D(KoTableBorderStyle);

    qreal totalWidth = d->edges[side].outerPen.widthF() + d->edges[side].spacing + d->edges[side].innerPen.widthF();
    if (d->edges[side].innerPen.widthF() > 0.0) {
        d->edges[side].outerPen.setWidthF(totalWidth - innerWidth - space);
        d->edges[side].spacing = space;
        d->edges[side].innerPen.setWidthF(innerWidth);
    }
}

bool KoTableBorderStyle::hasBorders() const
{
    Q_D(const KoTableBorderStyle);

    for (int i = Top; i <= BottomLeftToTopRight; i++)
        if (d->edges[i].outerPen.widthF() > 0.0)
            return true;
    return false;
}

void KoTableBorderStyle::paintBorders(QPainter &painter, const QRectF &bounds, QVector<QLineF> *accumulatedBlankBorders) const
{
    Q_D(const KoTableBorderStyle);

    QRectF innerBounds = bounds;

    if (d->edges[Top].outerPen.widthF() > 0) {
        QPen pen = d->edges[Top].outerPen;

        painter.setPen(pen);
        const qreal t = bounds.top() + pen.widthF() / 2.0;
        innerBounds.setTop(bounds.top() + d->edges[Top].spacing + pen.widthF());
        if(isDrawn(d->borderstyle[Top])) {
#if 0 // Unfinished code?
            const qreal width = pen.widthF()/6;
            qreal x;
            //for (
#endif
        } else {
            painter.drawLine(QLineF(bounds.left(), t, bounds.right(), t));
        }
    } else if (accumulatedBlankBorders) {
        // No border but we'd like to draw one for user convenience when on screen
        accumulatedBlankBorders->append(QLineF(bounds.left(), bounds.top(), bounds.right(), bounds.top()));

    }
    if (d->edges[Bottom].outerPen.widthF() > 0) {
        QPen pen = d->edges[Bottom].outerPen;
        painter.setPen(pen);
        const qreal b = bounds.bottom() - pen.widthF() / 2.0;
        innerBounds.setBottom(bounds.bottom() - d->edges[Bottom].spacing - pen.widthF());
        painter.drawLine(QLineF(bounds.left(), b, bounds.right(), b));
    } else if (accumulatedBlankBorders) {
        // No border but we'd like to draw one for user convenience when on screen
        accumulatedBlankBorders->append(QLineF(bounds.left(), bounds.bottom(), bounds.right(), bounds.bottom()));

    }
    if (d->edges[Left].outerPen.widthF() > 0) {
        QPen pen = d->edges[Left].outerPen;
        painter.setPen(pen);
        const qreal l = bounds.left() + pen.widthF() / 2.0;
        innerBounds.setLeft(bounds.left() + d->edges[Left].spacing + pen.widthF());
        painter.drawLine(QLineF(l, bounds.top() + d->edges[Top].outerPen.widthF(), l, bounds.bottom() - d->edges[Bottom].outerPen.widthF()));
    } else if (accumulatedBlankBorders) {
        // No border but we'd like to draw one for user convenience when on screen
        accumulatedBlankBorders->append(QLineF(bounds.left(), bounds.top(), bounds.left(), bounds.bottom()));

    }
    if (d->edges[Right].outerPen.widthF() > 0) {
        QPen pen = d->edges[Right].outerPen;
        painter.setPen(pen);
        const qreal r = bounds.right() - pen.widthF() / 2.0;
        innerBounds.setRight(bounds.right() - d->edges[Right].spacing - pen.widthF());
        painter.drawLine(QLineF(r, bounds.top() + d->edges[Top].outerPen.widthF(), r, bounds.bottom() - d->edges[Bottom].outerPen.widthF()));
    } else if (accumulatedBlankBorders) {
        // No border but we'd like to draw one for user convenience when on screen
        accumulatedBlankBorders->append(QLineF(bounds.right(), bounds.top(), bounds.right(), bounds.bottom()));

    }
    paintDiagonalBorders(painter, bounds);

    // inner lines
    if (d->edges[Top].innerPen.widthF() > 0) {
        QPen pen = d->edges[Top].innerPen;
        painter.setPen(pen);
        const qreal t = innerBounds.top() + pen.widthF() / 2.0;
        painter.drawLine(QLineF(innerBounds.left(), t, innerBounds.right(), t));
    }
    if (d->edges[Bottom].innerPen.widthF() > 0) {
        QPen pen = d->edges[Bottom].innerPen;
        painter.setPen(pen);
        const qreal b = innerBounds.bottom() - pen.widthF() / 2.0;
        painter.drawLine(QLineF(innerBounds.left(), b, innerBounds.right(), b));
    }
    if (d->edges[Left].innerPen.widthF() > 0) {
        QPen pen = d->edges[Left].innerPen;
        painter.setPen(pen);
        const qreal l = innerBounds.left() + pen.widthF() / 2.0;
        painter.drawLine(QLineF(l, innerBounds.top() + d->edges[Top].innerPen.widthF(), l, innerBounds.bottom() - d->edges[Bottom].innerPen.widthF()));
    }
    if (d->edges[Right].innerPen.widthF() > 0) {
        QPen pen = d->edges[Right].innerPen;
        painter.setPen(pen);
        const qreal r = innerBounds.right() - pen.widthF() / 2.0;
        painter.drawLine(QLineF(r, innerBounds.top() + d->edges[Top].innerPen.widthF(), r, innerBounds.bottom() - d->edges[Bottom].innerPen.widthF()));
    }
}

void KoTableBorderStyle::paintDiagonalBorders(QPainter &painter, const QRectF &bounds) const
{
    Q_D(const KoTableBorderStyle);

    if (d->edges[TopLeftToBottomRight].outerPen.widthF() > 0) {
        QPen diagonalPen = d->edges[TopLeftToBottomRight].outerPen;
        painter.setPen(diagonalPen);

        QPen topPen = d->edges[Top].outerPen;
        const qreal top = bounds.top() + topPen.widthF() / 2.0;
        QPen leftPen = d->edges[Left].outerPen;
        const qreal left = bounds.left() + leftPen.widthF() / 2.0;
        QPen bottomPen = d->edges[Bottom].outerPen;
        const qreal bottom = bounds.bottom() - bottomPen.widthF() / 2.0;
        QPen rightPen = d->edges[Right].outerPen;
        const qreal right = bounds.right() - rightPen.widthF() / 2.0;

        painter.drawLine(QLineF(left, top, right, bottom));
    }
    if (d->edges[BottomLeftToTopRight].outerPen.widthF() > 0) {
        QPen pen = d->edges[BottomLeftToTopRight].outerPen;
        painter.setPen(pen);

        QPen topPen = d->edges[Top].outerPen;
        const qreal top = bounds.top() + topPen.widthF() / 2.0;
        QPen leftPen = d->edges[Left].outerPen;
        const qreal left = bounds.left() + leftPen.widthF() / 2.0;
        QPen bottomPen = d->edges[Bottom].outerPen;
        const qreal bottom = bounds.bottom() - bottomPen.widthF() / 2.0;
        QPen rightPen = d->edges[Right].outerPen;
        const qreal right = bounds.right() - rightPen.widthF() / 2.0;

        painter.drawLine(QLineF(left, bottom, right, top));
    }
}

void KoTableBorderStyle::drawTopHorizontalBorder(QPainter &painter, qreal x, qreal y, qreal w, QVector<QLineF> *accumulatedBlankBorders) const
{
    Q_D(const KoTableBorderStyle);

    qreal t=y;
    if (d->edges[Top].outerPen.widthF() > 0) {
        QPen pen = d->edges[Top].outerPen;

        painter.setPen(pen);
        t += pen.widthF() / 2.0;
        if(isDrawn(d->borderstyle[Top])) {
                drawHorizontalWave(d->borderstyle[Top], painter,x,w,t);
        } else {
            painter.drawLine(QLineF(x, t, x+w, t));
        }
        t = y + d->edges[Top].spacing + pen.widthF();
    } else if (accumulatedBlankBorders) {
        // No border but we'd like to draw one for user convenience when on screen
        accumulatedBlankBorders->append(QLineF(x, t, x+w, t));
    }

    // inner line
    if (d->edges[Top].innerPen.widthF() > 0) {
        QPen pen = d->edges[Top].innerPen;
        painter.setPen(pen);
        t += pen.widthF() / 2.0;
        if(isDrawn(d->borderstyle[Top])) {
                drawHorizontalWave(d->borderstyle[Top], painter,x,w,t);
        } else {
            painter.drawLine(QLineF(x, t, x+w, t));
        }
    }
}

void KoTableBorderStyle::drawSharedHorizontalBorder(QPainter &painter, const KoTableBorderStyle &styleBelow,  qreal x, qreal y, qreal w, QVector<QLineF> *accumulatedBlankBorders) const
{
    Q_D(const KoTableBorderStyle);
    const KoTableBorderStylePrivate *styleBelowD = static_cast<const KoTableBorderStylePrivate*>(styleBelow.d_func());

    bool paintThis = true;
    if (d->borderstyle[Bottom] == BorderNone) {
        if (styleBelowD->borderstyle[Top] == BorderNone) {
            if (accumulatedBlankBorders) {
                accumulatedBlankBorders->append(QLineF(x, y, x+w, y));
            }
            return;
        }
        paintThis = false;
    }
    else {
        if (styleBelowD->borderstyle[Top] != BorderNone) {
            qreal thisWidth = d->edges[Bottom].outerPen.widthF() + d->edges[Bottom].spacing + d->edges[Bottom].innerPen.widthF();
            qreal thatWidth = styleBelowD->edges[Top].outerPen.widthF() + styleBelowD->edges[Top].spacing
                            + styleBelowD->edges[Top].innerPen.widthF();
            paintThis = thisWidth >= thatWidth;
        }
    }

    const KoTableBorderStylePrivate::Edge &edge = paintThis ? d->edges[Bottom]: styleBelowD->edges[Top];
    const BorderStyle borderStyle = paintThis ? d->borderstyle[Bottom]: d->borderstyle[Top];
    qreal t=y;

    if (edge.outerPen.widthF() > 0) {
        QPen pen = edge.outerPen;
        const qreal linewidth = pen.widthF();

        painter.setPen(pen);
        t += linewidth / 2.0;
        if(isDrawn(borderStyle)) {
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
        if(isDrawn(borderStyle)) {
            drawHorizontalWave(borderStyle, painter,x,w,t);
        } else {
            painter.drawLine(QLineF(x, t, x+w, t));
        }
    }
}

void KoTableBorderStyle::drawBottomHorizontalBorder(QPainter &painter, qreal x, qreal y, qreal w, QVector<QLineF> *accumulatedBlankBorders) const
{
    Q_D(const KoTableBorderStyle);

    qreal t=y;
    if (d->edges[Bottom].outerPen.widthF() > 0) {
        QPen pen = d->edges[Bottom].outerPen;

        painter.setPen(pen);
        t -= pen.widthF() / 2.0;
        if(isDrawn(d->borderstyle[Bottom])) {
            drawHorizontalWave(d->borderstyle[Bottom], painter,x,w,t);
        } else {
            painter.drawLine(QLineF(x, t, x+w, t));
        }
        t = y - d->edges[Bottom].spacing - pen.widthF();
    } else if (accumulatedBlankBorders) {
        // No border but we'd like to draw one for user convenience when on screen
        accumulatedBlankBorders->append(QLineF(x, t, x+w, t));

    }

    // inner line
    if (d->edges[Bottom].innerPen.widthF() > 0) {
        QPen pen = d->edges[Bottom].innerPen;
        painter.setPen(pen);
        t -= pen.widthF() / 2.0;
        if(isDrawn(d->borderstyle[Bottom])) {
            drawHorizontalWave(d->borderstyle[Bottom], painter,x,w,t);
        } else {
            painter.drawLine(QLineF(x, t, x+w, t));
        }
    }
}

void KoTableBorderStyle::drawLeftmostVerticalBorder(QPainter &painter, qreal x, qreal y, qreal h, QVector<QLineF> *accumulatedBlankBorders) const
{
    Q_D(const KoTableBorderStyle);

    qreal thisWidth = d->edges[Left].outerPen.widthF() + d->edges[Left].spacing + d->edges[Left].innerPen.widthF();
    qreal l = x - thisWidth / 2.0;

    if (d->edges[Left].outerPen.widthF() > 0) {
        QPen pen = d->edges[Left].outerPen;

        painter.setPen(pen);
        l += pen.widthF() / 2.0;
        if(isDrawn(d->borderstyle[Left])) {
            drawVerticalWave(d->borderstyle[Left], painter,y,h,l);
        } else {
            painter.drawLine(QLineF(l, y, l, y+h));
        }
        l += d->edges[Left].spacing + pen.widthF() / 2.0;
    } else if (accumulatedBlankBorders) {
        // No border but we'd like to draw one for user convenience when on screen
        accumulatedBlankBorders->append(QLineF(l, y, l, y+h));

    }

    // inner line
    if (d->edges[Left].innerPen.widthF() > 0) {
        QPen pen = d->edges[Left].innerPen;
        painter.setPen(pen);
        l += pen.widthF() / 2.0;
        if(isDrawn(d->borderstyle[Left])) {
            drawVerticalWave(d->borderstyle[Left], painter,y,h,l);
        } else {
            painter.drawLine(QLineF(l, y, l, y+h));
        }
    }
}

void KoTableBorderStyle::drawSharedVerticalBorder(QPainter &painter, const KoTableBorderStyle &styleRight,  qreal x, qreal y, qreal h, QVector<QLineF> *accumulatedBlankBorders) const
{
    Q_D(const KoTableBorderStyle);
    const KoTableBorderStylePrivate *styleRightD = static_cast<const KoTableBorderStylePrivate*>(styleRight.d_func());

    // First determine which style "wins" by comparing total width
    qreal thisWidth = d->edges[Right].outerPen.widthF() + d->edges[Right].spacing + d->edges[Right].innerPen.widthF();
    qreal thatWidth = styleRightD->edges[Left].outerPen.widthF() + styleRightD->edges[Left].spacing
                                    + styleRightD->edges[Left].innerPen.widthF();

    qreal l=x;

    if(thisWidth >= thatWidth) {
        // left style wins
        l -= thisWidth / 2.0;
        if (d->edges[Right].outerPen.widthF() > 0) {
            QPen pen = d->edges[Right].outerPen;

            painter.setPen(pen);
            l += pen.widthF() / 2.0;
            if(isDrawn(d->borderstyle[Right])) {
                drawVerticalWave(d->borderstyle[Right], painter,y,h,l);
            } else {
                painter.drawLine(QLineF(l, y, l, y+h));
            }
            l += d->edges[Right].spacing + pen.widthF() / 2.0;
        } else if (accumulatedBlankBorders) {
            // No border but we'd like to draw one for user convenience when on screen
            accumulatedBlankBorders->append(QLineF(l, y, l, y+h));

        }

        // inner line
        if (d->edges[Right].innerPen.widthF() > 0) {
            QPen pen = d->edges[Right].innerPen;
            painter.setPen(pen);
            l += pen.widthF() / 2.0;
            if(isDrawn(d->borderstyle[Right])) {
                drawVerticalWave(d->borderstyle[Right], painter,y,h,l);
            } else {
                painter.drawLine(QLineF(l, y, l, y+h));
            }
        }
    } else {
        // right style wins
        l -= thatWidth/2.0;
        if (styleRightD->edges[Left].outerPen.widthF() > 0) {
            QPen pen = styleRightD->edges[Left].outerPen;

            painter.setPen(pen);
            l += pen.widthF() / 2.0;
            if(isDrawn(d->borderstyle[Left])) {
                drawVerticalWave(d->borderstyle[Left], painter,y,h,l);
            } else {
                painter.drawLine(QLineF(l, y, l, y+h));
            }
            l += styleRightD->edges[Left].spacing + pen.widthF() / 2.0;
        }
        // inner line
        if (styleRightD->edges[Left].innerPen.widthF() > 0) {
            QPen pen = styleRightD->edges[Left].innerPen;
            painter.setPen(pen);
            l += pen.widthF() / 2.0;
            if(isDrawn(d->borderstyle[Left])) {
                drawVerticalWave(d->borderstyle[Left], painter,y,h,l);
            } else {
                painter.drawLine(QLineF(l, y, l, y+h));
            }
        }
    }
}

void KoTableBorderStyle::drawRightmostVerticalBorder(QPainter &painter, qreal x, qreal y, qreal h, QVector<QLineF> *accumulatedBlankBorders) const
{
    Q_D(const KoTableBorderStyle);

    qreal thisWidth = d->edges[Right].outerPen.widthF() + d->edges[Right].spacing + d->edges[Right].innerPen.widthF();
    qreal l = x - thisWidth / 2.0;

    if (d->edges[Right].outerPen.widthF() > 0) {
        QPen pen = d->edges[Right].outerPen;

        painter.setPen(pen);
        l += pen.widthF() / 2.0;
        if(isDrawn(d->borderstyle[Right])) {
            drawVerticalWave(d->borderstyle[Right], painter,y,h,l);
        } else {
            painter.drawLine(QLineF(l, y, l, y+h));
        }
        l += d->edges[Right].spacing - pen.widthF() / 2.0;
    } else if (accumulatedBlankBorders) {
        // No border but we'd like to draw one for user convenience when on screen
        accumulatedBlankBorders->append(QLineF(l, y, l, y+h));

    }

    // inner line
    if (d->edges[Right].innerPen.widthF() > 0) {
        QPen pen = d->edges[Right].innerPen;
        painter.setPen(pen);
        l += pen.widthF() / 2.0;
        if(isDrawn(d->borderstyle[Right])) {
            drawVerticalWave(d->borderstyle[Right], painter,y,h,l);
        } else {
            painter.drawLine(QLineF(l, y, l, y+h));
        }
    }
}

qreal KoTableBorderStyle::leftBorderWidth() const
{
    Q_D(const KoTableBorderStyle);

    const KoTableBorderStylePrivate::Edge &edge = d->edges[Left];
    return edge.spacing + edge.innerPen.widthF() + edge.outerPen.widthF();
}

qreal KoTableBorderStyle::rightBorderWidth() const
{
    Q_D(const KoTableBorderStyle);

    const KoTableBorderStylePrivate::Edge &edge = d->edges[Right];
    return edge.spacing + edge.innerPen.widthF() + edge.outerPen.widthF();
}

qreal KoTableBorderStyle::topBorderWidth() const
{
    Q_D(const KoTableBorderStyle);

    const KoTableBorderStylePrivate::Edge &edge = d->edges[Top];
    return edge.spacing + edge.innerPen.widthF() + edge.outerPen.widthF();
}

qreal KoTableBorderStyle::bottomBorderWidth() const
{
    Q_D(const KoTableBorderStyle);

    const KoTableBorderStylePrivate::Edge &edge = d->edges[Bottom];
    return edge.spacing + edge.innerPen.widthF() + edge.outerPen.widthF();
}

