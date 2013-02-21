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

    QMap<KoBorder::BorderSide, KoBorder::BorderData> data;
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


void KoBorder::setBorderStyle(BorderSide side, BorderStyle style)
{
    if (!d->data.contains(side)) {
        BorderData data;
        data.style = style;
        d->data[side] = data;
    } else {
        d->data[side].style = style;
    }
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
    if (d->data.contains(LeftBorder) && borderWidth(LeftBorder) > 0.0)
        return true;
    if (d->data.contains(RightBorder) && borderWidth(RightBorder) > 0.0)
        return true;
    if (d->data.contains(TopBorder) && borderWidth(TopBorder) > 0.0)
        return true;
    if (d->data.contains(BottomBorder) && borderWidth(BottomBorder) > 0.0)
        return true;
    if (d->data.contains(TlbrBorder) && borderWidth(TlbrBorder) > 0.0)
        return true;
    if (d->data.contains(BltrBorder) && borderWidth(BltrBorder) > 0.0)
        return true;
    return false;
}

bool KoBorder::hasBorder(KoBorder::BorderSide side) const
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
            setBorderColor(LeftBorder, allBordersColor);
            setBorderColor(TopBorder, allBordersColor);
            setBorderColor(RightBorder, allBordersColor);
            setBorderColor(BottomBorder, allBordersColor);
        }
        if (style.hasAttributeNS(KoXmlNS::calligra, "specialborder")) {
            allBordersStyle = KoBorder::odfBorderStyle(style.attributeNS(KoXmlNS::calligra, "specialborder"), &foundStyle);
        }
        if (foundStyle) {
            setBorderStyle(LeftBorder, allBordersStyle);
            setBorderStyle(TopBorder, allBordersStyle);
            setBorderStyle(RightBorder, allBordersStyle);
            setBorderStyle(BottomBorder, allBordersStyle);
        }
        if (foundWidth) {
            setBorderWidth(LeftBorder, allBordersWidth);
            setBorderWidth(TopBorder, allBordersWidth);
            setBorderWidth(RightBorder, allBordersWidth);
            setBorderWidth(BottomBorder, allBordersWidth);
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
                setBorderColor(LeftBorder, borderColor);
            }
            if (style.hasAttributeNS(KoXmlNS::calligra, "specialborder-left")) {
                borderStyle = KoBorder::odfBorderStyle(style.attributeNS(KoXmlNS::calligra, "specialborder-left"), &foundStyle);
            }
            if (foundStyle) {
                setBorderStyle(LeftBorder, borderStyle);
            }
            if (foundWidth) {
                setBorderWidth(LeftBorder, borderWidth);
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
                setBorderColor(TopBorder, borderColor);
            }
            if (style.hasAttributeNS(KoXmlNS::calligra, "specialborder-top")) {
                borderStyle = KoBorder::odfBorderStyle(style.attributeNS(KoXmlNS::calligra, "specialborder-top"), &foundStyle);
            }
            if (foundStyle) {
                setBorderStyle(TopBorder, borderStyle);
            }
            if (foundWidth) {
                setBorderWidth(TopBorder, borderWidth);
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
                setBorderColor(RightBorder, borderColor);
            }
            if (style.hasAttributeNS(KoXmlNS::calligra, "specialborder-right")) {
                borderStyle = KoBorder::odfBorderStyle(style.attributeNS(KoXmlNS::calligra, "specialborder-right"), &foundStyle);
            }
            if (foundStyle) {
                setBorderStyle(RightBorder, borderStyle);
            }
            if (foundWidth) {
                setBorderWidth(RightBorder, borderWidth);
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
                setBorderColor(BottomBorder, borderColor);
            }
            if (style.hasAttributeNS(KoXmlNS::calligra, "specialborder-bottom")) {
                borderStyle = KoBorder::odfBorderStyle(style.attributeNS(KoXmlNS::calligra, "specialborder-bottom"), &foundStyle);
            }
            if (foundStyle) {
                setBorderStyle(BottomBorder, borderStyle);
            }
            if (foundWidth) {
                setBorderWidth(BottomBorder, borderWidth);
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
            setBorderColor(TlbrBorder, borderColor);
        }
        if (style.hasAttributeNS(KoXmlNS::calligra, "specialborder-tl-br")) {
            borderStyle = KoBorder::odfBorderStyle(style.attributeNS(KoXmlNS::calligra, "specialborder-tl-br"), &foundStyle);
        }
        if (foundStyle) {
            setBorderStyle(TlbrBorder, borderStyle);
        }
        if (foundWidth) {
            setBorderWidth(TlbrBorder, borderWidth);
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
            setBorderColor(BltrBorder, borderColor);
        }
        if (style.hasAttributeNS(KoXmlNS::calligra, "specialborder-bl-tr")) {
            borderStyle = KoBorder::odfBorderStyle(style.attributeNS(KoXmlNS::calligra, "specialborder-bl-tr"), &foundStyle);
        }
        if (foundStyle) {
            setBorderStyle(BltrBorder, borderStyle);
        }
        if (foundWidth) {
            setBorderWidth(BltrBorder, borderWidth);
        }
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
