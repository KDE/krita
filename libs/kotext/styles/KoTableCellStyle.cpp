/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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
#include "KoTableCellStyle.h"
#include "KoStyleManager.h"
#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include "Styles_p.h"
#include "KoTextDocument.h"

#include <KDebug>

#include <QTextTable>
#include <QTextTableFormat>

#include <KoUnit.h>
#include <KoStyleStack.h>
#include <KoOdfLoadingContext.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>

struct Edge {
    Edge() : innerPen(), outerPen(), spacing(0.0) { }
    QPen innerPen;
    QPen outerPen;
    qreal spacing;
};

class KoTableCellStyle::Private
{
public:
    Private() : parentStyle(0), next(0) {}

    ~Private() {
    }

    void setProperty(int key, const QVariant &value) {
        stylesPrivate.add(key, value);
    }

    Edge edges[6];
    BorderStyle borderstyle[6];
    QString name;
    KoTableCellStyle *parentStyle;
    int next;
    StylePrivate stylesPrivate;
};

KoTableCellStyle::KoTableCellStyle(QObject *parent)
        : QObject(parent), d(new Private())
{
    //d->edges[Top].outerPen = format.penProperty(TopBorderOuterPen);
    d->edges[Top].spacing = 0;
   // d->edges[Top].innerPen = format.penProperty(TopBorderInnerPen);
    d->borderstyle[Top] = BorderNone;

   // d->edges[Left].outerPen = format.penProperty(LeftBorderOuterPen);
    d->edges[Left].spacing = 0;
   // d->edges[Left].innerPen = format.penProperty(LeftBorderInnerPen);
    d->borderstyle[Left] = BorderNone;

    //d->edges[Bottom].outerPen =format.penProperty(BottomBorderOuterPen);
    d->edges[Bottom].spacing = 0;
    //d->edges[Bottom].innerPen = format.penProperty(BottomBorderInnerPen);
    d->borderstyle[Bottom] = BorderNone;

    //d->edges[Right].outerPen = format.penProperty(RightBorderOuterPen);
    d->edges[Right].spacing = 0;
    //d->edges[Right].innerPen = format.penProperty(RightBorderInnerPen);
    d->borderstyle[Right] = BorderNone;

    d->edges[TopLeftToBottomRight].spacing = 0;
    d->borderstyle[TopLeftToBottomRight] = BorderNone;

    d->edges[BottomLeftToTopRight].spacing = 0;
    d->borderstyle[BottomLeftToTopRight] = BorderNone;
}

bool KoTableCellStyle::isDrawn(BorderStyle style) const
{
    if (style == BorderWave)
        return true;
    if (style == BorderDoubleWave)
        return true;
    if (style == BorderSlash)
        return true;
    return false;
}

KoTableCellStyle::KoTableCellStyle(const QTextTableCellFormat &format, QObject *parent)
        : QObject(parent),
        d(new Private())
{
    d->stylesPrivate = format.properties();

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

KoTableCellStyle *KoTableCellStyle::fromTableCell(const QTextTableCell &tableCell, QObject *parent)
{
    QTextTableCellFormat tableCellFormat = tableCell.format().toTableCellFormat();
    return new KoTableCellStyle(tableCellFormat, parent);
}

KoTableCellStyle::~KoTableCellStyle()
{
    delete d;
}

QRectF KoTableCellStyle::contentRect(const QRectF &boundingRect) const
{
    return boundingRect.adjusted(
                d->edges[Left].outerPen.widthF() + d->edges[Left].spacing + d->edges[Left].innerPen.widthF() + propertyDouble(QTextFormat::TableCellLeftPadding),
                d->edges[Top].outerPen.widthF() + d->edges[Top].spacing + d->edges[Top].innerPen.widthF() + propertyDouble(QTextFormat::TableCellTopPadding),
                - d->edges[Right].outerPen.widthF() - d->edges[Right].spacing - d->edges[Right].innerPen.widthF() - propertyDouble(QTextFormat::TableCellRightPadding),
                - d->edges[Bottom].outerPen.widthF() - d->edges[Bottom].spacing - d->edges[Bottom].innerPen.widthF() - propertyDouble(QTextFormat::TableCellBottomPadding)
   );
}

QRectF KoTableCellStyle::boundingRect(const QRectF &contentRect) const
{
    return contentRect.adjusted(
                - d->edges[Left].outerPen.widthF() - d->edges[Left].spacing - d->edges[Left].innerPen.widthF() - propertyDouble(QTextFormat::TableCellLeftPadding),
                - d->edges[Top].outerPen.widthF() - d->edges[Top].spacing - d->edges[Top].innerPen.widthF() - propertyDouble(QTextFormat::TableCellTopPadding),
                d->edges[Right].outerPen.widthF() + d->edges[Right].spacing + d->edges[Right].innerPen.widthF() + propertyDouble(QTextFormat::TableCellRightPadding),
                d->edges[Bottom].outerPen.widthF() + d->edges[Bottom].spacing + d->edges[Bottom].innerPen.widthF() + propertyDouble(QTextFormat::TableCellBottomPadding)
   );
}

void KoTableCellStyle::setEdge(Side side, BorderStyle style, qreal width, QColor color)
{
    Edge edge;
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

void KoTableCellStyle::setEdgeDoubleBorderValues(Side side, qreal innerWidth, qreal space)
{
    qreal totalWidth = d->edges[side].outerPen.widthF() + d->edges[side].spacing + d->edges[side].innerPen.widthF();
    if (d->edges[side].innerPen.widthF() > 0.0) {
        d->edges[side].outerPen.setWidthF(totalWidth - innerWidth - space);
        d->edges[side].spacing = space;
        d->edges[side].innerPen.setWidthF(innerWidth);
    }
}

bool KoTableCellStyle::hasBorders() const
{
    for (int i = Top; i <= BottomLeftToTopRight; i++)
        if (d->edges[i].outerPen.widthF() > 0.0)
            return true;
    return false;
}

void KoTableCellStyle::paintBackground(QPainter &painter, const QRectF &bounds) const
{
    QRectF innerBounds = bounds;

    if (hasProperty(CellBackgroundBrush)) {
        painter.fillRect(bounds, background());
    }
}

void KoTableCellStyle::drawHorizontalWave(BorderStyle style, QPainter &painter, qreal x, qreal w, qreal t) const
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

void KoTableCellStyle::drawVerticalWave(BorderStyle style, QPainter &painter, qreal y, qreal h, qreal t) const
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

void KoTableCellStyle::paintBorders(QPainter &painter, const QRectF &bounds) const
{
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
    }
    if (d->edges[Bottom].outerPen.widthF() > 0) {
        QPen pen = d->edges[Bottom].outerPen;
        painter.setPen(pen);
        const qreal b = bounds.bottom() - pen.widthF() / 2.0;
        innerBounds.setBottom(bounds.bottom() - d->edges[Bottom].spacing - pen.widthF());
        painter.drawLine(QLineF(bounds.left(), b, bounds.right(), b));
    }
    if (d->edges[Left].outerPen.widthF() > 0) {
        QPen pen = d->edges[Left].outerPen;
        painter.setPen(pen);
        const qreal l = bounds.left() + pen.widthF() / 2.0;
        innerBounds.setLeft(bounds.left() + d->edges[Left].spacing + pen.widthF());
        painter.drawLine(QLineF(l, bounds.top() + d->edges[Top].outerPen.widthF(), l, bounds.bottom() - d->edges[Bottom].outerPen.widthF()));
    }
    if (d->edges[Right].outerPen.widthF() > 0) {
        QPen pen = d->edges[Right].outerPen;
        painter.setPen(pen);
        const qreal r = bounds.right() - pen.widthF() / 2.0;
        innerBounds.setRight(bounds.right() - d->edges[Right].spacing - pen.widthF());
        painter.drawLine(QLineF(r, bounds.top() + d->edges[Top].outerPen.widthF(), r, bounds.bottom() - d->edges[Bottom].outerPen.widthF()));
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

void KoTableCellStyle::paintDiagonalBorders(QPainter &painter, const QRectF &bounds) const
{
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

void KoTableCellStyle::drawTopHorizontalBorder(QPainter &painter, qreal x, qreal y, qreal w, QPainterPath *accumulatedBlankBorders) const
{
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
        accumulatedBlankBorders->moveTo(x, t);
        accumulatedBlankBorders->lineTo(x+w, t);
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

void KoTableCellStyle::drawSharedHorizontalBorder(QPainter &painter, const KoTableCellStyle &styleBelow,  qreal x, qreal y, qreal w, QPainterPath *accumulatedBlankBorders) const
{
    // First determine which style "wins" by comparing total width
    qreal thisWidth = d->edges[Bottom].outerPen.widthF() + d->edges[Bottom].spacing + d->edges[Bottom].innerPen.widthF();
    qreal thatWidth = styleBelow.d->edges[Top].outerPen.widthF() + styleBelow.d->edges[Top].spacing
                                    + styleBelow.d->edges[Top].innerPen.widthF();
    if(thisWidth >= thatWidth) {
        // bottom style wins
       qreal t=y;
        if (d->edges[Bottom].outerPen.widthF() > 0) {
            QPen pen = d->edges[Bottom].outerPen;
            const qreal linewidth = pen.widthF();

            painter.setPen(pen);
            t += linewidth / 2.0;
            if(isDrawn(d->borderstyle[Bottom])) {
                drawHorizontalWave(d->borderstyle[Bottom], painter,x,w,t);
            } else {
                painter.drawLine(QLineF(x, t, x+w, t));
            }
            t = y + d->edges[Bottom].spacing + linewidth;
        } else if (accumulatedBlankBorders) {
            // No border but we'd like to draw one for user convenience when on screen
            accumulatedBlankBorders->moveTo(x, t);
            accumulatedBlankBorders->lineTo(x+w, t);

        }
        // inner line
        if (d->edges[Bottom].innerPen.widthF() > 0) {
            QPen pen = d->edges[Bottom].innerPen;
            painter.setPen(pen);
            t += pen.widthF() / 2.0;
            if(isDrawn(d->borderstyle[Bottom])) {
                drawHorizontalWave(d->borderstyle[Bottom], painter,x,w,t);
            } else {
                painter.drawLine(QLineF(x, t, x+w, t));
            }
        }
    } else {
        // top style wins
        qreal t=y;
        if (styleBelow.d->edges[Top].outerPen.widthF() > 0) {
            QPen pen = styleBelow.d->edges[Top].outerPen;

            painter.setPen(pen);
            t += pen.widthF() / 2.0;
            if(isDrawn(d->borderstyle[Top])) {
                drawHorizontalWave(d->borderstyle[Top], painter,x,w,t);
            } else {
                painter.drawLine(QLineF(x, t, x+w, t));
            }
            t = y + styleBelow.d->edges[Top].spacing + pen.widthF();
        }
        // inner line
        if (styleBelow.d->edges[Top].innerPen.widthF() > 0) {
            QPen pen = styleBelow.d->edges[Top].innerPen;
            painter.setPen(pen);
            t += pen.widthF() / 2.0;
            if(isDrawn(d->borderstyle[Top])) {
                drawHorizontalWave(d->borderstyle[Top], painter,x,w,t);
            } else {
                painter.drawLine(QLineF(x, t, x+w, t));
            }
        }
    }
}

void KoTableCellStyle::drawBottomHorizontalBorder(QPainter &painter, qreal x, qreal y, qreal w, QPainterPath *accumulatedBlankBorders) const
{
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
        accumulatedBlankBorders->moveTo(x, t);
        accumulatedBlankBorders->lineTo(x+w, t);

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

void KoTableCellStyle::drawLeftmostVerticalBorder(QPainter &painter, qreal x, qreal y, qreal h, QPainterPath *accumulatedBlankBorders) const
{
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
        accumulatedBlankBorders->moveTo(l, y);
        accumulatedBlankBorders->lineTo(l, y+h);

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

void KoTableCellStyle::drawSharedVerticalBorder(QPainter &painter, const KoTableCellStyle &styleRight,  qreal x, qreal y, qreal h, QPainterPath *accumulatedBlankBorders) const
{
    // First determine which style "wins" by comparing total width
    qreal thisWidth = d->edges[Right].outerPen.widthF() + d->edges[Right].spacing + d->edges[Right].innerPen.widthF();
    qreal thatWidth = styleRight.d->edges[Left].outerPen.widthF() + styleRight.d->edges[Left].spacing
                                    + styleRight.d->edges[Left].innerPen.widthF();

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
            accumulatedBlankBorders->moveTo(l, y);
            accumulatedBlankBorders->lineTo(l, y+h);

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
        if (styleRight.d->edges[Left].outerPen.widthF() > 0) {
            QPen pen = styleRight.d->edges[Left].outerPen;

            painter.setPen(pen);
            l += pen.widthF() / 2.0;
            if(isDrawn(d->borderstyle[Left])) {
                drawVerticalWave(d->borderstyle[Left], painter,y,h,l);
            } else {
                painter.drawLine(QLineF(l, y, l, y+h));
            }
            l += styleRight.d->edges[Left].spacing + pen.widthF() / 2.0;
        }
        // inner line
        if (styleRight.d->edges[Left].innerPen.widthF() > 0) {
            QPen pen = styleRight.d->edges[Left].innerPen;
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

void KoTableCellStyle::drawRightmostVerticalBorder(QPainter &painter, qreal x, qreal y, qreal h, QPainterPath *accumulatedBlankBorders) const
{
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
        accumulatedBlankBorders->moveTo(l, y);
        accumulatedBlankBorders->lineTo(l, y+h);

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

KoTableCellStyle::BorderStyle KoTableCellStyle::oasisBorderStyle(const QString &borderstyle)
{
    if (borderstyle == "none")
        return BorderNone;
    if (borderstyle == "double")
        return BorderDouble;
    if (borderstyle == "dotted")
        return BorderDotted;
    if (borderstyle == "dashed")
        return BorderDashed;
    if (borderstyle == "dash-largegap")
        return BorderDashedLong;
    if (borderstyle == "dot-dash") // not offficially odf, but we suppport it anyway
        return BorderDashDot;
    if (borderstyle == "dot-dot-dash") // not offficially odf, but we suppport it anyway
        return BorderDashDotDot;
    if (borderstyle == "slash") // not offficially odf, but we suppport it anyway
        return BorderSlash;
    if (borderstyle == "wave") // not offficially odf, but we suppport it anyway
        return BorderWave;
    if (borderstyle == "double-wave") // not offficially odf, but we suppport it anyway
        return BorderDoubleWave;
    return BorderSolid; // not needed to handle "solid" since it's the default
}

QString KoTableCellStyle::odfBorderStyleString(const KoTableCellStyle::BorderStyle borderstyle)
{
    switch (borderstyle) {
    case BorderDouble:
        return QString("double");
    case BorderSolid:
        return QString("solid");
    case BorderDashed:
        return QString("dashed");
    case BorderDotted:
        return QString("dotted");
    default:
    case BorderNone:
        return QString("none");
    }
}

void KoTableCellStyle::setParentStyle(KoTableCellStyle *parent)
{
    d->parentStyle = parent;
}

void KoTableCellStyle::setLeftPadding(qreal padding)
{
    setProperty(QTextFormat::TableCellLeftPadding, padding);
}

void KoTableCellStyle::setTopPadding(qreal padding)
{
    setProperty(QTextFormat::TableCellTopPadding, padding);
}

void KoTableCellStyle::setRightPadding(qreal padding)
{
    setProperty(QTextFormat::TableCellRightPadding, padding);
}

void KoTableCellStyle::setBottomPadding(qreal padding)
{
    setProperty(QTextFormat::TableCellBottomPadding, padding);
}

qreal KoTableCellStyle::leftPadding() const
{
    return propertyDouble(QTextFormat::TableCellLeftPadding);
}

qreal KoTableCellStyle::rightPadding() const
{
    return propertyDouble(QTextFormat::TableCellRightPadding);
}

qreal KoTableCellStyle::topPadding() const
{
    return propertyDouble(QTextFormat::TableCellTopPadding);
}

qreal KoTableCellStyle::bottomPadding() const
{
    return propertyDouble(QTextFormat::TableCellBottomPadding);
}

qreal KoTableCellStyle::leftBorderWidth() const
{
    const Edge &edge = d->edges[Left];
    return edge.spacing + edge.innerPen.widthF() + edge.outerPen.widthF();
}

qreal KoTableCellStyle::rightBorderWidth() const
{
    const Edge &edge = d->edges[Right];
    return edge.spacing + edge.innerPen.widthF() + edge.outerPen.widthF();
}

qreal KoTableCellStyle::topBorderWidth() const
{
    const Edge &edge = d->edges[Top];
    return edge.spacing + edge.innerPen.widthF() + edge.outerPen.widthF();
}

qreal KoTableCellStyle::bottomBorderWidth() const
{
    const Edge &edge = d->edges[Bottom];
    return edge.spacing + edge.innerPen.widthF() + edge.outerPen.widthF();
}


void KoTableCellStyle::setPadding(qreal padding)
{
    setBottomPadding(padding);
    setTopPadding(padding);
    setRightPadding(padding);
    setLeftPadding(padding);
}

void KoTableCellStyle::setProperty(int key, const QVariant &value)
{
    if (d->parentStyle) {
        QVariant var = d->parentStyle->value(key);
        if (!var.isNull() && var == value) { // same as parent, so its actually a reset.
            d->stylesPrivate.remove(key);
            return;
        }
    }
    d->stylesPrivate.add(key, value);
}

void KoTableCellStyle::remove(int key)
{
    d->stylesPrivate.remove(key);
}

QVariant KoTableCellStyle::value(int key) const
{
    QVariant var = d->stylesPrivate.value(key);
    if (var.isNull() && d->parentStyle)
        var = d->parentStyle->value(key);
    return var;
}

bool KoTableCellStyle::hasProperty(int key) const
{
    return d->stylesPrivate.contains(key);
}

qreal KoTableCellStyle::propertyDouble(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return 0.0;
    return variant.toDouble();
}

int KoTableCellStyle::propertyInt(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return 0;
    return variant.toInt();
}

bool KoTableCellStyle::propertyBoolean(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return false;
    return variant.toBool();
}

QColor KoTableCellStyle::propertyColor(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull()) {
        return QColor();
    }
    return qvariant_cast<QColor>(variant);
}

void KoTableCellStyle::applyStyle(QTextTableCellFormat &format) const
{
    if (d->parentStyle) {
        d->parentStyle->applyStyle(format);
    }
    QList<int> keys = d->stylesPrivate.keys();
    for (int i = 0; i < keys.count(); i++) {
        QVariant variant = d->stylesPrivate.value(keys[i]);
        format.setProperty(keys[i], variant);
    }

    format.setProperty(TopBorderOuterPen, d->edges[Top].outerPen);
    format.setProperty(TopBorderSpacing,  d->edges[Top].spacing);
    format.setProperty(TopBorderInnerPen, d->edges[Top].innerPen);
    format.setProperty(TopBorderStyle, d->borderstyle[Top]);
    format.setProperty(LeftBorderOuterPen, d->edges[Left].outerPen);
    format.setProperty(LeftBorderSpacing,  d->edges[Left].spacing);
    format.setProperty(LeftBorderInnerPen, d->edges[Left].innerPen);
    format.setProperty(LeftBorderStyle, d->borderstyle[Left]);
    format.setProperty(BottomBorderOuterPen, d->edges[Bottom].outerPen);
    format.setProperty(BottomBorderSpacing,  d->edges[Bottom].spacing);
    format.setProperty(BottomBorderInnerPen, d->edges[Bottom].innerPen);
    format.setProperty(BottomBorderStyle, d->borderstyle[Bottom]);
    format.setProperty(RightBorderOuterPen, d->edges[Right].outerPen);
    format.setProperty(RightBorderSpacing,  d->edges[Right].spacing);
    format.setProperty(RightBorderInnerPen, d->edges[Right].innerPen);
    format.setProperty(RightBorderStyle, d->borderstyle[Right]);
    format.setProperty(TopLeftToBottomRightBorderOuterPen, d->edges[TopLeftToBottomRight].outerPen);
    format.setProperty(TopLeftToBottomRightBorderSpacing,  d->edges[TopLeftToBottomRight].spacing);
    format.setProperty(TopLeftToBottomRightBorderInnerPen, d->edges[TopLeftToBottomRight].innerPen);
    format.setProperty(TopLeftToBottomRightBorderStyle, d->borderstyle[TopLeftToBottomRight]);
    format.setProperty(BottomLeftToTopRightBorderOuterPen, d->edges[BottomLeftToTopRight].outerPen);
    format.setProperty(BottomLeftToTopRightBorderSpacing,  d->edges[BottomLeftToTopRight].spacing);
    format.setProperty(BottomLeftToTopRightBorderInnerPen, d->edges[BottomLeftToTopRight].innerPen);
    format.setProperty(BottomLeftToTopRightBorderStyle, d->borderstyle[BottomLeftToTopRight]);
}

void KoTableCellStyle::setBackground(const QBrush &brush)
{
    setProperty(CellBackgroundBrush, brush);
}

void KoTableCellStyle::clearBackground()
{
    d->stylesPrivate.remove(CellBackgroundBrush);
}

QBrush KoTableCellStyle::background() const
{
    QVariant variant = d->stylesPrivate.value(CellBackgroundBrush);

    if (variant.isNull()) {
        return QBrush();
    }
    return qvariant_cast<QBrush>(variant);
}

void KoTableCellStyle::setAlignment(Qt::Alignment alignment)
{
    setProperty(QTextFormat::BlockAlignment, (int) alignment);
}

Qt::Alignment KoTableCellStyle::alignment() const
{
    return static_cast<Qt::Alignment>(propertyInt(VerticalAlignment));
}

KoTableCellStyle *KoTableCellStyle::parentStyle() const
{
    return d->parentStyle;
}

QString KoTableCellStyle::name() const
{
    return d->name;
}

void KoTableCellStyle::setName(const QString &name)
{
    if (name == d->name)
        return;
    d->name = name;
    emit nameChanged(name);
}

int KoTableCellStyle::styleId() const
{
    return propertyInt(StyleId);
}

void KoTableCellStyle::setStyleId(int id)
{
    setProperty(StyleId, id); if (d->next == 0) d->next = id;
}

QString KoTableCellStyle::masterPageName() const
{
    return value(MasterPageName).toString();
}

void KoTableCellStyle::setMasterPageName(const QString &name)
{
    setProperty(MasterPageName, name);
}

void KoTableCellStyle::loadOdf(const KoXmlElement *element, KoOdfLoadingContext &context)
{
    if (element->hasAttributeNS(KoXmlNS::style, "display-name"))
        d->name = element->attributeNS(KoXmlNS::style, "display-name", QString());

    if (d->name.isEmpty()) // if no style:display-name is given us the style:name
        d->name = element->attributeNS(KoXmlNS::style, "name", QString());

    QString masterPage = element->attributeNS(KoXmlNS::style, "master-page-name", QString());
    if (! masterPage.isEmpty()) {
        setMasterPageName(masterPage);
    }
    context.styleStack().save();
    QString family = element->attributeNS(KoXmlNS::style, "family", "table-cell");
    context.addStyles(element, family.toLocal8Bit().constData());   // Load all parents - only because we don't support inheritance.

    loadOdfProperties(context.styleStack());   // load the KoTableCellStyle from the stylestack
}

void KoTableCellStyle::loadOdfProperties(KoStyleStack &styleStack)
{
    styleStack.setTypeProperties("graphic");
    // Padding
    if (styleStack.hasProperty(KoXmlNS::fo, "padding-left"))
        setLeftPadding(KoUnit::parseValue(styleStack.property(KoXmlNS::fo, "padding-left")));
    if (styleStack.hasProperty(KoXmlNS::fo, "padding-right"))
        setRightPadding(KoUnit::parseValue(styleStack.property(KoXmlNS::fo, "padding-right")));
    if (styleStack.hasProperty(KoXmlNS::fo, "padding-top"))
        setTopPadding(KoUnit::parseValue(styleStack.property(KoXmlNS::fo, "padding-top")));
    if (styleStack.hasProperty(KoXmlNS::fo, "padding-bottom"))
        setBottomPadding(KoUnit::parseValue(styleStack.property(KoXmlNS::fo, "padding-bottom")));
    if (styleStack.hasProperty(KoXmlNS::fo, "padding"))
        setPadding(KoUnit::parseValue(styleStack.property(KoXmlNS::fo, "padding")));

    // The fo:background-color attribute specifies the background color of a cell.
    if (styleStack.hasProperty(KoXmlNS::fo, "background-color")) {
        const QString bgcolor = styleStack.property(KoXmlNS::fo, "background-color");
        QBrush brush = background();
        if (bgcolor == "transparent")
           clearBackground();
        else {
            if (brush.style() == Qt::NoBrush)
                brush.setStyle(Qt::SolidPattern);
            brush.setColor(bgcolor); // #rrggbb format
            setBackground(brush);
        }
    }

    styleStack.setTypeProperties("paragraph");
    // Borders
    if (styleStack.hasProperty(KoXmlNS::fo, "border", "left")) {
        QString border = styleStack.property(KoXmlNS::fo, "border", "left");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KoXmlNS::koffice, "specialborder", "left")) {
            style = styleStack.property(KoXmlNS::koffice, "specialborder", "left");
        }
        if (!border.isEmpty() && border != "none" && border != "hidden") {
            setEdge(Left, oasisBorderStyle(style), KoUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "border", "top")) {
        QString border = styleStack.property(KoXmlNS::fo, "border", "top");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KoXmlNS::koffice, "specialborder", "top")) {
            style = styleStack.property(KoXmlNS::koffice, "specialborder", "top");
        }
        if (!border.isEmpty() && border != "none" && border != "hidden") {
            setEdge(Top, oasisBorderStyle(style), KoUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
        }
    }

    if (styleStack.hasProperty(KoXmlNS::fo, "border", "right")) {
        QString border = styleStack.property(KoXmlNS::fo, "border", "right");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KoXmlNS::koffice, "specialborder", "right")) {
            style = styleStack.property(KoXmlNS::koffice, "specialborder", "right");
        }
        if (!border.isEmpty() && border != "none" && border != "hidden") {
            setEdge(Right, oasisBorderStyle(style), KoUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "border", "bottom")) {
        QString border = styleStack.property(KoXmlNS::fo, "border", "bottom");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KoXmlNS::koffice, "specialborder", "bottom")) {
            style = styleStack.property(KoXmlNS::koffice, "specialborder", "bottom");
        }
        if (!border.isEmpty() && border != "none" && border != "hidden") {
            setEdge(Bottom, oasisBorderStyle(style), KoUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::style, "diagonal-tl-br")) {
        QString border = styleStack.property(KoXmlNS::style, "diagonal-tl-br");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KoXmlNS::koffice, "specialborder", "tl-br")) {
            style = styleStack.property(KoXmlNS::koffice, "specialborder", "tl-br");
        }
        setEdge(TopLeftToBottomRight, oasisBorderStyle(style), KoUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
    }
    if (styleStack.hasProperty(KoXmlNS::style, "diagonal-bl-tr")) {
        QString border = styleStack.property(KoXmlNS::style, "diagonal-bl-tr");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KoXmlNS::koffice, "specialborder", "bl-tr")) {
            style = styleStack.property(KoXmlNS::koffice, "specialborder", "bl-tr");
        }
        setEdge(BottomLeftToTopRight, oasisBorderStyle(style), KoUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
    }

    if (styleStack.hasProperty(KoXmlNS::style, "border-line-width", "left")) {
        QString borderLineWidth = styleStack.property(KoXmlNS::style, "border-line-width", "left");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setEdgeDoubleBorderValues(Left, KoUnit::parseValue(blw[0], 1.0), KoUnit::parseValue(blw[1], 0.1));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::style, "border-line-width", "top")) {
        QString borderLineWidth = styleStack.property(KoXmlNS::style, "border-line-width", "top");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setEdgeDoubleBorderValues(Top, KoUnit::parseValue(blw[0], 1.0), KoUnit::parseValue(blw[1], 0.1));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::style, "border-line-width", "right")) {
        QString borderLineWidth = styleStack.property(KoXmlNS::style, "border-line-width", "right");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setEdgeDoubleBorderValues(Right, KoUnit::parseValue(blw[0], 1.0), KoUnit::parseValue(blw[1], 0.1));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::style, "border-line-width", "bottom")) {
        QString borderLineWidth = styleStack.property(KoXmlNS::style, "border-line-width", "bottom");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setEdgeDoubleBorderValues(Bottom, KoUnit::parseValue(blw[0], 1.0), KoUnit::parseValue(blw[1], 0.1));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::style, "diagonal-tl-br-widths")) {
        QString borderLineWidth = styleStack.property(KoXmlNS::style, "diagonal-tl-br-widths");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setEdgeDoubleBorderValues(TopLeftToBottomRight, KoUnit::parseValue(blw[0], 1.0), KoUnit::parseValue(blw[1], 0.1));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::style, "diagonal-bl-tr-widths")) {
        QString borderLineWidth = styleStack.property(KoXmlNS::style, "diagonal-bl-tr-widths");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setEdgeDoubleBorderValues(BottomLeftToTopRight, KoUnit::parseValue(blw[0], 1.0), KoUnit::parseValue(blw[1], 0.1));
        }
    }

    // Alignment
    const QString verticalAlign(styleStack.property(KoXmlNS::style, "vertical-align"));
    if (!verticalAlign.isEmpty()) {
        setAlignment(KoText::valignmentFromString(verticalAlign));
    }
    styleStack.restore();
}

void KoTableCellStyle::copyProperties(const KoTableCellStyle *style)
{
    d->stylesPrivate = style->d->stylesPrivate;
    setName(style->name()); // make sure we emit property change
    d->next = style->d->next;
    d->parentStyle = style->d->parentStyle;
}

KoTableCellStyle *KoTableCellStyle::clone(QObject *parent)
{
    KoTableCellStyle *newStyle = new KoTableCellStyle(parent);
    newStyle->copyProperties(this);
    return newStyle;
}


bool KoTableCellStyle::operator==(const KoTableCellStyle &other) const
{

    return other.d->stylesPrivate == d->stylesPrivate;
}

void KoTableCellStyle::removeDuplicates(const KoTableCellStyle &other)
{
    d->stylesPrivate.removeDuplicates(other.d->stylesPrivate);
}

void KoTableCellStyle::saveOdf(KoGenStyle &style)
{
    Q_UNUSED(style);
/*
    QList<int> keys = d->stylesPrivate.keys();
    foreach(int key, keys) {
        if (key == QTextFormat::BlockAlignment) {
            int alignValue = 0;
            bool ok = false;
            alignValue = d->stylesPrivate.value(key).toInt(&ok);
            if (ok) {
                Qt::Alignment alignment = (Qt::Alignment) alignValue;
                QString align = KoText::alignmentToString(alignment);
                if (!align.isEmpty())
                    style.addProperty("fo:text-align", align, KoGenStyle::ParagraphType);
            }
        } else if (key == KoTableCellStyle::TextProgressionDirection) {
            int directionValue = 0;
            bool ok = false;
            directionValue = d->stylesPrivate.value(key).toInt(&ok);
            if (ok) {
                QString direction = "";
                if (directionValue == KoText::LeftRightTopBottom)
                    direction = "lr";
                else if (directionValue == KoText::RightLeftTopBottom)
                    direction = "rl";
                else if (directionValue == KoText::TopBottomRightLeft)
                    direction = "tb";
                if (!direction.isEmpty())
                    style.addProperty("style:writing-mode", direction, KoGenStyle::ParagraphType);
            }
        } else if (key == CellBackgroundBrush) {
            QBrush backBrush = background();
            if (backBrush.style() != Qt::NoBrush)
                style.addProperty("fo:background-color", backBrush.color().name(), KoGenStyle::ParagraphType);
            else
                style.addProperty("fo:background-color", "transparent", KoGenStyle::ParagraphType);

    // Border
    QString leftBorder = QString("%1pt %2 %3").arg(QString::number(leftBorderWidth()),
                         odfBorderStyleString(leftBorderStyle()),
                         leftBorderColor().name());
    QString rightBorder = QString("%1pt %2 %3").arg(QString::number(rightBorderWidth()),
                          odfBorderStyleString(rightBorderStyle()),
                          rightBorderColor().name());
    QString topBorder = QString("%1pt %2 %3").arg(QString::number(topBorderWidth()),
                        odfBorderStyleString(topBorderStyle()),
                        topBorderColor().name());
    QString bottomBorder = QString("%1pt %2 %3").arg(QString::number(bottomBorderWidth()),
                           odfBorderStyleString(bottomBorderStyle()),
                           bottomBorderColor().name());
    if (leftBorder == rightBorder && leftBorder == topBorder && leftBorder == bottomBorder) {
        if (leftBorderWidth() > 0 && leftBorderStyle() != KoParagraphStyle::BorderNone)
            style.addProperty("fo:border", leftBorder, KoGenStyle::ParagraphType);
    } else {
        if (leftBorderWidth() > 0 && leftBorderStyle() != KoParagraphStyle::BorderNone)
            style.addProperty("fo:border-left", leftBorder, KoGenStyle::ParagraphType);
        if (rightBorderWidth() > 0 && rightBorderStyle() != KoParagraphStyle::BorderNone)
            style.addProperty("fo:border-right", rightBorder, KoGenStyle::ParagraphType);
        if (topBorderWidth() > 0 && topBorderStyle() != KoParagraphStyle::BorderNone)
            style.addProperty("fo:border-top", topBorder, KoGenStyle::ParagraphType);
        if (bottomBorderWidth() > 0 && bottomBorderStyle() != KoParagraphStyle::BorderNone)
            style.addProperty("fo:border-bottom", bottomBorder, KoGenStyle::ParagraphType);
    }
    QString leftBorderLineWidth = QString("%1pt %2pt %3pt").arg(QString::number(leftInnerBorderWidth()),
                                  QString::number(leftBorderSpacing()),
                                  QString::number(leftBorderWidth()));
    QString rightBorderLineWidth = QString("%1pt %2pt %3pt").arg(QString::number(rightInnerBorderWidth()),
                                   QString::number(rightBorderSpacing()),
                                   QString::number(rightBorderWidth()));
    QString topBorderLineWidth = QString("%1pt %2pt %3pt").arg(QString::number(topInnerBorderWidth()),
                                 QString::number(topBorderSpacing()),
                                 QString::number(topBorderWidth()));
    QString bottomBorderLineWidth = QString("%1pt %2pt %3pt").arg(QString::number(bottomInnerBorderWidth()),
                                    QString::number(bottomBorderSpacing()),
                                    QString::number(bottomBorderWidth()));
    if (leftBorderLineWidth == rightBorderLineWidth &&
            leftBorderLineWidth == topBorderLineWidth &&
            leftBorderLineWidth == bottomBorderLineWidth &&
            leftBorderStyle() == KoParagraphStyle::BorderDouble &&
            rightBorderStyle() == KoParagraphStyle::BorderDouble &&
            topBorderStyle() == KoParagraphStyle::BorderDouble &&
            bottomBorderStyle() == KoParagraphStyle::BorderDouble) {
        style.addProperty("style:border-line-width", leftBorderLineWidth, KoGenStyle::ParagraphType);
    } else {
        if (leftBorderStyle() == KoParagraphStyle::BorderDouble)
            style.addProperty("style:border-line-width-left", leftBorderLineWidth, KoGenStyle::ParagraphType);
        if (rightBorderStyle() == KoParagraphStyle::BorderDouble)
            style.addProperty("style:border-line-width-right", rightBorderLineWidth, KoGenStyle::ParagraphType);
        if (topBorderStyle() == KoParagraphStyle::BorderDouble)
            style.addProperty("style:border-line-width-top", topBorderLineWidth, KoGenStyle::ParagraphType);
        if (bottomBorderStyle() == KoParagraphStyle::BorderDouble)
            style.addProperty("style:border-line-width-bottom", bottomBorderLineWidth, KoGenStyle::ParagraphType);
    }
*/
}

#include <KoTableCellStyle.moc>
