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

    KoBorder::BorderData leftBorder;
    KoBorder::BorderData topBorder;
    KoBorder::BorderData rightBorder;
    KoBorder::BorderData bottomBorder;
    KoBorder::BorderData tlbrBorder;
    KoBorder::BorderData trblBorder;
};

KoBorderPrivate::KoBorderPrivate()
{
}

KoBorderPrivate::~KoBorderPrivate()
{
}

KoBorder::BorderData::BorderData()
    : style(KoBorder::BorderNone),
    width(0),
    innerWidth(0),
    spacing(0)
{
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

    // Left Borders
    if (d->leftBorder.style == BorderNone && other.d->leftBorder.style == BorderNone) {
        // If both styles are None, then the rest of the values don't
        // need to be compared.
        ;
    }
    else if (d->leftBorder.style != other.d->leftBorder.style) {
        // If any of them are non-None, and they are different, the
        // borders are also different.
        return false;
    }
    else {
        // Here we know that the border styles are the same, now
        // compare the rest of the values.
        if (d->leftBorder.width != other.d->leftBorder.width)
            return false;
        if (d->leftBorder.color != other.d->leftBorder.color)
            return false;

        // If the border style == BorderDouble, then compare a couple
        // of other values too.
        if (d->leftBorder.style == BorderDouble) {
            if (d->leftBorder.innerWidth != other.d->leftBorder.innerWidth)
                return false;
            if (d->leftBorder.spacing != other.d->leftBorder.spacing)
                return false;
        }
    }

    // Tope Borders
    if (d->topBorder.style == BorderNone && other.d->topBorder.style == BorderNone) {
        // If both styles are None, then the rest of the values don't
        // need to be compared.
        ;
    }
    else if (d->topBorder.style != other.d->topBorder.style) {
        // If any of them are non-None, and they are different, the
        // borders are also different.
        return false;
    }
    else {
        // Here we know that the border styles are the same, now
        // compare the rest of the values.
        if (d->topBorder.width != other.d->topBorder.width)
            return false;
        if (d->topBorder.color != other.d->topBorder.color)
            return false;

        // If the border style == BorderDouble, then compare a couple
        // of other values too.
        if (d->topBorder.style == BorderDouble) {
            if (d->topBorder.innerWidth != other.d->topBorder.innerWidth)
                return false;
            if (d->topBorder.spacing != other.d->topBorder.spacing)
                return false;
        }
    }

    // Right Borders
    if (d->rightBorder.style == BorderNone && other.d->rightBorder.style == BorderNone) {
        // If both styles are None, then the rest of the values don't
        // need to be compared.
        ;
    }
    else if (d->rightBorder.style != other.d->rightBorder.style) {
        // If any of them are non-None, and they are different, the
        // borders are also different.
        return false;
    }
    else {
        // Here we know that the border styles are the same, now
        // compare the rest of the values.
        if (d->rightBorder.width != other.d->rightBorder.width)
            return false;
        if (d->rightBorder.color != other.d->rightBorder.color)
            return false;

        // If the border style == BorderDouble, then compare a couple
        // of other values too.
        if (d->rightBorder.style == BorderDouble) {
            if (d->rightBorder.innerWidth != other.d->rightBorder.innerWidth)
                return false;
            if (d->rightBorder.spacing != other.d->rightBorder.spacing)
                return false;
        }
    }

    // Bottom Borders
    if (d->bottomBorder.style == BorderNone && other.d->bottomBorder.style == BorderNone) {
        // If both styles are None, then the rest of the values don't
        // need to be compared.
        ;
    }
    else if (d->bottomBorder.style != other.d->bottomBorder.style) {
        // If any of them are non-None, and they are different, the
        // borders are also different.
        return false;
    }
    else {
        // Here we know that the border styles are the same, now
        // compare the rest of the values.
        if (d->bottomBorder.width != other.d->bottomBorder.width)
            return false;
        if (d->bottomBorder.color != other.d->bottomBorder.color)
            return false;

        // If the border style == BorderDouble, then compare a couple
        // of other values too.
        if (d->bottomBorder.style == BorderDouble) {
            if (d->bottomBorder.innerWidth != other.d->bottomBorder.innerWidth)
                return false;
            if (d->bottomBorder.spacing != other.d->bottomBorder.spacing)
                return false;
        }
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


// ----------------------------------------------------------------


void KoBorder::setLeftBorderStyle(BorderStyle style)
{
    d->leftBorder.style = style;
}

KoBorder::BorderStyle KoBorder::leftBorderStyle() const
{
    return d->leftBorder.style;
}

void KoBorder::setLeftBorderColor(const QColor &color)
{
    d->leftBorder.color = color;
}

QColor KoBorder::leftBorderColor() const
{
    return d->leftBorder.color;
}

void KoBorder::setLeftBorderWidth(qreal width)
{
    d->leftBorder.width = width;
}

qreal KoBorder::leftBorderWidth() const
{
    return d->leftBorder.width;
}

void KoBorder::setLeftInnerBorderWidth(qreal width)
{
    d->leftBorder.innerWidth = width;
}

qreal KoBorder::leftInnerBorderWidth() const
{
    return d->leftBorder.innerWidth;
}

void KoBorder::setLeftBorderSpacing(qreal width)
{
    d->leftBorder.spacing = width;
}

qreal KoBorder::leftBorderSpacing() const
{
    return d->leftBorder.spacing;
}


void KoBorder::setTopBorderStyle(BorderStyle style)
{
    d->topBorder.style = style;
}

KoBorder::BorderStyle KoBorder::topBorderStyle() const
{
    return d->topBorder.style;
}

void KoBorder::setTopBorderColor(const QColor &color)
{
    d->topBorder.color = color;
}

QColor KoBorder::topBorderColor() const
{
    return d->topBorder.color;
}

void KoBorder::setTopBorderWidth(qreal width)
{
    d->topBorder.width = width;
}

qreal KoBorder::topBorderWidth() const
{
    return d->topBorder.width;
}

void KoBorder::setTopInnerBorderWidth(qreal width)
{
    d->topBorder.innerWidth = width;
}

qreal KoBorder::topInnerBorderWidth() const
{
    return d->topBorder.innerWidth;
}

void KoBorder::setTopBorderSpacing(qreal width)
{
    d->topBorder.spacing = width;
}

qreal KoBorder::topBorderSpacing() const
{
    return d->topBorder.spacing;
}


void KoBorder::setRightBorderStyle(BorderStyle style)
{
    d->rightBorder.style = style;
}

KoBorder::BorderStyle KoBorder::rightBorderStyle() const
{
    return d->rightBorder.style;
}

void KoBorder::setRightBorderColor(const QColor &color)
{
    d->rightBorder.color = color;
}

QColor KoBorder::rightBorderColor() const
{
    return d->rightBorder.color;
}

void KoBorder::setRightBorderWidth(qreal width)
{
    d->rightBorder.width = width;
}

qreal KoBorder::rightBorderWidth() const
{
    return d->rightBorder.width;
}

void KoBorder::setRightInnerBorderWidth(qreal width)
{
    d->rightBorder.innerWidth = width;
}

qreal KoBorder::rightInnerBorderWidth() const
{
    return d->rightBorder.innerWidth;
}

void KoBorder::setRightBorderSpacing(qreal width)
{
    d->rightBorder.spacing = width;
}

qreal KoBorder::rightBorderSpacing() const
{
    return d->rightBorder.spacing;
}


void KoBorder::setBottomBorderStyle(BorderStyle style)
{
    d->bottomBorder.style = style;
}

KoBorder::BorderStyle KoBorder::bottomBorderStyle() const
{
    return d->bottomBorder.style;
}

void KoBorder::setBottomBorderColor(const QColor &color)
{
    d->bottomBorder.color = color;
}

QColor KoBorder::bottomBorderColor() const
{
    return d->bottomBorder.color;
}

void KoBorder::setBottomBorderWidth(qreal width)
{
    d->bottomBorder.width = width;
}

qreal KoBorder::bottomBorderWidth() const
{
    return d->bottomBorder.width;
}

void KoBorder::setBottomInnerBorderWidth(qreal width)
{
    d->bottomBorder.innerWidth = width;
}

qreal KoBorder::bottomInnerBorderWidth() const
{
    return d->bottomBorder.innerWidth;
}

void KoBorder::setBottomBorderSpacing(qreal width)
{
    d->bottomBorder.spacing = width;
}

qreal KoBorder::bottomBorderSpacing() const
{
    return d->bottomBorder.spacing;
}

void KoBorder::setTlbrBorderStyle(BorderStyle style)
{
    d->tlbrBorder.style = style;
}

KoBorder::BorderStyle KoBorder::tlbrBorderStyle() const
{
    return d->tlbrBorder.style;
}

void KoBorder::setTlbrBorderColor(const QColor &color)
{
    d->tlbrBorder.color = color;
}

QColor KoBorder::tlbrBorderColor() const
{
    return d->tlbrBorder.color;
}

void KoBorder::setTlbrBorderWidth(qreal width)
{
    d->tlbrBorder.width = width;
}

qreal KoBorder::tlbrBorderWidth() const
{
    return d->tlbrBorder.width;
}

void KoBorder::setTlbrInnerBorderWidth(qreal width)
{
    d->tlbrBorder.innerWidth = width;
}

qreal KoBorder::tlbrInnerBorderWidth() const
{
    return d->tlbrBorder.innerWidth;
}

void KoBorder::setTlbrBorderSpacing(qreal width)
{
    d->tlbrBorder.spacing = width;
}

qreal KoBorder::tlbrBorderSpacing() const
{
    return d->tlbrBorder.spacing;
}

void KoBorder::setTrblBorderStyle(BorderStyle style)
{
    d->trblBorder.style = style;
}

KoBorder::BorderStyle KoBorder::trblBorderStyle() const
{
    return d->trblBorder.style;
}

void KoBorder::setTrblBorderColor(const QColor &color)
{
    d->trblBorder.color = color;
}

QColor KoBorder::trblBorderColor() const
{
    return d->trblBorder.color;
}

void KoBorder::setTrblBorderWidth(qreal width)
{
    d->trblBorder.width = width;
}

qreal KoBorder::trblBorderWidth() const
{
    return d->trblBorder.width;
}

void KoBorder::setTrblInnerBorderWidth(qreal width)
{
    d->trblBorder.innerWidth = width;
}

qreal KoBorder::trblInnerBorderWidth() const
{
    return d->trblBorder.innerWidth;
}

void KoBorder::setTrblBorderSpacing(qreal width)
{
    d->trblBorder.spacing = width;
}

qreal KoBorder::trblBorderSpacing() const
{
    return d->trblBorder.spacing;
}

KoBorder::BorderData KoBorder::leftBorderData() const
{
    return d->leftBorder;
}

KoBorder::BorderData KoBorder::topBorderData() const
{
    return d->topBorder;
}

KoBorder::BorderData KoBorder::rightBorderData() const
{
    return d->rightBorder;
}

KoBorder::BorderData KoBorder::bottomBorderData() const
{
    return d->bottomBorder;
}

KoBorder::BorderData KoBorder::trblBorderData() const
{
    return d->trblBorder;
}

KoBorder::BorderData KoBorder::tlbrBorderData() const
{
    return d->tlbrBorder;
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
            setLeftBorderColor(allBordersColor);
            setTopBorderColor(allBordersColor);
            setRightBorderColor(allBordersColor);
            setBottomBorderColor(allBordersColor);
        }
        if (foundStyle) {
            setLeftBorderStyle(allBordersStyle);
            setTopBorderStyle(allBordersStyle);
            setRightBorderStyle(allBordersStyle);
            setBottomBorderStyle(allBordersStyle);
        }
        if (foundWidth) {
            setLeftBorderWidth(allBordersWidth);
            setTopBorderWidth(allBordersWidth);
            setRightBorderWidth(allBordersWidth);
            setBottomBorderWidth(allBordersWidth);
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
                setLeftBorderColor(borderColor);
            }
            if (foundStyle) {
                setLeftBorderStyle(borderStyle);
            }
            if (foundWidth) {
                setLeftBorderWidth(borderWidth);
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
                setTopBorderColor(borderColor);
            }
            if (foundStyle) {
                setTopBorderStyle(borderStyle);
            }
            if (foundWidth) {
                setTopBorderWidth(borderWidth);
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
                setRightBorderColor(borderColor);
            }
            if (foundStyle) {
                setRightBorderStyle(borderStyle);
            }
            if (foundWidth) {
                setRightBorderWidth(borderWidth);
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
                setBottomBorderColor(borderColor);
            }
            if (foundStyle) {
                setBottomBorderStyle(borderStyle);
            }
            if (foundWidth) {
                setBottomBorderWidth(borderWidth);
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
            setTlbrBorderColor(borderColor);
        }
        if (foundStyle) {
            setTlbrBorderStyle(borderStyle);
        }
        if (foundWidth) {
            setTlbrBorderWidth(borderWidth);
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
            setTrblBorderColor(borderColor);
        }
        if (foundStyle) {
            setTrblBorderStyle(borderStyle);
        }
        if (foundWidth) {
            setTrblBorderWidth(borderWidth);
        }
    }

    // Handle double borders.
    if (style.hasAttributeNS(KoXmlNS::style, "border-line-width")) {
        result = true;
        QString borderLineWidth = style.attributeNS(KoXmlNS::style, "border-line-width");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setLeftInnerBorderWidth(KoUnit::parseValue(blw[0], 0.1));
            setLeftBorderSpacing(KoUnit::parseValue(blw[1], 1.0));
            setLeftBorderWidth(KoUnit::parseValue(blw[2], 0.1));

            setTopInnerBorderWidth(KoUnit::parseValue(blw[0], 0.1));
            setTopBorderSpacing(KoUnit::parseValue(blw[1], 1.0));
            setTopBorderWidth(KoUnit::parseValue(blw[2], 0.1));

            setRightInnerBorderWidth(KoUnit::parseValue(blw[0], 0.1));
            setRightBorderSpacing(KoUnit::parseValue(blw[1], 1.0));
            setRightBorderWidth(KoUnit::parseValue(blw[2], 0.1));

            setBottomInnerBorderWidth(KoUnit::parseValue(blw[0], 0.1));
            setBottomBorderSpacing(KoUnit::parseValue(blw[1], 1.0));
            setBottomBorderWidth(KoUnit::parseValue(blw[2], 0.1));
        }
    }
    else {
        if (style.hasAttributeNS(KoXmlNS::style, "border-line-width-left")) {
            result = true;
            QString borderLineWidth = style.attributeNS(KoXmlNS::style, "border-line-width-left");
            if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
                QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
                setLeftInnerBorderWidth(KoUnit::parseValue(blw[0], 0.1));
                setLeftBorderSpacing(KoUnit::parseValue(blw[1], 1.0));
                setLeftBorderWidth(KoUnit::parseValue(blw[2], 0.1));
            }
        }
        if (style.hasAttributeNS(KoXmlNS::style, "border-line-width-top")) {
            result = true;
            QString borderLineWidth = style.attributeNS(KoXmlNS::style, "border-line-width-top");
            if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
                QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
                setTopInnerBorderWidth(KoUnit::parseValue(blw[0], 0.1));
                setTopBorderSpacing(KoUnit::parseValue(blw[1], 1.0));
                setTopBorderWidth(KoUnit::parseValue(blw[2], 0.1));
            }
        }
        if (style.hasAttributeNS(KoXmlNS::style, "border-line-width-right")) {
            result = true;
            QString borderLineWidth = style.attributeNS(KoXmlNS::style, "border-line-width-right");
            if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
                QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
                setRightInnerBorderWidth(KoUnit::parseValue(blw[0], 0.1));
                setRightBorderSpacing(KoUnit::parseValue(blw[1], 1.0));
                setRightBorderWidth(KoUnit::parseValue(blw[2], 0.1));
            }
        }
        if (style.hasAttributeNS(KoXmlNS::style, "border-line-width-bottom")) {
            result = true;
            QString borderLineWidth = style.attributeNS(KoXmlNS::style, "border-line-width-bottom");
            if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
                QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
                setBottomInnerBorderWidth(KoUnit::parseValue(blw[0], 0.1));
                setBottomBorderSpacing(KoUnit::parseValue(blw[1], 1.0));
                setBottomBorderWidth(KoUnit::parseValue(blw[2], 0.1));
            }
        }
    }

    if (style.hasAttributeNS(KoXmlNS::style, "diagonal-tl-br-widths")) {
        result = true;
        QString borderLineWidth = style.attributeNS(KoXmlNS::style, "diagonal-tl-br-widths");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setTlbrInnerBorderWidth(KoUnit::parseValue(blw[0], 0.1));
            setTlbrBorderSpacing(KoUnit::parseValue(blw[1], 1.0));
            setTlbrBorderWidth(KoUnit::parseValue(blw[2], 0.1));
        }
    }
    if (style.hasAttributeNS(KoXmlNS::style, "diagonal-bl-tr-widths")) {
        result = true;
        QString borderLineWidth = style.attributeNS(KoXmlNS::style, "diagonal-bl-tr-widths");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setTrblInnerBorderWidth(KoUnit::parseValue(blw[0], 0.1));
            setTrblBorderSpacing(KoUnit::parseValue(blw[1], 1.0));
            setTrblBorderWidth(KoUnit::parseValue(blw[2], 0.1));
        }
    }
    return result;
}

void KoBorder::saveOdf(KoGenStyle &style, KoGenStyle::PropertyType type) const
{
    kWarning(32500) << "Saving border";
    // Get the strings that describe respective borders.
    QString leftBorderString = QString("%1pt %2 %3")
                                 .arg(QString::number(leftBorderWidth()),
                                      odfBorderStyleString(leftBorderStyle()),
                                      leftBorderColor().name());
    QString rightBorderString =  QString("%1pt %2 %3")
                                  .arg(QString::number(rightBorderWidth()),
                                       odfBorderStyleString(rightBorderStyle()),
                                       rightBorderColor().name());
    QString topBorderString = QString("%1pt %2 %3")
                                .arg(QString::number(topBorderWidth()),
                                     odfBorderStyleString(topBorderStyle()),
                                     topBorderColor().name());
    QString bottomBorderString = QString("%1pt %2 %3")
                                   .arg(QString::number(bottomBorderWidth()),
                                        odfBorderStyleString(bottomBorderStyle()),
                                        bottomBorderColor().name());

    QString tlbrBorderString = QString("%1pt %2 %3")
                                .arg(QString::number(tlbrBorderWidth()),
                                     odfBorderStyleString(tlbrBorderStyle()),
                                     tlbrBorderColor().name());
    QString trblBorderString = QString("%1pt %2 %3")
                                   .arg(QString::number(trblBorderWidth()),
                                        odfBorderStyleString(trblBorderStyle()),
                                        trblBorderColor().name());

    // Check if we can save all borders in one fo:border attribute, or
    // if we have to use several different ones like fo:border-left, etc.
    if (leftBorderString == rightBorderString
        && leftBorderString == topBorderString
        && leftBorderString == bottomBorderString) {

        // Yes, they were all the same, so use only fo:border
        style.addProperty("fo:border", leftBorderString, type);
    } else {
        // No, they were different, so use the individual borders.
        //if (leftBorderStyle() != BorderNone)
            style.addProperty("fo:border-left", leftBorderString, type);
        //if (rightBorderStyle() != BorderNone)
            style.addProperty("fo:border-right", rightBorderString, type);
        //if (topBorderStyle() != BorderNone)
            style.addProperty("fo:border-top", topBorderString, type);
        //if (bottomBorderStyle() != BorderNone)
            style.addProperty("fo:border-bottom", bottomBorderString, type);
    }

    //if (tlbrBorderStyle() != BorderNone) {
        style.addProperty("style:diagonal-tl-br", tlbrBorderString, type);
    //}
    //if (trblBorderStyle() != BorderNone) {
        style.addProperty("style:diagonal-bl-tr", trblBorderString, type);
    //}

    // Handle double borders
    QString leftBorderLineWidth = QString("%1pt %2pt %3pt")
                                    .arg(QString::number(leftInnerBorderWidth()),
                                         QString::number(leftBorderSpacing()),
                                         QString::number(leftBorderWidth()));
    QString rightBorderLineWidth = QString("%1pt %2pt %3pt")
                                     .arg(QString::number(rightInnerBorderWidth()),
                                          QString::number(rightBorderSpacing()),
                                          QString::number(rightBorderWidth()));
    QString topBorderLineWidth = QString("%1pt %2pt %3pt")
                                   .arg(QString::number(topInnerBorderWidth()),
                                        QString::number(topBorderSpacing()),
                                        QString::number(topBorderWidth()));
    QString bottomBorderLineWidth = QString("%1pt %2pt %3pt")
                                      .arg(QString::number(bottomInnerBorderWidth()),
                                           QString::number(bottomBorderSpacing()),
                                           QString::number(bottomBorderWidth()));

    QString tlbrBorderLineWidth = QString("%1pt %2pt %3pt")
                                   .arg(QString::number(tlbrInnerBorderWidth()),
                                        QString::number(tlbrBorderSpacing()),
                                        QString::number(tlbrBorderWidth()));
    QString trblBorderLineWidth = QString("%1pt %2pt %3pt")
                                      .arg(QString::number(trblInnerBorderWidth()),
                                           QString::number(trblBorderSpacing()),
                                           QString::number(trblBorderWidth()));

    if (leftBorderLineWidth == rightBorderLineWidth
        && leftBorderLineWidth == topBorderLineWidth
        && leftBorderLineWidth == bottomBorderLineWidth
        && leftBorderStyle() == BorderDouble
        && rightBorderStyle() == BorderDouble
        && topBorderStyle() == BorderDouble
        && bottomBorderStyle() == BorderDouble) {
        style.addProperty("style:border-line-width", leftBorderLineWidth, type);
    } else {
        if (leftBorderStyle() == BorderDouble)
            style.addProperty("style:border-line-width-left", leftBorderLineWidth, type);
        if (rightBorderStyle() == BorderDouble)
            style.addProperty("style:border-line-width-right", rightBorderLineWidth, type);
        if (topBorderStyle() == BorderDouble)
            style.addProperty("style:border-line-width-top", topBorderLineWidth, type);
        if (bottomBorderStyle() == BorderDouble)
            style.addProperty("style:border-line-width-bottom", bottomBorderLineWidth, type);
    }

    if (tlbrBorderStyle() == BorderDouble) {
        style.addProperty("style:diagonal-tl-br-widths", tlbrBorderLineWidth, type);
    }
    if (trblBorderStyle() == BorderDouble) {
        style.addProperty("style:diagonal-bl-tr-widths", trblBorderLineWidth, type);
    }
}
