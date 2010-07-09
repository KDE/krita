/* This file is part of the KDE project
 *
 * Copyright (C) 2009 Inge Wallin <inge@lysator.liu.se>
 * Copyright (C) 2009 Thomas Zander <zander@kde.org>
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

KoBorder::BorderStyle KoBorder::odfBorderStyle(const QString &borderstyle)
{
    // Note: the styles marked "Not odf compatible" below are legacies
    //       from the old kword format.  There are also lots of border
    //       styles in the MS DOC that we may have to handle at some point.
    if (borderstyle == "none")
        return BorderNone;
    if (borderstyle == "dashed")
        return BorderDashed;
    if (borderstyle == "dotted")
        return BorderDotted;
    if (borderstyle == "dot-dash")
        return BorderDashDotPattern;
    if (borderstyle == "dot-dot-dash")
        return BorderDashDotDotPattern;
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

    // Not needed to handle "solid" since it's the default.
    return BorderSolid;
}

QString KoBorder::odfBorderStyleString(BorderStyle borderstyle)
{
    switch (borderstyle) {
    case BorderDashed:
        return QString("dashed");
    case BorderDotted:
        return QString("dotted");
    case BorderDashDotPattern:
        return QString("dot-dash");
    case BorderDashDotDotPattern:
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


// ----------------------------------------------------------------
//                         load and save

void KoBorder::loadOdf(const KoXmlElement &style)
{
    if (style.hasAttributeNS(KoXmlNS::fo, "border")) {
        QString border = style.attributeNS(KoXmlNS::fo, "border");

        //kDebug() << "*** *** Found border: " << border;

        if (!border.isEmpty() && border != "none" && border != "hidden") {
            QStringList borderData = border.split(' ', QString::SkipEmptyParts);

            const qreal borderWidth = KoUnit::parseValue(borderData[0], 1.0);
            const BorderStyle borderStyle = odfBorderStyle(borderData[1]);
            const QColor borderColor = QColor(borderData[2]);

            setLeftBorderWidth(borderWidth);
            setLeftBorderStyle(borderStyle);
            setLeftBorderColor(borderColor);

            setTopBorderWidth(borderWidth);
            setTopBorderStyle(borderStyle);
            setTopBorderColor(borderColor);

            setRightBorderWidth(borderWidth);
            setRightBorderStyle(borderStyle);
            setRightBorderColor(borderColor);

            setBottomBorderWidth(borderWidth);
            setBottomBorderStyle(borderStyle);
            setBottomBorderColor(borderColor);
        }
    }
    else {
        // No common border attributes, check for the individual ones.
        if (style.hasAttributeNS(KoXmlNS::fo, "border-left")) {
            QString border = style.attributeNS(KoXmlNS::fo, "border-left");
            if (!border.isEmpty() && border != "none" && border != "hidden") {

                QStringList borderData = border.split(' ', QString::SkipEmptyParts);

                setLeftBorderWidth(KoUnit::parseValue(borderData[0], 1.0));
                setLeftBorderStyle(odfBorderStyle(borderData[1]));
                QString specialBorderStyle = style.attributeNS(KoXmlNS::koffice, "specialborder-left");
                if (!specialBorderStyle.isEmpty()) {
                    setLeftBorderStyle(odfBorderStyle(specialBorderStyle));
                }
                setLeftBorderColor(QColor(borderData[2]));
            }
        }
        if (style.hasAttributeNS(KoXmlNS::fo, "border-top")) {
            QString border = style.attributeNS(KoXmlNS::fo, "border-top");
            if (!border.isEmpty() && border != "none" && border != "hidden") {
                QStringList borderData = border.split(' ', QString::SkipEmptyParts);
                setTopBorderWidth(KoUnit::parseValue(borderData[0], 1.0));
                setTopBorderStyle(odfBorderStyle(borderData[1]));
                QString specialBorderStyle = style.attributeNS(KoXmlNS::koffice, "specialborder-top");
                if (!specialBorderStyle.isEmpty()) {
                    setTopBorderStyle(odfBorderStyle(specialBorderStyle));
                }
                setTopBorderColor(QColor(borderData[2]));
            }
        }
        if (style.hasAttributeNS(KoXmlNS::fo, "border-right")) {
            QString border = style.attributeNS(KoXmlNS::fo, "border-right");
            if (!border.isEmpty() && border != "none" && border != "hidden") {
                QStringList borderData = border.split(' ', QString::SkipEmptyParts);
                setRightBorderWidth(KoUnit::parseValue(borderData[0], 1.0));
                setRightBorderStyle(odfBorderStyle(borderData[1]));
                QString specialBorderStyle = style.attributeNS(KoXmlNS::koffice, "specialborder-right");
                if (!specialBorderStyle.isEmpty()) {
                    setRightBorderStyle(odfBorderStyle(specialBorderStyle));
                }
                setRightBorderColor(QColor(borderData[2]));
            }
        }
        if (style.hasAttributeNS(KoXmlNS::fo, "border-bottom")) {
            QString border = style.attributeNS(KoXmlNS::fo, "border-bottom");
            if (!border.isEmpty() && border != "none" && border != "hidden") {
                QStringList borderData = border.split(' ', QString::SkipEmptyParts);
                setBottomBorderWidth(KoUnit::parseValue(borderData[0], 1.0));
                setBottomBorderStyle(odfBorderStyle(borderData[1]));
                QString specialBorderStyle = style.attributeNS(KoXmlNS::koffice, "specialborder-bottom");
                if (!specialBorderStyle.isEmpty()) {
                    setBottomBorderStyle(odfBorderStyle(specialBorderStyle));
                }
                setBottomBorderColor(QColor(borderData[2]));
            }
        }
    }

    // Handle double borders.
    if (style.hasAttributeNS(KoXmlNS::style, "border-line-width")) {
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
            QString borderLineWidth = style.attributeNS(KoXmlNS::style, "border-line-width-left");
            if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
                QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
                setLeftInnerBorderWidth(KoUnit::parseValue(blw[0], 0.1));
                setLeftBorderSpacing(KoUnit::parseValue(blw[1], 1.0));
                setLeftBorderWidth(KoUnit::parseValue(blw[2], 0.1));
            }
        }
        if (style.hasAttributeNS(KoXmlNS::style, "border-line-width-top")) {
            QString borderLineWidth = style.attributeNS(KoXmlNS::style, "border-line-width-top");
            if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
                QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
                setTopInnerBorderWidth(KoUnit::parseValue(blw[0], 0.1));
                setTopBorderSpacing(KoUnit::parseValue(blw[1], 1.0));
                setTopBorderWidth(KoUnit::parseValue(blw[2], 0.1));
            }
        }
        if (style.hasAttributeNS(KoXmlNS::style, "border-line-width-right")) {
            QString borderLineWidth = style.attributeNS(KoXmlNS::style, "border-line-width-right");
            if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
                QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
                setRightInnerBorderWidth(KoUnit::parseValue(blw[0], 0.1));
                setRightBorderSpacing(KoUnit::parseValue(blw[1], 1.0));
                setRightBorderWidth(KoUnit::parseValue(blw[2], 0.1));
            }
        }
        if (style.hasAttributeNS(KoXmlNS::style, "border-line-width-bottom")) {
            QString borderLineWidth = style.attributeNS(KoXmlNS::style, "border-line-width-bottom");
            if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
                QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
                setBottomInnerBorderWidth(KoUnit::parseValue(blw[0], 0.1));
                setBottomBorderSpacing(KoUnit::parseValue(blw[1], 1.0));
                setBottomBorderWidth(KoUnit::parseValue(blw[2], 0.1));
            }
        }
    }
}

void KoBorder::saveOdf(KoGenStyle &style) const
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

    // Check if we can save all borders in one fo:border attribute, or
    // if we have to use several different ones like fo:border-left, etc.
    if (leftBorderString == rightBorderString
        && leftBorderString == topBorderString
        && leftBorderString == bottomBorderString) {

        // Yes, they were all the same, so use only fo:border
        if (leftBorderStyle() != BorderNone)
            style.addProperty("fo:border", leftBorderString);
    } else {
        // No, they were different, so use the individual borders.
        if (leftBorderStyle() != BorderNone)
            style.addProperty("fo:border-left", leftBorderString);
        if (rightBorderStyle() != BorderNone)
            style.addProperty("fo:border-right", rightBorderString);
        if (topBorderStyle() != BorderNone)
            style.addProperty("fo:border-top", topBorderString);
        if (bottomBorderStyle() != BorderNone)
            style.addProperty("fo:border-bottom", bottomBorderString);
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

    if (leftBorderLineWidth == rightBorderLineWidth
        && leftBorderLineWidth == topBorderLineWidth
        && leftBorderLineWidth == bottomBorderLineWidth
        && leftBorderStyle() == BorderDouble
        && rightBorderStyle() == BorderDouble
        && topBorderStyle() == BorderDouble
        && bottomBorderStyle() == BorderDouble) {
        style.addProperty("style:border-line-width", leftBorderLineWidth);
    } else {
        if (leftBorderStyle() == BorderDouble)
            style.addProperty("style:border-line-width-left", leftBorderLineWidth);
        if (rightBorderStyle() == BorderDouble)
            style.addProperty("style:border-line-width-right", rightBorderLineWidth);
        if (topBorderStyle() == BorderDouble)
            style.addProperty("style:border-line-width-top", topBorderLineWidth);
        if (bottomBorderStyle() == BorderDouble)
            style.addProperty("style:border-line-width-bottom", bottomBorderLineWidth);
    }
}
