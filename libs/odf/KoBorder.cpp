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

#include <KDebug>

#include <KoUnit.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoGenStyle.h>


class KoBorderPrivate : public QSharedData
{
public:
    KoBorderPrivate();
    ~KoBorderPrivate();

    QMap<KoBorder::Side, KoBorder::BorderData> data;
};

KoBorderPrivate::KoBorderPrivate()
{
}

KoBorderPrivate::~KoBorderPrivate()
{
}

KoBorder::BorderData::BorderData()
    : style(KoBorder::BorderNone),
    spacing(0),
    innerPen(QPen()),
    outerPen(QPen())
{
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

    KoBorder::Side key;
    
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
    if (borderstyle == "slash") // not officially odf, but we suppport it anyway
        return KoBorder::BorderSlash;
    if (borderstyle == "wave") // not officially odf, but we suppport it anyway
        return KoBorder::BorderWave;
    if (borderstyle == "double-wave") // not officially odf, but we suppport it anyway
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
        return QString("slash"); // not officially odf, but we suppport it anyway
    case KoBorder::BorderWave:
        return QString("wave"); // not officially odf, but we suppport it anyway
    case KoBorder::BorderDoubleWave:
        return QString("double-wave"); // not officially odf, but we suppport it anyway

    default:
        // Handle remaining styles as odf type style.
        return odfBorderStyleString(borderstyle);
    }
}


// ----------------------------------------------------------------
//                         Getters and Setters


void KoBorder::setBorderStyle(Side side, BorderStyle style)
{
    if (!d->data.contains(side)) {
        BorderData data;
        data.style = style;
        d->data[side] = data;
    } else {
        d->data[side].style = style;
    }
}

KoBorder::BorderStyle KoBorder::borderStyle(Side side) const
{
    if (!d->data.contains(side)) {
        return BorderNone;
    } else {
        return d->data[side].style;
    }
}

void KoBorder::setBorderColor(Side side, const QColor &color)
{
    if (!d->data.contains(side)) {
        BorderData data;
        data.outerPen.setColor(color);
        d->data[side] = data;
    } else {
        d->data[side].outerPen.setColor(color);
    }
}

QColor KoBorder::borderColor(Side side) const
{
    if (!d->data.contains(side)) {
        return QColor();
    } else {
        return d->data[side].outerPen.color();
    }
}

void KoBorder::setBorderWidth(Side side, qreal width)
{
    if (!d->data.contains(side)) {
        BorderData data;
        data.outerPen.setWidthF(width);
        d->data[side] = data;
    } else {
        d->data[side].outerPen.setWidthF(width);
    }
}

qreal KoBorder::borderWidth(Side side) const
{
    if (!d->data.contains(side)) {
        return 0;
    } else {
        return d->data[side].outerPen.widthF();
    }
}

void KoBorder::setInnerBorderWidth(Side side, qreal width)
{
    if (!d->data.contains(side)) {
        BorderData data;
        data.innerPen.setWidthF(width);
        d->data[side] = data;
    } else {
        d->data[side].innerPen.setWidthF(width);
    }
}

qreal KoBorder::innerBorderWidth(Side side) const
{
    if (!d->data.contains(side)) {
        return 0;
    } else {
        return d->data[side].innerPen.widthF();
    }
}

void KoBorder::setBorderSpacing(Side side, qreal width)
{
    if (!d->data.contains(side)) {
        BorderData data;
        data.spacing = width;
        d->data[side] = data;
    } else {
        d->data[side].spacing = width;
    }
}

qreal KoBorder::borderSpacing(Side side) const
{
    if (!d->data.contains(side)) {
        return 0;
    } else {
        return d->data[side].spacing;
    }
}


KoBorder::BorderData KoBorder::borderData(Side side) const
{
    return d->data.value(side, BorderData());
}

void KoBorder::setBorderData(Side side, const BorderData &data)
{
    d->data[side] = data;
}


// -------------------------------

bool KoBorder::hasBorder() const
{
    if (d->data.contains(Left) && borderWidth(Left) > 0.0)
        return true;
    if (d->data.contains(Right) && borderWidth(Right) > 0.0)
        return true;
    if (d->data.contains(Top) && borderWidth(Top) > 0.0)
        return true;
    if (d->data.contains(Bottom) && borderWidth(Bottom) > 0.0)
        return true;
    if (d->data.contains(BottomLeftToTopRight) && borderWidth(BottomLeftToTopRight) > 0.0)
        return true;
    if (d->data.contains(TopLeftToBottomRight) && borderWidth(TopLeftToBottomRight) > 0.0)
        return true;
    return false;
}

bool KoBorder::hasBorder(KoBorder::Side side) const
{
    return d->data.contains(side);
}

void parseOdfBorder (const QString &border, QColor *color, KoBorder::BorderStyle *borderStyle, bool *hasBorderStyle, qreal *borderWidth, bool *hasBorderWidth)
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
    if (style.hasAttributeNS(KoXmlNS::fo, "border")) {
        result = true;
        QString border = style.attributeNS(KoXmlNS::fo, "border");

        //kDebug() << "*** *** Found border: " << border;
        QColor allBordersColor;
        KoBorder::BorderStyle allBordersStyle;
        qreal allBordersWidth;
        bool foundStyle, foundWidth;
        parseOdfBorder(border, &allBordersColor, &allBordersStyle, &foundStyle, &allBordersWidth, &foundWidth);
        if (allBordersColor.isValid()) {
            setBorderColor(Left, allBordersColor);
            setBorderColor(Top, allBordersColor);
            setBorderColor(Right, allBordersColor);
            setBorderColor(Bottom, allBordersColor);
        }
        if (style.hasAttributeNS(KoXmlNS::calligra, "specialborder")) {
            allBordersStyle = KoBorder::odfBorderStyle(style.attributeNS(KoXmlNS::calligra, "specialborder"), &foundStyle);
        }
        if (foundStyle) {
            setBorderStyle(Left, allBordersStyle);
            setBorderStyle(Top, allBordersStyle);
            setBorderStyle(Right, allBordersStyle);
            setBorderStyle(Bottom, allBordersStyle);
        }
        if (foundWidth) {
            setBorderWidth(Left, allBordersWidth);
            setBorderWidth(Top, allBordersWidth);
            setBorderWidth(Right, allBordersWidth);
            setBorderWidth(Bottom, allBordersWidth);
        }
    }
    else {
        // No common border attributes, check for the individual ones.
        if (style.hasAttributeNS(KoXmlNS::fo, "border-left")) {
            result = true;
            QString border = style.attributeNS(KoXmlNS::fo, "border-left");
            QColor borderColor;
            KoBorder::BorderStyle borderStyle;
            qreal borderWidth;
            bool foundStyle, foundWidth;
            parseOdfBorder(border, &borderColor, &borderStyle, &foundStyle, &borderWidth, &foundWidth);
            if (borderColor.isValid()) {
                setBorderColor(Left, borderColor);
            }
            if (style.hasAttributeNS(KoXmlNS::calligra, "specialborder-left")) {
                borderStyle = KoBorder::odfBorderStyle(style.attributeNS(KoXmlNS::calligra, "specialborder-left"), &foundStyle);
            }
            if (foundStyle) {
                setBorderStyle(Left, borderStyle);
            }
            if (foundWidth) {
                setBorderWidth(Left, borderWidth);
            }
        }
        if (style.hasAttributeNS(KoXmlNS::fo, "border-top")) {
            result = true;
            QString border = style.attributeNS(KoXmlNS::fo, "border-top");
            QColor borderColor;
            KoBorder::BorderStyle borderStyle;
            qreal borderWidth;
            bool foundStyle, foundWidth;
            parseOdfBorder(border, &borderColor, &borderStyle, &foundStyle, &borderWidth, &foundWidth);
            if (borderColor.isValid()) {
                setBorderColor(Top, borderColor);
            }
            if (style.hasAttributeNS(KoXmlNS::calligra, "specialborder-top")) {
                borderStyle = KoBorder::odfBorderStyle(style.attributeNS(KoXmlNS::calligra, "specialborder-top"), &foundStyle);
            }
            if (foundStyle) {
                setBorderStyle(Top, borderStyle);
            }
            if (foundWidth) {
                setBorderWidth(Top, borderWidth);
            }
        }
        if (style.hasAttributeNS(KoXmlNS::fo, "border-right")) {
            result = true;
            QString border = style.attributeNS(KoXmlNS::fo, "border-right");
            QColor borderColor;
            KoBorder::BorderStyle borderStyle;
            qreal borderWidth;
            bool foundStyle, foundWidth;
            parseOdfBorder(border, &borderColor, &borderStyle, &foundStyle, &borderWidth, &foundWidth);
            if (borderColor.isValid()) {
                setBorderColor(Right, borderColor);
            }
            if (style.hasAttributeNS(KoXmlNS::calligra, "specialborder-right")) {
                borderStyle = KoBorder::odfBorderStyle(style.attributeNS(KoXmlNS::calligra, "specialborder-right"), &foundStyle);
            }
            if (foundStyle) {
                setBorderStyle(Right, borderStyle);
            }
            if (foundWidth) {
                setBorderWidth(Right, borderWidth);
            }
        }
        if (style.hasAttributeNS(KoXmlNS::fo, "border-bottom")) {
            result = true;
            QString border = style.attributeNS(KoXmlNS::fo, "border-bottom");
            QColor borderColor;
            KoBorder::BorderStyle borderStyle;
            qreal borderWidth;
            bool foundStyle, foundWidth;
            parseOdfBorder(border, &borderColor, &borderStyle, &foundStyle, &borderWidth, &foundWidth);
            if (borderColor.isValid()) {
                setBorderColor(Bottom, borderColor);
            }
            if (style.hasAttributeNS(KoXmlNS::calligra, "specialborder-bottom")) {
                borderStyle = KoBorder::odfBorderStyle(style.attributeNS(KoXmlNS::calligra, "specialborder-bottom"), &foundStyle);
            }
            if (foundStyle) {
                setBorderStyle(Bottom, borderStyle);
            }
            if (foundWidth) {
                setBorderWidth(Bottom, borderWidth);
            }
        }
    }

    if (style.hasAttributeNS(KoXmlNS::style, "diagonal-tl-br")) {
        result = true;
        QString border = style.attributeNS(KoXmlNS::style, "diagonal-tl-br");
        QColor borderColor;
        KoBorder::BorderStyle borderStyle;
        qreal borderWidth;
        bool foundStyle, foundWidth;
        parseOdfBorder(border, &borderColor, &borderStyle, &foundStyle, &borderWidth, &foundWidth);
        if (borderColor.isValid()) {
            setBorderColor(TopLeftToBottomRight, borderColor);
        }
        if (style.hasAttributeNS(KoXmlNS::calligra, "specialborder-tl-br")) {
            borderStyle = KoBorder::odfBorderStyle(style.attributeNS(KoXmlNS::calligra, "specialborder-tl-br"), &foundStyle);
        }
        if (foundStyle) {
            setBorderStyle(TopLeftToBottomRight, borderStyle);
        }
        if (foundWidth) {
            setBorderWidth(TopLeftToBottomRight, borderWidth);
        }
    }
    if (style.hasAttributeNS(KoXmlNS::style, "diagonal-bl-tr")) {
        result = true;
        QString border = style.attributeNS(KoXmlNS::style, "diagonal-bl-tr");
        QColor borderColor;
        KoBorder::BorderStyle borderStyle;
        qreal borderWidth;
        bool foundStyle, foundWidth;
        parseOdfBorder(border, &borderColor, &borderStyle, &foundStyle, &borderWidth, &foundWidth);
        if (borderColor.isValid()) {
            setBorderColor(BottomLeftToTopRight, borderColor);
        }
        if (style.hasAttributeNS(KoXmlNS::calligra, "specialborder-bl-tr")) {
            borderStyle = KoBorder::odfBorderStyle(style.attributeNS(KoXmlNS::calligra, "specialborder-bl-tr"), &foundStyle);
        }
        if (foundStyle) {
            setBorderStyle(BottomLeftToTopRight, borderStyle);
        }
        if (foundWidth) {
            setBorderWidth(BottomLeftToTopRight, borderWidth);
        }
    }

    // Handle double borders.
    if (style.hasAttributeNS(KoXmlNS::style, "border-line-width")) {
        result = true;
        QString borderLineWidth = style.attributeNS(KoXmlNS::style, "border-line-width");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setInnerBorderWidth(Left, KoUnit::parseValue(blw[0], 0.1));
            setBorderSpacing(Left, KoUnit::parseValue(blw[1], 1.0));
            setBorderWidth(Left, KoUnit::parseValue(blw[2], 0.1));

            setInnerBorderWidth(Top, KoUnit::parseValue(blw[0], 0.1));
            setBorderSpacing(Top, KoUnit::parseValue(blw[1], 1.0));
            setBorderWidth(Top, KoUnit::parseValue(blw[2], 0.1));

            setInnerBorderWidth(Right, KoUnit::parseValue(blw[0], 0.1));
            setBorderSpacing(Right, KoUnit::parseValue(blw[1], 1.0));
            setBorderWidth(Right, KoUnit::parseValue(blw[2], 0.1));

            setInnerBorderWidth(Bottom, KoUnit::parseValue(blw[0], 0.1));
            setBorderSpacing(Bottom, KoUnit::parseValue(blw[1], 1.0));
            setBorderWidth(Bottom, KoUnit::parseValue(blw[2], 0.1));
        }
    }
    else {
        if (style.hasAttributeNS(KoXmlNS::style, "border-line-width-left")) {
            result = true;
            QString borderLineWidth = style.attributeNS(KoXmlNS::style, "border-line-width-left");
            if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
                QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
                setInnerBorderWidth(Left, KoUnit::parseValue(blw[0], 0.1));
                setBorderSpacing(Left, KoUnit::parseValue(blw[1], 1.0));
                setBorderWidth(Left, KoUnit::parseValue(blw[2], 0.1));
            }
        }
        if (style.hasAttributeNS(KoXmlNS::style, "border-line-width-top")) {
            result = true;
            QString borderLineWidth = style.attributeNS(KoXmlNS::style, "border-line-width-top");
            if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
                QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
                setInnerBorderWidth(Top, KoUnit::parseValue(blw[0], 0.1));
                setBorderSpacing(Top, KoUnit::parseValue(blw[1], 1.0));
                setBorderWidth(Top, KoUnit::parseValue(blw[2], 0.1));
            }
        }
        if (style.hasAttributeNS(KoXmlNS::style, "border-line-width-right")) {
            result = true;
            QString borderLineWidth = style.attributeNS(KoXmlNS::style, "border-line-width-right");
            if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
                QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
                setInnerBorderWidth(Right, KoUnit::parseValue(blw[0], 0.1));
                setBorderSpacing(Right, KoUnit::parseValue(blw[1], 1.0));
                setBorderWidth(Right, KoUnit::parseValue(blw[2], 0.1));
            }
        }
        if (style.hasAttributeNS(KoXmlNS::style, "border-line-width-bottom")) {
            result = true;
            QString borderLineWidth = style.attributeNS(KoXmlNS::style, "border-line-width-bottom");
            if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
                QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
                setInnerBorderWidth(Bottom, KoUnit::parseValue(blw[0], 0.1));
                setBorderSpacing(Bottom, KoUnit::parseValue(blw[1], 1.0));
                setBorderWidth(Bottom, KoUnit::parseValue(blw[2], 0.1));
            }
        }
    }

    if (style.hasAttributeNS(KoXmlNS::style, "diagonal-tl-br-widths")) {
        result = true;
        QString borderLineWidth = style.attributeNS(KoXmlNS::style, "diagonal-tl-br-widths");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setInnerBorderWidth(TopLeftToBottomRight, KoUnit::parseValue(blw[0], 0.1));
            setBorderSpacing(TopLeftToBottomRight, KoUnit::parseValue(blw[1], 1.0));
            setBorderWidth(TopLeftToBottomRight, KoUnit::parseValue(blw[2], 0.1));
        }
    }
    if (style.hasAttributeNS(KoXmlNS::style, "diagonal-bl-tr-widths")) {
        result = true;
        QString borderLineWidth = style.attributeNS(KoXmlNS::style, "diagonal-bl-tr-widths");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setInnerBorderWidth(BottomLeftToTopRight, KoUnit::parseValue(blw[0], 0.1));
            setBorderSpacing(BottomLeftToTopRight, KoUnit::parseValue(blw[1], 1.0));
            setBorderWidth(BottomLeftToTopRight, KoUnit::parseValue(blw[2], 0.1));
        }
    }
    return result;
}

void KoBorder::saveOdf(KoGenStyle &style, KoGenStyle::PropertyType type) const
{
    // Get the strings that describe respective borders.
    QString leftBorderString = QString("%1pt %2 %3")
                                 .arg(QString::number(borderWidth(Left)),
                                      odfBorderStyleString(borderStyle(Left)),
                                      borderColor(Left).name());
    QString rightBorderString =  QString("%1pt %2 %3")
                                  .arg(QString::number(borderWidth(Right)),
                                       odfBorderStyleString(borderStyle(Right)),
                                       borderColor(Right).name());
    QString topBorderString = QString("%1pt %2 %3")
                                .arg(QString::number(borderWidth(Top)),
                                     odfBorderStyleString(borderStyle(Top)),
                                     borderColor(Top).name());
    QString bottomBorderString = QString("%1pt %2 %3")
                                   .arg(QString::number(borderWidth(Bottom)),
                                        odfBorderStyleString(borderStyle(Bottom)),
                                        borderColor(Bottom).name());

    QString tlbrBorderString = QString("%1pt %2 %3")
                                .arg(QString::number(borderWidth(TopLeftToBottomRight)),
                                     odfBorderStyleString(borderStyle(TopLeftToBottomRight)),
                                     borderColor(TopLeftToBottomRight).name());
    QString trblBorderString = QString("%1pt %2 %3")
                                   .arg(QString::number(borderWidth(BottomLeftToTopRight)),
                                        odfBorderStyleString(borderStyle(BottomLeftToTopRight)),
                                        borderColor(BottomLeftToTopRight).name());

    // Get the strings that describe respective special borders (for special mso support).
    QString leftBorderSpecialString = msoBorderStyleString(borderStyle(Left));
    QString rightBorderSpecialString = msoBorderStyleString(borderStyle(Right));
    QString topBorderSpecialString = msoBorderStyleString(borderStyle(Top));
    QString bottomBorderSpecialString = msoBorderStyleString(borderStyle(Bottom));
    QString tlbrBorderSpecialString = msoBorderStyleString(borderStyle(TopLeftToBottomRight));
    QString trblBorderSpecialString = msoBorderStyleString(borderStyle(BottomLeftToTopRight));

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
                                    .arg(QString::number(innerBorderWidth(Left)),
                                         QString::number(borderSpacing(Left)),
                                         QString::number(borderWidth(Left)));
    QString rightBorderLineWidth = QString("%1pt %2pt %3pt")
                                     .arg(QString::number(innerBorderWidth(Right)),
                                          QString::number(borderSpacing(Right)),
                                          QString::number(borderWidth(Right)));
    QString topBorderLineWidth = QString("%1pt %2pt %3pt")
                                   .arg(QString::number(innerBorderWidth(Top)),
                                        QString::number(borderSpacing(Top)),
                                        QString::number(borderWidth(Top)));
    QString bottomBorderLineWidth = QString("%1pt %2pt %3pt")
                                      .arg(QString::number(innerBorderWidth(Bottom)),
                                           QString::number(borderSpacing(Bottom)),
                                           QString::number(borderWidth(Bottom)));

    QString tlbrBorderLineWidth = QString("%1pt %2pt %3pt")
                                   .arg(QString::number(innerBorderWidth(TopLeftToBottomRight)),
                                        QString::number(borderSpacing(TopLeftToBottomRight)),
                                        QString::number(borderWidth(TopLeftToBottomRight)));
    QString trblBorderLineWidth = QString("%1pt %2pt %3pt")
                                      .arg(QString::number(innerBorderWidth(BottomLeftToTopRight)),
                                           QString::number(borderSpacing(BottomLeftToTopRight)),
                                           QString::number(borderWidth(BottomLeftToTopRight)));

    if (leftBorderLineWidth == rightBorderLineWidth
        && leftBorderLineWidth == topBorderLineWidth
        && leftBorderLineWidth == bottomBorderLineWidth
        && borderStyle(Left) == borderStyle(Right)
        && borderStyle(Top) == borderStyle(Bottom)
        && borderStyle(Top) == borderStyle(Left)
        && (borderStyle(Left) == BorderDouble || borderStyle(Left) == BorderDoubleWave)) {
        style.addProperty("style:border-line-width", leftBorderLineWidth, type);
    } else {
        if (borderStyle(Left) == BorderDouble || borderStyle(Left) == BorderDoubleWave)
            style.addProperty("style:border-line-width-left", leftBorderLineWidth, type);
        if (borderStyle(Right) == BorderDouble || borderStyle(Right) == BorderDoubleWave)
            style.addProperty("style:border-line-width-right", rightBorderLineWidth, type);
        if (borderStyle(Top) == BorderDouble || borderStyle(Top) == BorderDoubleWave)
            style.addProperty("style:border-line-width-top", topBorderLineWidth, type);
        if (borderStyle(Bottom) == BorderDouble || borderStyle(Bottom) == BorderDoubleWave)
            style.addProperty("style:border-line-width-bottom", bottomBorderLineWidth, type);
    }

    if (style.type() != KoGenStyle::PageLayoutStyle) {
        if (borderStyle(TopLeftToBottomRight) == BorderDouble || borderStyle(TopLeftToBottomRight) == BorderDoubleWave) {
            style.addProperty("style:diagonal-tl-br-widths", tlbrBorderLineWidth, type);
        }
        if (borderStyle(BottomLeftToTopRight) == BorderDouble || borderStyle(BottomLeftToTopRight) == BorderDoubleWave) {
            style.addProperty("style:diagonal-bl-tr-widths", trblBorderLineWidth, type);
        }
    }
}
