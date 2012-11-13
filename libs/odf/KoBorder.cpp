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
#define setBorderSideProperty( side,property,value ) \
    if (!d->data.contains( side )) {\
        BorderData data; \
        data.property = value; \
        d->data[side] = data; \
    } else { \
        d->data[side].property = value; \
    }

#define setPenBorderSideProperty( side,pen,setter,value ) \
    if (!d->data.contains( side )) {\
        BorderData data; \
        data.pen.setter(value); \
        d->data[side] = data; \
    } else { \
        d->data[side].pen.setter(value); \
    }

#define getBorderSideProperty( side,property,default ) \
    if (!d->data.contains( side )) { \
        return default; \
    } else { \
        return d->data[side].property; \
    }

#define getPenBorderSideProperty( side,pen,property,default ) \
    if (!d->data.contains( side )) { \
        return default; \
    } else { \
        return d->data[side].pen.property(); \
    }


#define sideFunctions(side, styleSetter,styleGetter,colorSetter,colorGetter, \
                    widthSetter,widthGetter,innerWidthSetter,innerWidthGetter, \
                    spacingSetter,spacingGetter,dataSetter,dataGetter) \
void KoBorder::styleSetter(BorderStyle style) \
{ \
setBorderSideProperty(side, style, style); \
} \
KoBorder::BorderStyle KoBorder::styleGetter() const \
{ \
getBorderSideProperty(side, style, BorderNone); \
} \
void KoBorder::colorSetter(const QColor &color) \
{ \
setPenBorderSideProperty(side, outerPen, setColor, color); \
} \
QColor KoBorder::colorGetter() const \
{ \
getPenBorderSideProperty(side, outerPen, color, QColor()); \
} \
void KoBorder::widthSetter(qreal width) \
{ \
setPenBorderSideProperty(side, outerPen, setWidthF, width); \
} \
qreal KoBorder::widthGetter() const \
{ \
getPenBorderSideProperty(side, outerPen, widthF, 0); \
} \
void KoBorder::innerWidthSetter(qreal width) \
{ \
setPenBorderSideProperty(side, innerPen, setWidthF, width); \
} \
qreal KoBorder::innerWidthGetter() const \
{ \
getPenBorderSideProperty(side, innerPen, widthF, 0); \
} \
void KoBorder::spacingSetter(qreal width) \
{ \
setBorderSideProperty(side, spacing, width); \
} \
qreal KoBorder::spacingGetter() const \
{ \
getBorderSideProperty(side, spacing, 0); \
} \
void KoBorder::dataSetter(const BorderData &data) \
{ \
d->data[side] = data; \
} \
KoBorder::BorderData KoBorder::dataGetter() const \
{ \
return d->data.value(side, BorderData()); \
}

sideFunctions(Left, setLeftBorderStyle, leftBorderStyle,
              setLeftBorderColor, leftBorderColor, setLeftBorderWidth,
              leftBorderWidth, setLeftInnerBorderWidth, leftInnerBorderWidth,
              setLeftBorderSpacing, leftBorderSpacing, setLeftBorderData, leftBorderData)

sideFunctions(Right, setRightBorderStyle, rightBorderStyle,
              setRightBorderColor, rightBorderColor, setRightBorderWidth,
              rightBorderWidth, setRightInnerBorderWidth, rightInnerBorderWidth,
              setRightBorderSpacing, rightBorderSpacing, setRightBorderData, rightBorderData)

sideFunctions(Top, setTopBorderStyle, topBorderStyle,
              setTopBorderColor, topBorderColor, setTopBorderWidth,
              topBorderWidth, setTopInnerBorderWidth, topInnerBorderWidth,
              setTopBorderSpacing, topBorderSpacing, setTopBorderData, topBorderData)

sideFunctions(Bottom, setBottomBorderStyle, bottomBorderStyle,
              setBottomBorderColor, bottomBorderColor, setBottomBorderWidth,
              bottomBorderWidth, setBottomInnerBorderWidth, bottomInnerBorderWidth,
              setBottomBorderSpacing, bottomBorderSpacing, setBottomBorderData, bottomBorderData)

sideFunctions(BottomLeftToTopRight, setTrblBorderStyle, trblBorderStyle,
              setTrblBorderColor, trblBorderColor, setTrblBorderWidth,
              trblBorderWidth, setTrblInnerBorderWidth, trblInnerBorderWidth,
              setTrblBorderSpacing, trblBorderSpacing, setTrblBorderData, trblBorderData)

sideFunctions(TopLeftToBottomRight, setTlbrBorderStyle, tlbrBorderStyle,
              setTlbrBorderColor, tlbrBorderColor, setTlbrBorderWidth,
              tlbrBorderWidth, setTlbrInnerBorderWidth, tlbrInnerBorderWidth,
              setTlbrBorderSpacing, tlbrBorderSpacing, setTlbrBorderData, tlbrBorderData)


// -------------------------------

bool KoBorder::hasBorder() const
{
    if (d->data.contains(Left) && leftBorderWidth() > 0.0)
        return true;
    if (d->data.contains(Right) && rightBorderWidth() > 0.0)
        return true;
    if (d->data.contains(Top) && topBorderWidth() > 0.0)
        return true;
    if (d->data.contains(Bottom) && bottomBorderWidth() > 0.0)
        return true;
    if (d->data.contains(BottomLeftToTopRight) && trblBorderWidth() > 0.0)
        return true;
    if (d->data.contains(TopLeftToBottomRight) && tlbrBorderWidth() > 0.0)
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
            setLeftBorderColor(allBordersColor);
            setTopBorderColor(allBordersColor);
            setRightBorderColor(allBordersColor);
            setBottomBorderColor(allBordersColor);
        }
        if (style.hasAttributeNS(KoXmlNS::calligra, "specialborder")) {
            allBordersStyle = KoBorder::odfBorderStyle(style.attributeNS(KoXmlNS::calligra, "specialborder"), &foundStyle);
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
            if (style.hasAttributeNS(KoXmlNS::calligra, "specialborder-left")) {
                borderStyle = KoBorder::odfBorderStyle(style.attributeNS(KoXmlNS::calligra, "specialborder-left"), &foundStyle);
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
            if (style.hasAttributeNS(KoXmlNS::calligra, "specialborder-top")) {
                borderStyle = KoBorder::odfBorderStyle(style.attributeNS(KoXmlNS::calligra, "specialborder-top"), &foundStyle);
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
            if (style.hasAttributeNS(KoXmlNS::calligra, "specialborder-right")) {
                borderStyle = KoBorder::odfBorderStyle(style.attributeNS(KoXmlNS::calligra, "specialborder-right"), &foundStyle);
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
            if (style.hasAttributeNS(KoXmlNS::calligra, "specialborder-bottom")) {
                borderStyle = KoBorder::odfBorderStyle(style.attributeNS(KoXmlNS::calligra, "specialborder-bottom"), &foundStyle);
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
        if (style.hasAttributeNS(KoXmlNS::calligra, "specialborder-tl-br")) {
            borderStyle = KoBorder::odfBorderStyle(style.attributeNS(KoXmlNS::calligra, "specialborder-tl-br"), &foundStyle);
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
        if (style.hasAttributeNS(KoXmlNS::calligra, "specialborder-bl-tr")) {
            borderStyle = KoBorder::odfBorderStyle(style.attributeNS(KoXmlNS::calligra, "specialborder-bl-tr"), &foundStyle);
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

    // Get the strings that describe respective special borders (for special mso support).
    QString leftBorderSpecialString = msoBorderStyleString(leftBorderStyle());
    QString rightBorderSpecialString = msoBorderStyleString(rightBorderStyle());
    QString topBorderSpecialString = msoBorderStyleString(topBorderStyle());
    QString bottomBorderSpecialString = msoBorderStyleString(bottomBorderStyle());
    QString tlbrBorderSpecialString = msoBorderStyleString(tlbrBorderStyle());
    QString trblBorderSpecialString = msoBorderStyleString(trblBorderStyle());

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
        && leftBorderStyle() == rightBorderStyle()
        && topBorderStyle() == bottomBorderStyle()
        && topBorderStyle() == leftBorderStyle()
        && (leftBorderStyle() == BorderDouble || leftBorderStyle() == BorderDoubleWave)) {
        style.addProperty("style:border-line-width", leftBorderLineWidth, type);
    } else {
        if (leftBorderStyle() == BorderDouble || leftBorderStyle() == BorderDoubleWave)
            style.addProperty("style:border-line-width-left", leftBorderLineWidth, type);
        if (rightBorderStyle() == BorderDouble || rightBorderStyle() == BorderDoubleWave)
            style.addProperty("style:border-line-width-right", rightBorderLineWidth, type);
        if (topBorderStyle() == BorderDouble || topBorderStyle() == BorderDoubleWave)
            style.addProperty("style:border-line-width-top", topBorderLineWidth, type);
        if (bottomBorderStyle() == BorderDouble || bottomBorderStyle() == BorderDoubleWave)
            style.addProperty("style:border-line-width-bottom", bottomBorderLineWidth, type);
    }

    if (style.type() != KoGenStyle::PageLayoutStyle) {
        if (tlbrBorderStyle() == BorderDouble || tlbrBorderStyle() == BorderDoubleWave) {
            style.addProperty("style:diagonal-tl-br-widths", tlbrBorderLineWidth, type);
        }
        if (trblBorderStyle() == BorderDouble || trblBorderStyle() == BorderDoubleWave) {
            style.addProperty("style:diagonal-bl-tr-widths", trblBorderLineWidth, type);
        }
    }
}
