/* This file is part of the KDE project
 *
 * Copyright (C) 2009 Inge Wallin <inge@lysator.liu.se>
 * Copyright (C) 2009 Thomas Zander <zander@kde.org>
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

#include "KoBorder.h"

#include <QPainter>

#include <OdfDebug.h>

#include <KoUnit.h>
#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include <KoStyleStack.h>


class KoBorderPrivate : public QSharedData
{
public:
    KoBorderPrivate();
    ~KoBorderPrivate();

    QMap<KoBorder::BorderSide, KoBorder::BorderData> data;
};

KoBorderPrivate::KoBorderPrivate()
{
}

KoBorderPrivate::~KoBorderPrivate()
{
}

KoBorder::BorderData::BorderData()
    : style(KoBorder::BorderNone)
    , outerPen(QPen())
    , innerPen(QPen())
    , spacing(0)
{
    outerPen.setWidthF(0.0f);
    innerPen.setWidthF(0.0f);
}

bool KoBorder::BorderData::operator==(const KoBorder::BorderData& other) const
{
    // Left Borders
    if (style == BorderNone && other.style == BorderNone) {
        // If both styles are None, then the rest of the values don't
        // need to be compared.
        ;
    }
    else if (style != other.style) {
        // If any of them are non-None, and they are different, the
        // borders are also different.
        return false;
    }
    else {
        // Here we know that the border styles are the same, now
        // compare the rest of the values.
        if (outerPen != other.outerPen)
            return false;

        // If the border style == BorderDouble, then compare a couple
        // of other values too.
        if (style == BorderDouble) {
            if (innerPen != other.innerPen)
                return false;
            if (spacing != other.spacing)
                return false;
        }
    }

    return true;
}

// ----------------------------------------------------------------

KoBorder::KoBorder()
    : d(new KoBorderPrivate)
{
}

KoBorder::KoBorder(const KoBorder &kb)
    : d(kb.d)
{
}

KoBorder::~KoBorder()
{
    // No delete because d is a QSharedDataPointer.
}


// ----------------------------------------------------------------
//                             operators

KoBorder &KoBorder::operator=(const KoBorder &other)
{
    d = other.d;

    return *this;
}

bool KoBorder::operator==(const KoBorder &other) const
{
    if (d.data() == other.d.data())
        return true;


    if (d->data.size() != other.d->data.size())
        return false;

    KoBorder::BorderSide key;

    foreach (key, d->data.keys()) {
        if (!other.d->data.contains(key))
            return false;
        if (!(other.d->data[key] == d->data[key]))
            return false;
    }

    return true;
}


// ----------------------------------------------------------------
//                 public, non-class functions

KoBorder::BorderStyle KoBorder::odfBorderStyle(const QString &borderstyle, bool *converted)
{
    // Note: the styles marked "Not odf compatible" below are legacies
    //       from the old words format.  There are also lots of border
    //       styles in the MS DOC that we may have to handle at some point.
    if (converted)
        *converted = true;
    if (borderstyle == "none")
        return BorderNone;
    if (borderstyle == "solid")
        return BorderSolid;
    if (borderstyle == "dashed")
        return BorderDashed;
    if (borderstyle == "dotted")
        return BorderDotted;
    if (borderstyle == "dot-dash")
        return BorderDashDot;
    if (borderstyle == "dot-dot-dash")
        return BorderDashDotDot;
    if (borderstyle == "double")
        return BorderDouble;
    if (borderstyle == "groove")   // Not odf compatible -- see above
        return BorderGroove;
    if (borderstyle == "ridge")   // Not odf compatible -- see above
        return BorderRidge;
    if (borderstyle == "inset")   // Not odf compatible -- see above
        return BorderInset;
    if (borderstyle == "outset")   // Not odf compatible -- see above
        return BorderOutset;
    if (borderstyle == "dash-largegap")
        return KoBorder::BorderDashedLong;
    if (borderstyle == "slash") // not officially odf, but we support it anyway
        return KoBorder::BorderSlash;
    if (borderstyle == "wave") // not officially odf, but we support it anyway
        return KoBorder::BorderWave;
    if (borderstyle == "double-wave") // not officially odf, but we support it anyway
        return KoBorder::BorderDoubleWave;

    if (converted)
        *converted = false;

    return BorderSolid;
}

QString KoBorder::odfBorderStyleString(BorderStyle borderstyle)
{
    switch (borderstyle) {
    case BorderDashed:
        return QString("dashed");
    case BorderDotted:
        return QString("dotted");
    case BorderDashDot:
        return QString("dot-dash");
    case BorderDashDotDot:
        return QString("dot-dot-dash");
    case BorderDouble:
        return QString("double");
    case BorderGroove:
        return QString("groove"); // not odf -- see above
    case BorderRidge:
        return QString("ridge"); // not odf -- see above
    case BorderInset:
        return QString("inset"); // not odf -- see above
    case BorderOutset:
        return QString("outset"); // not odf -- see above
    case BorderSolid:
        return QString("solid");
    case BorderNone:
        return QString("none");

    default:
        // Handle unknown types as solid.
        return QString("solid");
    }
}

QString KoBorder::msoBorderStyleString(BorderStyle borderstyle)
{
    switch (borderstyle) {
    case KoBorder::BorderDashedLong:
        return QString("dash-largegap");
    case KoBorder::BorderSlash:
        return QString("slash"); // not officially odf, but we support it anyway
    case KoBorder::BorderWave:
        return QString("wave"); // not officially odf, but we support it anyway
    case KoBorder::BorderDoubleWave:
        return QString("double-wave"); // not officially odf, but we support it anyway

    default:
        // Handle remaining styles as odf type style.
        return odfBorderStyleString(borderstyle);
    }
}


// ----------------------------------------------------------------
//                         Getters and Setters


void KoBorder::setBorderStyle(BorderSide side, BorderStyle style)
{
    if (d->data[side].style == style) {
        return;
    }

    if (!d->data.contains(side)) {
        BorderData data;
        data.style = style;
        d->data[side] = data;
    } else {
        d->data[side].style = style;
    }

    // Make a best effort to create the best possible dash pattern for the chosen style.
    // FIXME: KoTableCellStyle::setEdge() should call this function.
    BorderData &edge = d->data[side];
    qreal width = edge.outerPen.widthF();
    qreal innerWidth = 0;
    qreal middleWidth = 0;
    qreal space = 0;
    QVector<qreal> dashes;
    switch (style) {
    case KoBorder::BorderNone:
        width = 0.0;
        break;
    case KoBorder::BorderDouble:
        innerWidth = space = edge.outerPen.width() / 3; //some nice default look
        width -= (space + innerWidth);
        edge.outerPen.setStyle(Qt::SolidLine);
        break;
    case KoBorder::BorderDotted:
        dashes << 1 << 1;
        edge.outerPen.setDashPattern(dashes);
        break;
    case KoBorder::BorderDashed:
        dashes << 4 << 1;
        edge.outerPen.setDashPattern(dashes);
        break;
    case KoBorder::BorderDashedLong: {
        dashes << 4 << 4;
        edge.outerPen.setDashPattern(dashes);
        break;
    }
    case KoBorder::BorderTriple:
        innerWidth = middleWidth = space = width/6;
        width -= (space + innerWidth);
        edge.outerPen.setStyle(Qt::SolidLine);
        break;
    case KoBorder::BorderDashDot:
        dashes << 3 << 3<< 7 << 3;
        edge.outerPen.setDashPattern(dashes);
        break;
    case KoBorder::BorderDashDotDot:
        dashes << 2 << 2<< 6 << 2 << 2 << 2;
        edge.outerPen.setDashPattern(dashes);
        break;
    case KoBorder::BorderWave:
        edge.outerPen.setStyle(Qt::SolidLine);
        break;
    case KoBorder::BorderSlash:
        edge.outerPen.setStyle(Qt::SolidLine);
        break;
    case KoBorder::BorderDoubleWave:
        innerWidth = space = width/3; //some nice default look
        width -= (space + innerWidth);
        edge.outerPen.setStyle(Qt::SolidLine);
        break;
    default:
        edge.outerPen.setStyle(Qt::SolidLine);
        break;
    }
    edge.outerPen.setJoinStyle(Qt::MiterJoin);
    edge.outerPen.setCapStyle(Qt::FlatCap);
    edge.outerPen.setWidthF(width);

    edge.spacing = space;
    edge.innerPen = edge.outerPen;
    edge.innerPen.setWidthF(innerWidth);
}

KoBorder::BorderStyle KoBorder::borderStyle(BorderSide side) const
{
    if (!d->data.contains(side)) {
        return BorderNone;
    } else {
        return d->data[side].style;
    }
}

void KoBorder::setBorderColor(BorderSide side, const QColor &color)
{
    if (!d->data.contains(side)) {
        BorderData data;
        data.outerPen.setColor(color);
        d->data[side] = data;
    } else {
        d->data[side].outerPen.setColor(color);
    }
}

QColor KoBorder::borderColor(BorderSide side) const
{
    if (!d->data.contains(side)) {
        return QColor();
    } else {
        return d->data[side].outerPen.color();
    }
}

void KoBorder::setBorderWidth(BorderSide side, qreal width)
{
    if (!d->data.contains(side)) {
        BorderData data;
        data.outerPen.setWidthF(width);
        d->data[side] = data;
    } else {
        d->data[side].outerPen.setWidthF(width);
    }
}

qreal KoBorder::borderWidth(BorderSide side) const
{
    if (!d->data.contains(side)) {
        return 0;
    } else {
        if (d->data[side].style == BorderDouble)
            return (d->data[side].outerPen.widthF() + d->data[side].innerPen.widthF()
                    + d->data[side].spacing);
        else
            return d->data[side].outerPen.widthF();
    }
}

void KoBorder::setOuterBorderWidth(BorderSide side, qreal width)
{
    if (!d->data.contains(side)) {
        BorderData data;
        data.outerPen.setWidthF(width);
        d->data[side] = data;
    } else {
        d->data[side].outerPen.setWidthF(width);
    }
}

qreal KoBorder::outerBorderWidth(BorderSide side) const
{
    if (!d->data.contains(side)) {
        return 0;
    } else {
        return d->data[side].outerPen.widthF();
    }
}

void KoBorder::setInnerBorderWidth(BorderSide side, qreal width)
{
    if (!d->data.contains(side)) {
        BorderData data;
        data.innerPen.setWidthF(width);
        d->data[side] = data;
    } else {
        d->data[side].innerPen.setWidthF(width);
    }
}

qreal KoBorder::innerBorderWidth(BorderSide side) const
{
    if (!d->data.contains(side)) {
        return 0;
    } else {
        return d->data[side].innerPen.widthF();
    }
}

void KoBorder::setBorderSpacing(BorderSide side, qreal width)
{
    if (!d->data.contains(side)) {
        BorderData data;
        data.spacing = width;
        d->data[side] = data;
    } else {
        d->data[side].spacing = width;
    }
}

qreal KoBorder::borderSpacing(BorderSide side) const
{
    if (!d->data.contains(side)) {
        return 0;
    } else {
        return d->data[side].spacing;
    }
}


KoBorder::BorderData KoBorder::borderData(BorderSide side) const
{
    return d->data.value(side, BorderData());
}

void KoBorder::setBorderData(BorderSide side, const BorderData &data)
{
    d->data[side] = data;
}


// -------------------------------

bool KoBorder::hasBorder() const
{
    if (borderStyle(LeftBorder) != BorderNone && borderWidth(LeftBorder) > 0.0)
        return true;
    if (borderStyle(RightBorder) != BorderNone && borderWidth(RightBorder) > 0.0)
        return true;
    if (borderStyle(TopBorder) != BorderNone && borderWidth(TopBorder) > 0.0)
        return true;
    if (borderStyle(BottomBorder) != BorderNone && borderWidth(BottomBorder) > 0.0)
        return true;
    if (borderStyle(TlbrBorder) != BorderNone && borderWidth(TlbrBorder) > 0.0)
        return true;
    if (borderStyle(BltrBorder) != BorderNone && borderWidth(BltrBorder) > 0.0)
        return true;
    return false;
}

bool KoBorder::hasBorder(KoBorder::BorderSide side) const
{
    return borderStyle(side) != BorderNone && borderWidth(side) > 0.0;
}


// ----------------------------------------------------------------
//                         painting


void KoBorder::paint(QPainter &painter, const QRectF &borderRect,
                     BorderPaintArea whereToPaint) const
{
    Q_UNUSED(whereToPaint);
    // In tables it is apparently best practice to paint the
    // horizontal lines over the vertical ones.  So let's use the same
    // strategy here.

    QPointF start;
    QPointF end;

    // FIXME: Make KoBorder store pointers to BorderData instead.  This is very inefficient.
    BorderData leftEdge = borderData(KoBorder::LeftBorder);
    BorderData rightEdge = borderData(KoBorder::RightBorder);
    BorderData topEdge = borderData(KoBorder::TopBorder);
    BorderData bottomEdge = borderData(KoBorder::BottomBorder);

    // Left border
    if (hasBorder(LeftBorder)) {
        start = borderRect.topLeft();
        end   = borderRect.bottomLeft();
        paintBorderSide(painter, start, end, &leftEdge, true,
                        hasBorder(TopBorder) ? &topEdge : 0,
                        hasBorder(BottomBorder) ? &bottomEdge : 0,
                        1);
    }

    // Right border
    if (hasBorder(RightBorder)) {
        start = borderRect.topRight();
        end   = borderRect.bottomRight();
        paintBorderSide(painter, start, end, &rightEdge, true,
                        hasBorder(TopBorder) ? &topEdge : 0,
                        hasBorder(BottomBorder) ? &bottomEdge : 0,
                        -1);
    }

    // Top border
    if (hasBorder(TopBorder)) {
        start = borderRect.topLeft();
        end   = borderRect.topRight();
        paintBorderSide(painter, start, end, &topEdge, false,
                        hasBorder(LeftBorder) ? &leftEdge : 0,
                        hasBorder(RightBorder) ? &rightEdge : 0,
                        1);
    }

    // Bottom border
    if (hasBorder(BottomBorder)) {
        start = borderRect.bottomLeft();
        end   = borderRect.bottomRight();
        paintBorderSide(painter, start, end, &bottomEdge, false,
                        hasBorder(LeftBorder) ? &leftEdge : 0,
                        hasBorder(RightBorder) ? &rightEdge : 0,
                        -1);
    }

    // FIXME: Diagonal borders
}

void KoBorder::paintBorderSide(QPainter &painter, QPointF lineStart, QPointF lineEnd,
                               BorderData *borderData, bool isVertical,
                               BorderData *neighbour1, BorderData *neighbour2,
                               int inwardsAcross) const
{
    // Adjust the outer line so that it is inside the boundary.
    qreal displacement = borderData->outerPen.widthF() / qreal(2.0);
    if (isVertical) {
        lineStart.setX(lineStart.x() + inwardsAcross * displacement);
        lineEnd.setX(lineEnd.x() + inwardsAcross * displacement);
    }
    else {
        lineStart.setY(lineStart.y() + inwardsAcross * displacement);
        lineEnd.setY(lineEnd.y() + inwardsAcross * displacement);
    }

    painter.setPen(borderData->outerPen);
    painter.drawLine(lineStart, lineEnd);

    if (borderData->style == BorderDouble) {
        displacement = (borderData->outerPen.widthF() / qreal(2.0)
                        + borderData->spacing
                        + borderData->innerPen.widthF() / qreal(2.0));
        if (isVertical) {
            lineStart.setX(lineStart.x() + inwardsAcross * displacement);
            lineEnd.setX(lineEnd.x() + inwardsAcross * displacement);
        }
        else {
            lineStart.setY(lineStart.y() + inwardsAcross * displacement);
            lineEnd.setY(lineEnd.y() + inwardsAcross * displacement);
        }

        // Adjust for neighboring inner lines.
        if (neighbour1 && neighbour1->style == BorderDouble) {
            displacement = neighbour1->outerPen.widthF() + neighbour1->spacing;
            if (isVertical) {
                lineStart.setY(lineStart.y() + displacement);
            }
            else {
                lineStart.setX(lineStart.x() + displacement);
            }
        }
        if (neighbour2 && neighbour2->style == BorderDouble) {
            displacement = neighbour2->outerPen.widthF() + neighbour2->spacing;
            if (isVertical) {
                lineEnd.setY(lineEnd.y() - displacement);
            }
            else {
                lineEnd.setX(lineEnd.x() - displacement);
            }
        }

        // Draw the inner line.
        painter.setPen(borderData->innerPen);
        painter.drawLine(lineStart, lineEnd);
    }
}
