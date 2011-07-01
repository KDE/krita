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
#include "KoTableCellStyle.h"
#include "KoStyleManager.h"
#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include "Styles_p.h"
#include "KoTextDocument.h"
#include "KoTableCellStyle_p.h"
#include <KoShapeLoadingContext.h>
#include "KoCharacterStyle.h"

#include <KDebug>

#include <QTextTable>
#include <QTextTableFormat>

#include <KoUnit.h>
#include <KoStyleStack.h>
#include <KoOdfLoadingContext.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>

#include <cfloat>

KoTableCellStyle::RotationAlignment rotationAlignmentFromString(const QString& align)
{
    if (align == "bottom")
        return KoTableCellStyle::RAlignBottom;
    if (align == "center")
        return KoTableCellStyle::RAlignCenter;
    if (align == "top")
        return KoTableCellStyle::RAlignTop;
    
    return KoTableCellStyle::RAlignNone;
}

QString rotationAlignmentToString(KoTableCellStyle::RotationAlignment align)
{
    if (align == KoTableCellStyle::RAlignBottom)
        return "bottom";
    if (align == KoTableCellStyle::RAlignTop)
        return "top";
    if (align == KoTableCellStyle::RAlignCenter)
        return "center";
    return "none";
}

KoTableCellStylePrivate::KoTableCellStylePrivate()
    : charStyle(0)
    , parentStyle(0)
    , next(0)
{
}

KoTableCellStylePrivate::~KoTableCellStylePrivate()
{
}

void KoTableCellStylePrivate::setProperty(int key, const QVariant &value)
{
    stylesPrivate.add(key, value);
}

KoTableCellStyle::KoTableCellStyle(QObject *parent)
    : KoTableBorderStyle(*new KoTableCellStylePrivate(), parent)
{
    Q_D(KoTableCellStyle);
    d->charStyle = new KoCharacterStyle(this);
}

KoTableCellStyle::KoTableCellStyle(const QTextTableCellFormat &format, QObject *parent)
    : KoTableBorderStyle(*new KoTableCellStylePrivate(), format, parent)
{
    Q_D(KoTableCellStyle);
    d->stylesPrivate = format.properties();
    d->charStyle = new KoCharacterStyle(this);
}

KoTableCellStyle::~KoTableCellStyle()
{
}

KoTableCellStyle *KoTableCellStyle::fromTableCell(const QTextTableCell &tableCell, QObject *parent)
{
    QTextTableCellFormat tableCellFormat = tableCell.format().toTableCellFormat();
    return new KoTableCellStyle(tableCellFormat, parent);
}

QRectF KoTableCellStyle::contentRect(const QRectF &boundingRect) const
{
    Q_D(const KoTableCellStyle);
    return boundingRect.adjusted(
                d->edges[Left].outerPen.widthF() + d->edges[Left].spacing + d->edges[Left].innerPen.widthF() + propertyDouble(QTextFormat::TableCellLeftPadding),
                d->edges[Top].outerPen.widthF() + d->edges[Top].spacing + d->edges[Top].innerPen.widthF() + propertyDouble(QTextFormat::TableCellTopPadding),
                - d->edges[Right].outerPen.widthF() - d->edges[Right].spacing - d->edges[Right].innerPen.widthF() - propertyDouble(QTextFormat::TableCellRightPadding),
                - d->edges[Bottom].outerPen.widthF() - d->edges[Bottom].spacing - d->edges[Bottom].innerPen.widthF() - propertyDouble(QTextFormat::TableCellBottomPadding)
   );
}

QRectF KoTableCellStyle::boundingRect(const QRectF &contentRect) const
{
    Q_D(const KoTableCellStyle);
    return contentRect.adjusted(
                - d->edges[Left].outerPen.widthF() - d->edges[Left].spacing - d->edges[Left].innerPen.widthF() - propertyDouble(QTextFormat::TableCellLeftPadding),
                - d->edges[Top].outerPen.widthF() - d->edges[Top].spacing - d->edges[Top].innerPen.widthF() - propertyDouble(QTextFormat::TableCellTopPadding),
                d->edges[Right].outerPen.widthF() + d->edges[Right].spacing + d->edges[Right].innerPen.widthF() + propertyDouble(QTextFormat::TableCellRightPadding),
                d->edges[Bottom].outerPen.widthF() + d->edges[Bottom].spacing + d->edges[Bottom].innerPen.widthF() + propertyDouble(QTextFormat::TableCellBottomPadding)
   );
}

void KoTableCellStyle::paintBackground(QPainter &painter, const QRectF &bounds) const
{
    QRectF innerBounds = bounds;

    if (hasProperty(CellBackgroundBrush)) {
        painter.fillRect(bounds, background());
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
    Q_D(KoTableCellStyle);
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

void KoTableCellStyle::setPadding(qreal padding)
{
    setBottomPadding(padding);
    setTopPadding(padding);
    setRightPadding(padding);
    setLeftPadding(padding);
}

KoCharacterStyle *KoTableCellStyle::characterStyle()
{
    Q_D(KoTableCellStyle);
    return d->charStyle;
}

void KoTableCellStyle::setCharacterStyle(KoCharacterStyle *style)
{
    Q_D(KoTableCellStyle);
    if (d->charStyle == style) {
        return;
    }
    if (d->charStyle && d->charStyle->parent() == this) {
        delete d->charStyle;
    }
    d->charStyle = style;
}

bool KoTableCellStyle::shrinkToFit() const
{
    return propertyBoolean(ShrinkToFit);
}

void KoTableCellStyle::setShrinkToFit(bool state)
{
    setProperty(ShrinkToFit, state);
}

void KoTableCellStyle::setProperty(int key, const QVariant &value)
{
    Q_D(KoTableCellStyle);
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
    Q_D(KoTableCellStyle);
    d->stylesPrivate.remove(key);
}

QVariant KoTableCellStyle::value(int key) const
{
    Q_D(const KoTableCellStyle);
    QVariant var = d->stylesPrivate.value(key);
    if (var.isNull() && d->parentStyle)
        var = d->parentStyle->value(key);
    return var;
}

bool KoTableCellStyle::hasProperty(int key) const
{
    Q_D(const KoTableCellStyle);
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
    Q_D(const KoTableCellStyle);
    if (d->parentStyle) {
        d->parentStyle->applyStyle(format);
    }
    if (d->charStyle) {
        d->charStyle->applyStyle(format);
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
    Q_D(KoTableCellStyle);
    d->stylesPrivate.remove(CellBackgroundBrush);
}

QBrush KoTableCellStyle::background() const
{
    Q_D(const KoTableCellStyle);
    QVariant variant = d->stylesPrivate.value(CellBackgroundBrush);

    if (variant.isNull()) {
        return QBrush();
    }
    return qvariant_cast<QBrush>(variant);
}

void KoTableCellStyle::setWrap(bool state)
{
    setProperty(Wrap, state);
}

bool KoTableCellStyle::wrap() const
{
    return propertyBoolean(Wrap);
}

void KoTableCellStyle::setAlignment(Qt::Alignment alignment)
{
    setProperty(VerticalAlignment, (int) alignment);
}

Qt::Alignment KoTableCellStyle::alignment() const
{
    if (propertyInt(VerticalAlignment) == 0)
        return Qt::AlignTop;
    return static_cast<Qt::Alignment>(propertyInt(VerticalAlignment));
}

KoTableCellStyle *KoTableCellStyle::parentStyle() const
{
    Q_D(const KoTableCellStyle);
    return d->parentStyle;
}

QString KoTableCellStyle::name() const
{
    Q_D(const KoTableCellStyle);
    return d->name;
}

void KoTableCellStyle::setName(const QString &name)
{
    Q_D(KoTableCellStyle);
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
    Q_D(KoTableCellStyle);
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

void KoTableCellStyle::setCellProtection(KoTableCellStyle::CellProtectionFlag protection)
{
    setProperty(CellProtection, protection);
}

KoTableCellStyle::CellProtectionFlag KoTableCellStyle::cellProtection() const
{
    return (CellProtectionFlag) propertyInt(CellProtection);
}

void KoTableCellStyle::setTextDirection(KoText::Direction value)
{
    setProperty(TextWritingMode, value);
}

KoText::Direction KoTableCellStyle::textDirection() const
{
    return (KoText::Direction) propertyInt(TextWritingMode);
}

bool KoTableCellStyle::printContent() const
{
    return (hasProperty(PrintContent) && propertyBoolean(PrintContent));
}

void KoTableCellStyle::setPrintContent(bool state)
{
    setProperty(PrintContent, state);
}

bool KoTableCellStyle::repeatContent() const
{
    return (hasProperty(RepeatContent) && propertyBoolean(RepeatContent));
}

void KoTableCellStyle::setRepeatContent(bool state)
{
    setProperty(RepeatContent, state);
}

int KoTableCellStyle::decimalPlaces() const
{
    return propertyInt(DecimalPlaces);
}

void KoTableCellStyle::setDecimalPlaces(int places)
{
    setProperty(DecimalPlaces, places);
}

bool KoTableCellStyle::alignFromType() const
{
    return (hasProperty(AlignFromType) && propertyBoolean(AlignFromType));
}

void KoTableCellStyle::setAlignFromType(bool state)
{
    setProperty(AlignFromType, state);
}

qreal KoTableCellStyle::rotationAngle() const
{
    return propertyDouble(RotationAngle);
}

void KoTableCellStyle::setRotationAngle(qreal value)
{
    if (value >= 0)
        setProperty(RotationAngle, value);
}

void KoTableCellStyle::setVerticalGlyphOrientation(bool state)
{
    setProperty(VerticalGlyphOrientation, state);
}

bool KoTableCellStyle::verticalGlyphOrientation() const
{
    if (hasProperty(VerticalGlyphOrientation))
        return propertyBoolean(VerticalGlyphOrientation);
    return true;
}

void KoTableCellStyle::setDirection(KoTableCellStyle::CellTextDirection direction)
{
    setProperty(Direction, direction);
}

KoTableCellStyle::RotationAlignment KoTableCellStyle::rotationAlignment() const
{
    return static_cast<RotationAlignment>(propertyInt(RotationAlign));
}

void KoTableCellStyle::setRotationAlignment(KoTableCellStyle::RotationAlignment align)
{
    setProperty(RotationAlign, align);
}

KoTableCellStyle::CellTextDirection KoTableCellStyle::direction() const
{
    if (hasProperty(Direction))
        return (KoTableCellStyle::CellTextDirection) propertyInt(Direction);
    return KoTableCellStyle::Default;
}

void KoTableCellStyle::loadOdf(const KoXmlElement *element, KoShapeLoadingContext &scontext)
{
    KoOdfLoadingContext &context = scontext.odfLoadingContext();
    Q_D(KoTableCellStyle);
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

    context.styleStack().setTypeProperties("table-cell");
    loadOdfProperties(context.styleStack());

    KoCharacterStyle *charstyle = characterStyle();
    context.styleStack().setTypeProperties("text");   // load all style attributes from "style:text-properties"
    charstyle->loadOdf(scontext);   // load the KoCharacterStyle from the stylestack

    context.styleStack().setTypeProperties("graphic");
    loadOdfProperties(context.styleStack());

    context.styleStack().setTypeProperties("paragraph");
    loadOdfProperties(context.styleStack());
    context.styleStack().restore();
}

void KoTableCellStyle::loadOdfProperties(KoStyleStack &styleStack)
{
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

    // Borders
    if (styleStack.hasProperty(KoXmlNS::fo, "border", "left")) {
        QString border = styleStack.property(KoXmlNS::fo, "border", "left");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KoXmlNS::calligra, "specialborder", "left")) {
            style = styleStack.property(KoXmlNS::calligra, "specialborder", "left");
        }
        if (!border.isEmpty() && border != "none" && border != "hidden") {
            setEdge(Left, oasisBorderStyle(style), KoUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "border", "top")) {
        QString border = styleStack.property(KoXmlNS::fo, "border", "top");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KoXmlNS::calligra, "specialborder", "top")) {
            style = styleStack.property(KoXmlNS::calligra, "specialborder", "top");
        }
        if (!border.isEmpty() && border != "none" && border != "hidden") {
            setEdge(Top, oasisBorderStyle(style), KoUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
        }
    }

    if (styleStack.hasProperty(KoXmlNS::fo, "border", "right")) {
        QString border = styleStack.property(KoXmlNS::fo, "border", "right");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KoXmlNS::calligra, "specialborder", "right")) {
            style = styleStack.property(KoXmlNS::calligra, "specialborder", "right");
        }
        if (!border.isEmpty() && border != "none" && border != "hidden") {
            setEdge(Right, oasisBorderStyle(style), KoUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "border", "bottom")) {
        QString border = styleStack.property(KoXmlNS::fo, "border", "bottom");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KoXmlNS::calligra, "specialborder", "bottom")) {
            style = styleStack.property(KoXmlNS::calligra, "specialborder", "bottom");
        }
        if (!border.isEmpty() && border != "none" && border != "hidden") {
            setEdge(Bottom, oasisBorderStyle(style), KoUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::style, "diagonal-tl-br")) {
        QString border = styleStack.property(KoXmlNS::style, "diagonal-tl-br");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KoXmlNS::calligra, "specialborder", "tl-br")) {
            style = styleStack.property(KoXmlNS::calligra, "specialborder", "tl-br");
        }
        setEdge(TopLeftToBottomRight, oasisBorderStyle(style), KoUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
    }
    if (styleStack.hasProperty(KoXmlNS::style, "diagonal-bl-tr")) {
        QString border = styleStack.property(KoXmlNS::style, "diagonal-bl-tr");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KoXmlNS::calligra, "specialborder", "bl-tr")) {
            style = styleStack.property(KoXmlNS::calligra, "specialborder", "bl-tr");
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

    // The fo:background-color attribute specifies the background color of a cell.
    if (styleStack.hasProperty(KoXmlNS::fo, "background-color")) {
        const QString bgcolor = styleStack.property(KoXmlNS::fo, "background-color");
        QBrush brush = background();
        if (bgcolor == "transparent")
            setBackground(Qt::NoBrush);
        else {
            if (brush.style() == Qt::NoBrush)
                brush.setStyle(Qt::SolidPattern);
            brush.setColor(bgcolor); // #rrggbb format
            setBackground(brush);
        }
    }

    if (styleStack.hasProperty(KoXmlNS::draw, "opacity")) {
        const QString opacity = styleStack.property(KoXmlNS::draw, "opacity");
        if (!opacity.isEmpty() && opacity.right(1) == "%") {
            float percent = opacity.left(opacity.length() - 1).toFloat();
            QBrush brush = background();
            QColor color = brush.color();
            color.setAlphaF(percent / 100.0);
            brush.setColor(color);
            setBackground(brush);
        }
    }

    if (styleStack.hasProperty(KoXmlNS::style, "shrink-to-fit")) {
        setShrinkToFit(styleStack.property(KoXmlNS::style, "shrink-to-fit") == "true");
    }
    
    if (styleStack.hasProperty(KoXmlNS::style, "print-content")) {
        setPrintContent(styleStack.property(KoXmlNS::style, "print-content") == "true");
    }
    
    if (styleStack.hasProperty(KoXmlNS::style, "repeat-content")) {
        setRepeatContent(styleStack.property(KoXmlNS::style, "repeat-content") == "true");
    }
    
    if (styleStack.hasProperty(KoXmlNS::style, "repeat-content")) {
        setRepeatContent(styleStack.property(KoXmlNS::style, "repeat-content") == "true");
    }
    
    if (styleStack.hasProperty(KoXmlNS::style, "decimal-places")) {
        bool ok;
        int value = styleStack.property(KoXmlNS::style, "decimal-places").toInt(&ok);
        if (ok)
            setDecimalPlaces(value);
    }
    
    if (styleStack.hasProperty(KoXmlNS::style, "rotation-angle")) {
        setRotationAngle(KoUnit::parseAngle(styleStack.property(KoXmlNS::style, "rotation-angle")));
    }
    
    if (styleStack.hasProperty(KoXmlNS::style, "glyph-orientation-vertical"))
    {
        setVerticalGlyphOrientation(styleStack.property(KoXmlNS::style, "glyph-orientation-vertical") == "auto");
    }
    
    if (styleStack.hasProperty(KoXmlNS::style, "direction")) {
        if (styleStack.property(KoXmlNS::style, "direction") == "ltr")
            setDirection(KoTableCellStyle::LeftToRight);
        else
            setDirection(KoTableCellStyle::TopToBottom);
    }
    
    if (styleStack.hasProperty(KoXmlNS::style, "rotation-align")) {
        setRotationAlignment(rotationAlignmentFromString(styleStack.property(KoXmlNS::style, "rotation-align")));
    }
    
    if (styleStack.hasProperty(KoXmlNS::style, "text-align-source")) {
        setAlignFromType(styleStack.property(KoXmlNS::style, "text-align-source") == "value-type");
    }
    
    if (styleStack.hasProperty(KoXmlNS::fo, "wrap-option")) {
        setWrap(styleStack.property(KoXmlNS::fo, "wrap-option") == "wrap");
    }
    
    if (styleStack.hasProperty(KoXmlNS::style, "cell-protect")) {
        QString protection = styleStack.property(KoXmlNS::style, "cell-protect");
        if (protection == "none")
            setCellProtection(NoProtection);
        else if (protection == "hidden-and-protected")
            setCellProtection(HiddenAndProtected);
        else if (protection == "protected")
            setCellProtection(Protected);
        else if (protection == "formula-hidden")
            setCellProtection(FormulaHidden);
        else if ((protection == "protected formula-hidden") || (protection == "formula-hidden protected"))
            setCellProtection(ProtectedAndFormulaHidden);
    }
    // Alignment
    const QString verticalAlign(styleStack.property(KoXmlNS::style, "vertical-align"));
    if (!verticalAlign.isEmpty()) {
        if (verticalAlign == "automatic")
            setAlignment((Qt::AlignmentFlag) 0);
        else
            setAlignment(KoText::valignmentFromString(verticalAlign));
    }
    
    if (styleStack.hasProperty(KoXmlNS::style, "writing-mode"))
        setTextDirection(KoText::directionFromString(styleStack.property(KoXmlNS::style, "writing-mode")));
}

void KoTableCellStyle::copyProperties(const KoTableCellStyle *style)
{
    Q_D(KoTableCellStyle);
    const KoTableCellStylePrivate *styleD = static_cast<const KoTableCellStylePrivate*>(style->d_func());

    d->stylesPrivate = styleD->stylesPrivate;
    setName(style->name()); // make sure we emit property change
    d->next = styleD->next;
    d->parentStyle = styleD->parentStyle;
}

KoTableCellStyle *KoTableCellStyle::clone(QObject *parent)
{
    KoTableCellStyle *newStyle = new KoTableCellStyle(parent);
    newStyle->copyProperties(this);
    return newStyle;
}


bool KoTableCellStyle::operator==(const KoTableCellStyle &other) const
{
    Q_D(const KoTableCellStyle);
    const KoTableCellStylePrivate *otherD = static_cast<const KoTableCellStylePrivate*>(other.d_func());
    return otherD->stylesPrivate == d->stylesPrivate;
}

void KoTableCellStyle::removeDuplicates(const KoTableCellStyle &other)
{
    Q_D(KoTableCellStyle);
    const KoTableCellStylePrivate *otherD = static_cast<const KoTableCellStylePrivate*>(other.d_func());
    d->stylesPrivate.removeDuplicates(otherD->stylesPrivate);
}

void KoTableCellStyle::saveOdf(KoGenStyle &style)
{
    Q_D(KoTableCellStyle);
    QList<int> keys = d->stylesPrivate.keys();
    bool donePadding = false;
    if (hasProperty(QTextFormat::TableCellLeftPadding) && 
            hasProperty(QTextFormat::TableCellRightPadding) && 
            hasProperty(QTextFormat::TableCellTopPadding) && 
            hasProperty(QTextFormat::TableCellBottomPadding) && 
            leftPadding() == rightPadding() &&
            topPadding() == bottomPadding() &&
            topPadding() == leftPadding()) {
        donePadding = true;
        style.addPropertyPt("fo:padding", leftPadding(), KoGenStyle::TableCellType);
    }
    foreach(int key, keys) {
        if (key == CellBackgroundBrush) {
            QBrush backBrush = background();
            if (backBrush.style() != Qt::NoBrush)
                style.addProperty("fo:background-color", backBrush.color().name(), KoGenStyle::TableCellType);
            else
                style.addProperty("fo:background-color", "transparent", KoGenStyle::TableCellType);
            if (!backBrush.isOpaque()) {
                style.addProperty("draw:opacity", QString("%1%").arg(backBrush.color().alphaF() * 100.0), KoGenStyle::GraphicType);
            }
        } else if (key == VerticalAlignment) {
            if (propertyInt(VerticalAlignment) == 0)
                style.addProperty("style:vertical-align", "automatic", KoGenStyle::TableCellType);
            else
                style.addProperty("style:vertical-align", KoText::valignmentToString(alignment()), KoGenStyle::TableCellType);
        } else if ((key == QTextFormat::TableCellLeftPadding) && (!donePadding)) {
            style.addPropertyPt("fo:padding-left", leftPadding(), KoGenStyle::TableCellType);
        } else if ((key == QTextFormat::TableCellRightPadding) && (!donePadding)) {
            style.addPropertyPt("fo:padding-right", rightPadding(), KoGenStyle::TableCellType);
        } else if ((key == QTextFormat::TableCellTopPadding) && (!donePadding)) {
            style.addPropertyPt("fo:padding-top", topPadding(), KoGenStyle::TableCellType);
        } else if ((key == QTextFormat::TableCellBottomPadding) && (!donePadding)) {
            style.addPropertyPt("fo:padding-bottom", bottomPadding(), KoGenStyle::TableCellType);
        } else if (key == ShrinkToFit) {
            style.addProperty("style:shrink-to-fit", shrinkToFit(), KoGenStyle::TableCellType);
        } else if (key == PrintContent) {
            style.addProperty("style:print-content", printContent(), KoGenStyle::TableCellType);
        } else if (key == RepeatContent) {
            style.addProperty("style:repeat-content", repeatContent(), KoGenStyle::TableCellType);
        } else if (key == DecimalPlaces) {
            style.addProperty("style:decimal-places", decimalPlaces(), KoGenStyle::TableCellType);
        } else if (key == RotationAngle) {
            QString str;
            str.setNum(rotationAngle(), 'f', DBL_DIG);
            style.addProperty("style:rotation-angle", QString::number(rotationAngle()), KoGenStyle::TableCellType);
        } else if (key == Wrap) {
            if (wrap())
                style.addProperty("fo:wrap-option", "wrap", KoGenStyle::TableCellType);
            else
                style.addProperty("fo:wrap-option", "no-wrap", KoGenStyle::TableCellType);
        } else if (key == Direction) {
            if (direction() == LeftToRight)
                style.addProperty("style:direction", "ltr", KoGenStyle::TableCellType);
            else if (direction() == TopToBottom)
                style.addProperty("style:direction", "ttb", KoGenStyle::TableCellType);
        } else if (key == CellProtection) {
            if (cellProtection() == NoProtection)
                style.addProperty("style:cell-protect", "none", KoGenStyle::TableCellType);
            else if (cellProtection() == HiddenAndProtected)
                style.addProperty("style:cell-protect", "hidden-and-protected", KoGenStyle::TableCellType);
            else if (cellProtection() == Protected)
                style.addProperty("style:cell-protect", "protected", KoGenStyle::TableCellType);
            else if (cellProtection() == FormulaHidden)
                style.addProperty("style:cell-protect", "formula-hidden", KoGenStyle::TableCellType);
            else if (cellProtection() == ProtectedAndFormulaHidden)
                style.addProperty("style:cell-protect", "protected formula-hidden", KoGenStyle::TableCellType);
        } else if (key == AlignFromType) {
            if (alignFromType())
                style.addProperty("style:text-align-source", "value-type", KoGenStyle::TableCellType);
            else
                style.addProperty("style:text-align-source", "fix", KoGenStyle::TableCellType);
        } else if (key == RotationAlign) {
            style.addProperty("style:rotation-align", rotationAlignmentToString(rotationAlignment()), KoGenStyle::TableCellType);
        } else if (key == TextWritingMode) {
            style.addProperty("style:writing-mode", KoText::directionToString(textDirection()), KoGenStyle::TableCellType);
        } else if (key == VerticalGlyphOrientation) {
            if (verticalGlyphOrientation())
                style.addProperty("style:glyph-orientation-vertical", "auto", KoGenStyle::TableCellType);
            else
                style.addProperty("style:glyph-orientation-vertical", "0", KoGenStyle::TableCellType);
        }
    }
    if (d->charStyle) {
        d->charStyle->saveOdf(style);
    }

/*

    // Borders
    if (styleStack.hasProperty(KoXmlNS::fo, "border", "left")) {
        QString border = styleStack.property(KoXmlNS::fo, "border", "left");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KoXmlNS::calligra, "specialborder", "left")) {
            style = styleStack.property(KoXmlNS::calligra, "specialborder", "left");
        }
        if (!border.isEmpty() && border != "none" && border != "hidden") {
            setEdge(Left, oasisBorderStyle(style), KoUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "border", "top")) {
        QString border = styleStack.property(KoXmlNS::fo, "border", "top");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KoXmlNS::calligra, "specialborder", "top")) {
            style = styleStack.property(KoXmlNS::calligra, "specialborder", "top");
        }
        if (!border.isEmpty() && border != "none" && border != "hidden") {
            setEdge(Top, oasisBorderStyle(style), KoUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
        }
    }

    if (styleStack.hasProperty(KoXmlNS::fo, "border", "right")) {
        QString border = styleStack.property(KoXmlNS::fo, "border", "right");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KoXmlNS::calligra, "specialborder", "right")) {
            style = styleStack.property(KoXmlNS::calligra, "specialborder", "right");
        }
        if (!border.isEmpty() && border != "none" && border != "hidden") {
            setEdge(Right, oasisBorderStyle(style), KoUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "border", "bottom")) {
        QString border = styleStack.property(KoXmlNS::fo, "border", "bottom");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KoXmlNS::calligra, "specialborder", "bottom")) {
            style = styleStack.property(KoXmlNS::calligra, "specialborder", "bottom");
        }
        if (!border.isEmpty() && border != "none" && border != "hidden") {
            setEdge(Bottom, oasisBorderStyle(style), KoUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::style, "diagonal-tl-br")) {
        QString border = styleStack.property(KoXmlNS::style, "diagonal-tl-br");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KoXmlNS::calligra, "specialborder", "tl-br")) {
            style = styleStack.property(KoXmlNS::calligra, "specialborder", "tl-br");
        }
        setEdge(TopLeftToBottomRight, oasisBorderStyle(style), KoUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
    }
    if (styleStack.hasProperty(KoXmlNS::style, "diagonal-bl-tr")) {
        QString border = styleStack.property(KoXmlNS::style, "diagonal-bl-tr");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KoXmlNS::calligra, "specialborder", "bl-tr")) {
            style = styleStack.property(KoXmlNS::calligra, "specialborder", "bl-tr");
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
*/
}

#include <KoTableCellStyle.moc>
