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


// ----------------------------------------------------------------
//                         static functions


void parseOdfBorder(const QString &border, QColor *color,
                    KoBorder::BorderStyle *borderStyle, bool *hasBorderStyle,
                    qreal *borderWidth, bool *hasBorderWidth)
{
    *hasBorderStyle = false;
    *hasBorderWidth = false;

    if (!border.isEmpty() && border != "none" && border != "hidden") {
        QStringList borderData = border.split(' ', QString::SkipEmptyParts);
        if (borderData.length() > 0)
        {
            const QColor borderColor = QColor(borderData.last());
            if (borderColor.isValid()) {
                *color = borderColor;
                borderData.removeLast();
            }

            bool converted = false;
            const KoBorder::BorderStyle parsedBorderStyle = KoBorder::odfBorderStyle(borderData.last(), &converted);
            if (converted) {
                *hasBorderStyle = true;
                borderData.removeLast();
                *borderStyle = parsedBorderStyle;
            }

            if (!borderData.isEmpty()) {
                const qreal parsedBorderWidth = KoUnit::parseValue(borderData[0], 1.0);
                *borderWidth = parsedBorderWidth;
                *hasBorderWidth = true;
            }
        }
    }
}

// ----------------------------------------------------------------
//                         load and save

bool KoBorder::loadOdf(const KoXmlElement &style)
{
    bool result = false;

    QString borderString;
    bool hasSpecialBorder;
    QString specialBorderString;
    if (style.hasAttributeNS(KoXmlNS::fo, "border")) {
        borderString = style.attributeNS(KoXmlNS::fo, "border");
        if (borderString == "none") {
            // We use the "false" to indicate that there is no border
            // rather than that the parsing has failed.
            return false;
        }

        result = true;
        if ((hasSpecialBorder = style.hasAttributeNS(KoXmlNS::calligra, "specialborder"))) {
            specialBorderString = style.attributeNS(KoXmlNS::calligra, "specialborder");
        }
        parseAndSetBorder(borderString, hasSpecialBorder, specialBorderString);
    }
    else {
        // No common border attributes, check for the individual ones.
        if (style.hasAttributeNS(KoXmlNS::fo, "border-left")) {
            result = true;
            borderString = style.attributeNS(KoXmlNS::fo, "border-left");
            if ((hasSpecialBorder = style.hasAttributeNS(KoXmlNS::calligra, "specialborder-left"))) {
                specialBorderString = style.attributeNS(KoXmlNS::calligra, "specialborder-left");
            }
            parseAndSetBorder(LeftBorder, borderString, hasSpecialBorder, specialBorderString);
        }
        if (style.hasAttributeNS(KoXmlNS::fo, "border-top")) {
            result = true;
            borderString = style.attributeNS(KoXmlNS::fo, "border-top");
            if ((hasSpecialBorder = style.hasAttributeNS(KoXmlNS::calligra, "specialborder-top"))) {
                specialBorderString = style.attributeNS(KoXmlNS::calligra, "specialborder-top");
            }
            parseAndSetBorder(TopBorder, borderString, hasSpecialBorder, specialBorderString);
        }
        if (style.hasAttributeNS(KoXmlNS::fo, "border-right")) {
            result = true;
            borderString = style.attributeNS(KoXmlNS::fo, "border-right");
            if ((hasSpecialBorder = style.hasAttributeNS(KoXmlNS::calligra, "specialborder-right"))) {
                specialBorderString = style.attributeNS(KoXmlNS::calligra, "specialborder-right");
            }
            parseAndSetBorder(RightBorder, borderString, hasSpecialBorder, specialBorderString);
        }
        if (style.hasAttributeNS(KoXmlNS::fo, "border-bottom")) {
            result = true;
            borderString = style.attributeNS(KoXmlNS::fo, "border-bottom");
            if ((hasSpecialBorder = style.hasAttributeNS(KoXmlNS::calligra, "specialborder-bottom"))) {
                specialBorderString = style.attributeNS(KoXmlNS::calligra, "specialborder-bottom");
            }
            parseAndSetBorder(BottomBorder, borderString, hasSpecialBorder, specialBorderString);
        }
    }

    // Diagonals are treated individually and are NOT part of <style:border>.
    if (style.hasAttributeNS(KoXmlNS::style, "diagonal-tl-br")) {
        result = true;
        borderString = style.attributeNS(KoXmlNS::fo, "border-tl-br");
        if ((hasSpecialBorder = style.hasAttributeNS(KoXmlNS::calligra, "specialborder-tl-br"))) {
            specialBorderString = style.attributeNS(KoXmlNS::calligra, "specialborder-tl-br");
        }
        parseAndSetBorder(TlbrBorder, borderString, hasSpecialBorder, specialBorderString);
    }
    if (style.hasAttributeNS(KoXmlNS::style, "diagonal-bl-tr")) {
        result = true;
        borderString = style.attributeNS(KoXmlNS::fo, "border-bl-tr");
        if ((hasSpecialBorder = style.hasAttributeNS(KoXmlNS::calligra, "specialborder-bl-tr"))) {
            specialBorderString = style.attributeNS(KoXmlNS::calligra, "specialborder-bl-tr");
        }
        parseAndSetBorder(BltrBorder, borderString, hasSpecialBorder, specialBorderString);
    }

    // Handle double borders.
    if (style.hasAttributeNS(KoXmlNS::style, "border-line-width")) {
        result = true;
        QString borderLineWidth = style.attributeNS(KoXmlNS::style, "border-line-width");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setInnerBorderWidth(LeftBorder, KoUnit::parseValue(blw[0], 0.1));
            setBorderSpacing(LeftBorder, KoUnit::parseValue(blw[1], 1.0));
            setBorderWidth(LeftBorder, KoUnit::parseValue(blw[2], 0.1));

            setInnerBorderWidth(TopBorder, KoUnit::parseValue(blw[0], 0.1));
            setBorderSpacing(TopBorder, KoUnit::parseValue(blw[1], 1.0));
            setBorderWidth(TopBorder, KoUnit::parseValue(blw[2], 0.1));

            setInnerBorderWidth(RightBorder, KoUnit::parseValue(blw[0], 0.1));
            setBorderSpacing(RightBorder, KoUnit::parseValue(blw[1], 1.0));
            setBorderWidth(RightBorder, KoUnit::parseValue(blw[2], 0.1));

            setInnerBorderWidth(BottomBorder, KoUnit::parseValue(blw[0], 0.1));
            setBorderSpacing(BottomBorder, KoUnit::parseValue(blw[1], 1.0));
            setBorderWidth(BottomBorder, KoUnit::parseValue(blw[2], 0.1));
        }
    }
    else {
        if (style.hasAttributeNS(KoXmlNS::style, "border-line-width-left")) {
            result = true;
            QString borderLineWidth = style.attributeNS(KoXmlNS::style, "border-line-width-left");
            if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
                QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
                setInnerBorderWidth(LeftBorder, KoUnit::parseValue(blw[0], 0.1));
                setBorderSpacing(LeftBorder, KoUnit::parseValue(blw[1], 1.0));
                setBorderWidth(LeftBorder, KoUnit::parseValue(blw[2], 0.1));
            }
        }
        if (style.hasAttributeNS(KoXmlNS::style, "border-line-width-top")) {
            result = true;
            QString borderLineWidth = style.attributeNS(KoXmlNS::style, "border-line-width-top");
            if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
                QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
                setInnerBorderWidth(TopBorder, KoUnit::parseValue(blw[0], 0.1));
                setBorderSpacing(TopBorder, KoUnit::parseValue(blw[1], 1.0));
                setBorderWidth(TopBorder, KoUnit::parseValue(blw[2], 0.1));
            }
        }
        if (style.hasAttributeNS(KoXmlNS::style, "border-line-width-right")) {
            result = true;
            QString borderLineWidth = style.attributeNS(KoXmlNS::style, "border-line-width-right");
            if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
                QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
                setInnerBorderWidth(RightBorder, KoUnit::parseValue(blw[0], 0.1));
                setBorderSpacing(RightBorder, KoUnit::parseValue(blw[1], 1.0));
                setBorderWidth(RightBorder, KoUnit::parseValue(blw[2], 0.1));
            }
        }
        if (style.hasAttributeNS(KoXmlNS::style, "border-line-width-bottom")) {
            result = true;
            QString borderLineWidth = style.attributeNS(KoXmlNS::style, "border-line-width-bottom");
            if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
                QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
                setInnerBorderWidth(BottomBorder, KoUnit::parseValue(blw[0], 0.1));
                setBorderSpacing(BottomBorder, KoUnit::parseValue(blw[1], 1.0));
                setBorderWidth(BottomBorder, KoUnit::parseValue(blw[2], 0.1));
            }
        }
    }

    if (style.hasAttributeNS(KoXmlNS::style, "diagonal-tl-br-widths")) {
        result = true;
        QString borderLineWidth = style.attributeNS(KoXmlNS::style, "diagonal-tl-br-widths");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setInnerBorderWidth(TlbrBorder, KoUnit::parseValue(blw[0], 0.1));
            setBorderSpacing(TlbrBorder, KoUnit::parseValue(blw[1], 1.0));
            setBorderWidth(TlbrBorder, KoUnit::parseValue(blw[2], 0.1));
        }
    }
    if (style.hasAttributeNS(KoXmlNS::style, "diagonal-bl-tr-widths")) {
        result = true;
        QString borderLineWidth = style.attributeNS(KoXmlNS::style, "diagonal-bl-tr-widths");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setInnerBorderWidth(BltrBorder, KoUnit::parseValue(blw[0], 0.1));
            setBorderSpacing(BltrBorder, KoUnit::parseValue(blw[1], 1.0));
            setBorderWidth(BltrBorder, KoUnit::parseValue(blw[2], 0.1));
        }
    }
    return result;
}

bool KoBorder::loadOdf(const KoStyleStack &styleStack)
{
    bool result = false;

    QString borderString;
    bool hasSpecialBorder;
    QString specialBorderString;
    if (styleStack.hasProperty(KoXmlNS::fo, "border")) {
        result = true;
        borderString = styleStack.property(KoXmlNS::fo, "border");
        if ((hasSpecialBorder = styleStack.hasProperty(KoXmlNS::calligra, "specialborder"))) {
            specialBorderString = styleStack.property(KoXmlNS::calligra, "specialborder");
        }
        parseAndSetBorder(borderString, hasSpecialBorder, specialBorderString);
    }

    // Even if there are common border attributes, check for the
    // individual ones since they have precedence.

    if (styleStack.hasProperty(KoXmlNS::fo, "border-left")) {
        result = true;
        borderString = styleStack.property(KoXmlNS::fo, "border-left");
        if ((hasSpecialBorder = styleStack.hasProperty(KoXmlNS::calligra, "specialborder-left"))) {
            specialBorderString = styleStack.property(KoXmlNS::calligra, "specialborder-left");
        }
        parseAndSetBorder(LeftBorder, borderString, hasSpecialBorder, specialBorderString);
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "border-top")) {
        result = true;
        borderString = styleStack.property(KoXmlNS::fo, "border-top");
        if ((hasSpecialBorder = styleStack.hasProperty(KoXmlNS::calligra, "specialborder-top"))) {
            specialBorderString = styleStack.property(KoXmlNS::calligra, "specialborder-top");
        }
        parseAndSetBorder(TopBorder, borderString, hasSpecialBorder, specialBorderString);
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "border-right")) {
        result = true;
        borderString = styleStack.property(KoXmlNS::fo, "border-right");
        if ((hasSpecialBorder = styleStack.hasProperty(KoXmlNS::calligra, "specialborder-right"))) {
            specialBorderString = styleStack.property(KoXmlNS::calligra, "specialborder-right");
        }
        parseAndSetBorder(RightBorder, borderString, hasSpecialBorder, specialBorderString);

    }
    if (styleStack.hasProperty(KoXmlNS::fo, "border-bottom")) {
        result = true;
        borderString = styleStack.property(KoXmlNS::fo, "border-bottom");
        if ((hasSpecialBorder = styleStack.hasProperty(KoXmlNS::calligra, "specialborder-bottom"))) {
            specialBorderString = styleStack.property(KoXmlNS::calligra, "specialborder-bottom");
        }
        parseAndSetBorder(BottomBorder, borderString, hasSpecialBorder, specialBorderString);
    }

    // Diagonals are treated individually and are NOT part of <style:border>.
    if (styleStack.hasProperty(KoXmlNS::style, "diagonal-tl-br")) {
        result = true;
        borderString = styleStack.property(KoXmlNS::fo, "border-tl-br");
        if ((hasSpecialBorder = styleStack.hasProperty(KoXmlNS::calligra, "specialborder-tl-br"))) {
            specialBorderString = styleStack.property(KoXmlNS::calligra, "specialborder-tl-br");
        }
        parseAndSetBorder(TlbrBorder, borderString, hasSpecialBorder, specialBorderString);
    }
    if (styleStack.hasProperty(KoXmlNS::style, "diagonal-bl-tr")) {
        result = true;
        borderString = styleStack.property(KoXmlNS::fo, "border-bl-tr");
        if ((hasSpecialBorder = styleStack.hasProperty(KoXmlNS::calligra, "specialborder-bl-tr"))) {
            specialBorderString = styleStack.property(KoXmlNS::calligra, "specialborder-bl-tr");
        }
        parseAndSetBorder(BltrBorder, borderString, hasSpecialBorder, specialBorderString);
    }

    // Handle double borders.
    if (styleStack.hasProperty(KoXmlNS::style, "border-line-width")) {
        result = true;
        QString borderLineWidth = styleStack.property(KoXmlNS::style, "border-line-width");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setInnerBorderWidth(LeftBorder, KoUnit::parseValue(blw[0], 0.1));
            setBorderSpacing(LeftBorder, KoUnit::parseValue(blw[1], 1.0));
            setBorderWidth(LeftBorder, KoUnit::parseValue(blw[2], 0.1));

            setInnerBorderWidth(TopBorder, KoUnit::parseValue(blw[0], 0.1));
            setBorderSpacing(TopBorder, KoUnit::parseValue(blw[1], 1.0));
            setBorderWidth(TopBorder, KoUnit::parseValue(blw[2], 0.1));

            setInnerBorderWidth(RightBorder, KoUnit::parseValue(blw[0], 0.1));
            setBorderSpacing(RightBorder, KoUnit::parseValue(blw[1], 1.0));
            setBorderWidth(RightBorder, KoUnit::parseValue(blw[2], 0.1));

            setInnerBorderWidth(BottomBorder, KoUnit::parseValue(blw[0], 0.1));
            setBorderSpacing(BottomBorder, KoUnit::parseValue(blw[1], 1.0));
            setBorderWidth(BottomBorder, KoUnit::parseValue(blw[2], 0.1));
        }
    }
    // Even if there are common border attributes, check for the
    // individual ones since they have precedence.

    if (styleStack.hasProperty(KoXmlNS::style, "border-line-width-left")) {
        result = true;
        QString borderLineWidth = styleStack.property(KoXmlNS::style, "border-line-width-left");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setInnerBorderWidth(LeftBorder, KoUnit::parseValue(blw[0], 0.1));
            setBorderSpacing(LeftBorder, KoUnit::parseValue(blw[1], 1.0));
            setBorderWidth(LeftBorder, KoUnit::parseValue(blw[2], 0.1));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::style, "border-line-width-top")) {
        result = true;
        QString borderLineWidth = styleStack.property(KoXmlNS::style, "border-line-width-top");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setInnerBorderWidth(TopBorder, KoUnit::parseValue(blw[0], 0.1));
            setBorderSpacing(TopBorder, KoUnit::parseValue(blw[1], 1.0));
            setBorderWidth(TopBorder, KoUnit::parseValue(blw[2], 0.1));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::style, "border-line-width-right")) {
        result = true;
        QString borderLineWidth = styleStack.property(KoXmlNS::style, "border-line-width-right");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setInnerBorderWidth(RightBorder, KoUnit::parseValue(blw[0], 0.1));
            setBorderSpacing(RightBorder, KoUnit::parseValue(blw[1], 1.0));
            setBorderWidth(RightBorder, KoUnit::parseValue(blw[2], 0.1));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::style, "border-line-width-bottom")) {
        result = true;
        QString borderLineWidth = styleStack.property(KoXmlNS::style, "border-line-width-bottom");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setInnerBorderWidth(BottomBorder, KoUnit::parseValue(blw[0], 0.1));
            setBorderSpacing(BottomBorder, KoUnit::parseValue(blw[1], 1.0));
            setBorderWidth(BottomBorder, KoUnit::parseValue(blw[2], 0.1));
        }
    }

    // Diagonals are treated individually and are NOT part of <style:border>.
    if (styleStack.hasProperty(KoXmlNS::style, "diagonal-tl-br-widths")) {
        result = true;
        QString borderLineWidth = styleStack.property(KoXmlNS::style, "diagonal-tl-br-widths");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setInnerBorderWidth(TlbrBorder, KoUnit::parseValue(blw[0], 0.1));
            setBorderSpacing(TlbrBorder, KoUnit::parseValue(blw[1], 1.0));
            setBorderWidth(TlbrBorder, KoUnit::parseValue(blw[2], 0.1));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::style, "diagonal-bl-tr-widths")) {
        result = true;
        QString borderLineWidth = styleStack.property(KoXmlNS::style, "diagonal-bl-tr-widths");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setInnerBorderWidth(BltrBorder, KoUnit::parseValue(blw[0], 0.1));
            setBorderSpacing(BltrBorder, KoUnit::parseValue(blw[1], 1.0));
            setBorderWidth(BltrBorder, KoUnit::parseValue(blw[2], 0.1));
        }
    }

    return result;
}


// Private
void KoBorder::parseAndSetBorder(const QString &borderString,
                                 bool hasSpecialBorder, const QString &specialBorderString)
{
    if (borderString == "none") {
        return;
    }

    //debugOdf << "*** *** Found border: " << border;
    QColor bordersColor;
    BorderStyle bordersStyle;
    qreal bordersWidth;
    bool foundStyle;
    bool foundWidth;
    parseOdfBorder(borderString, &bordersColor, &bordersStyle, &foundStyle,
                   &bordersWidth, &foundWidth);
    if (bordersColor.isValid()) {
        setBorderColor(LeftBorder, bordersColor);
        setBorderColor(TopBorder, bordersColor);
        setBorderColor(RightBorder, bordersColor);
        setBorderColor(BottomBorder, bordersColor);
    }
    if (hasSpecialBorder) {
        bordersStyle = KoBorder::odfBorderStyle(specialBorderString, &foundStyle);
    }

    if (foundStyle) {
        setBorderStyle(LeftBorder, bordersStyle);
        setBorderStyle(TopBorder, bordersStyle);
        setBorderStyle(RightBorder, bordersStyle);
        setBorderStyle(BottomBorder, bordersStyle);
    }
    if (foundWidth) {
        setBorderWidth(LeftBorder, bordersWidth);
        setBorderWidth(TopBorder, bordersWidth);
        setBorderWidth(RightBorder, bordersWidth);
        setBorderWidth(BottomBorder, bordersWidth);
    }
}

// Private
void KoBorder::parseAndSetBorder(const BorderSide borderSide, const QString &borderString,
                                 bool hasSpecialBorder, const QString &specialBorderString)
{
    QColor borderColor;
    BorderStyle borderStyle;
    qreal borderWidth;
    bool foundStyle;
    bool foundWidth;

    parseOdfBorder(borderString, &borderColor, &borderStyle, &foundStyle,
                   &borderWidth, &foundWidth);
    if (borderColor.isValid()) {
        setBorderColor(borderSide, borderColor);
    }
    if (hasSpecialBorder) {
        borderStyle = KoBorder::odfBorderStyle(specialBorderString, &foundStyle);
    }

    if (foundStyle) {
        setBorderStyle( borderSide, borderStyle);
    }
    if (foundWidth) {
        setBorderWidth( borderSide, borderWidth);
    }
}

void KoBorder::saveOdf(KoGenStyle &style, KoGenStyle::PropertyType type) const
{
    // Get the strings that describe respective borders.
    QString leftBorderString = QString("%1pt %2 %3")
                                 .arg(QString::number(borderWidth(LeftBorder)),
                                      odfBorderStyleString(borderStyle(LeftBorder)),
                                      borderColor(LeftBorder).name());
    QString rightBorderString =  QString("%1pt %2 %3")
                                  .arg(QString::number(borderWidth(RightBorder)),
                                       odfBorderStyleString(borderStyle(RightBorder)),
                                       borderColor(RightBorder).name());
    QString topBorderString = QString("%1pt %2 %3")
                                .arg(QString::number(borderWidth(TopBorder)),
                                     odfBorderStyleString(borderStyle(TopBorder)),
                                     borderColor(TopBorder).name());
    QString bottomBorderString = QString("%1pt %2 %3")
                                   .arg(QString::number(borderWidth(BottomBorder)),
                                        odfBorderStyleString(borderStyle(BottomBorder)),
                                        borderColor(BottomBorder).name());

    QString tlbrBorderString = QString("%1pt %2 %3")
                                .arg(QString::number(borderWidth(TlbrBorder)),
                                     odfBorderStyleString(borderStyle(TlbrBorder)),
                                     borderColor(TlbrBorder).name());
    QString trblBorderString = QString("%1pt %2 %3")
                                   .arg(QString::number(borderWidth(BltrBorder)),
                                        odfBorderStyleString(borderStyle(BltrBorder)),
                                        borderColor(BltrBorder).name());

    // Get the strings that describe respective special borders (for special mso support).
    QString leftBorderSpecialString = msoBorderStyleString(borderStyle(LeftBorder));
    QString rightBorderSpecialString = msoBorderStyleString(borderStyle(RightBorder));
    QString topBorderSpecialString = msoBorderStyleString(borderStyle(TopBorder));
    QString bottomBorderSpecialString = msoBorderStyleString(borderStyle(BottomBorder));
    QString tlbrBorderSpecialString = msoBorderStyleString(borderStyle(TlbrBorder));
    QString trblBorderSpecialString = msoBorderStyleString(borderStyle(BltrBorder));

    // Check if we can save all borders in one fo:border attribute, or
    // if we have to use several different ones like fo:border-left, etc.
    if (leftBorderString == rightBorderString
        && leftBorderString == topBorderString
        && leftBorderString == bottomBorderString) {

        // Yes, they were all the same, so use only fo:border
        style.addProperty("fo:border", leftBorderString, type);
        style.addProperty("calligra:specialborder-left", leftBorderSpecialString, type);
        style.addProperty("calligra:specialborder-right", rightBorderSpecialString, type);
        style.addProperty("calligra:specialborder-top", topBorderSpecialString, type);
        style.addProperty("calligra:specialborder-bottom", bottomBorderSpecialString, type);
    } else {
        // No, they were different, so use the individual borders.
        //if (leftBorderStyle() != BorderNone)
            style.addProperty("fo:border-left", leftBorderString, type);
            style.addProperty("calligra:specialborder-left", leftBorderSpecialString, type);
        //if (rightBorderStyle() != BorderNone)
            style.addProperty("fo:border-right", rightBorderString, type);
            style.addProperty("calligra:specialborder-right", rightBorderSpecialString, type);
        //if (topBorderStyle() != BorderNone)
            style.addProperty("fo:border-top", topBorderString, type);
            style.addProperty("calligra:specialborder-top", topBorderSpecialString, type);
        //if (bottomBorderStyle() != BorderNone)
            style.addProperty("fo:border-bottom", bottomBorderString, type);
            style.addProperty("calligra:specialborder-bottom", bottomBorderSpecialString, type);
    }

    if (style.type() != KoGenStyle::PageLayoutStyle) {
        //if (tlbrBorderStyle() != BorderNone) {
            style.addProperty("style:diagonal-tl-br", tlbrBorderString, type);
        //}
        //if (trblBorderStyle() != BorderNone) {
            style.addProperty("style:diagonal-bl-tr", trblBorderString, type);
        //}
    }

    // Handle double borders
    QString leftBorderLineWidth = QString("%1pt %2pt %3pt")
                                    .arg(QString::number(innerBorderWidth(LeftBorder)),
                                         QString::number(borderSpacing(LeftBorder)),
                                         QString::number(borderWidth(LeftBorder)));
    QString rightBorderLineWidth = QString("%1pt %2pt %3pt")
                                     .arg(QString::number(innerBorderWidth(RightBorder)),
                                          QString::number(borderSpacing(RightBorder)),
                                          QString::number(borderWidth(RightBorder)));
    QString topBorderLineWidth = QString("%1pt %2pt %3pt")
                                   .arg(QString::number(innerBorderWidth(TopBorder)),
                                        QString::number(borderSpacing(TopBorder)),
                                        QString::number(borderWidth(TopBorder)));
    QString bottomBorderLineWidth = QString("%1pt %2pt %3pt")
                                      .arg(QString::number(innerBorderWidth(BottomBorder)),
                                           QString::number(borderSpacing(BottomBorder)),
                                           QString::number(borderWidth(BottomBorder)));

    QString tlbrBorderLineWidth = QString("%1pt %2pt %3pt")
                                   .arg(QString::number(innerBorderWidth(TlbrBorder)),
                                        QString::number(borderSpacing(TlbrBorder)),
                                        QString::number(borderWidth(TlbrBorder)));
    QString trblBorderLineWidth = QString("%1pt %2pt %3pt")
                                      .arg(QString::number(innerBorderWidth(BltrBorder)),
                                           QString::number(borderSpacing(BltrBorder)),
                                           QString::number(borderWidth(BltrBorder)));

    if (leftBorderLineWidth == rightBorderLineWidth
        && leftBorderLineWidth == topBorderLineWidth
        && leftBorderLineWidth == bottomBorderLineWidth
        && borderStyle(LeftBorder) == borderStyle(RightBorder)
        && borderStyle(TopBorder) == borderStyle(BottomBorder)
        && borderStyle(TopBorder) == borderStyle(LeftBorder)
        && (borderStyle(LeftBorder) == BorderDouble || borderStyle(LeftBorder) == BorderDoubleWave)) {
        style.addProperty("style:border-line-width", leftBorderLineWidth, type);
    } else {
        if (borderStyle(LeftBorder) == BorderDouble || borderStyle(LeftBorder) == BorderDoubleWave)
            style.addProperty("style:border-line-width-left", leftBorderLineWidth, type);
        if (borderStyle(RightBorder) == BorderDouble || borderStyle(RightBorder) == BorderDoubleWave)
            style.addProperty("style:border-line-width-right", rightBorderLineWidth, type);
        if (borderStyle(TopBorder) == BorderDouble || borderStyle(TopBorder) == BorderDoubleWave)
            style.addProperty("style:border-line-width-top", topBorderLineWidth, type);
        if (borderStyle(BottomBorder) == BorderDouble || borderStyle(BottomBorder) == BorderDoubleWave)
            style.addProperty("style:border-line-width-bottom", bottomBorderLineWidth, type);
    }

    if (style.type() != KoGenStyle::PageLayoutStyle) {
        if (borderStyle(TlbrBorder) == BorderDouble || borderStyle(TlbrBorder) == BorderDoubleWave) {
            style.addProperty("style:diagonal-tl-br-widths", tlbrBorderLineWidth, type);
        }
        if (borderStyle(BltrBorder) == BorderDouble || borderStyle(BltrBorder) == BorderDoubleWave) {
            style.addProperty("style:diagonal-bl-tr-widths", trblBorderLineWidth, type);
        }
    }
}
