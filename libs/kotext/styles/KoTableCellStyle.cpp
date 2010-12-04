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
#include "KoTableCellStyle_p.h"

#include <KDebug>

#include <QTextTable>
#include <QTextTableFormat>

#include <KoUnit.h>
#include <KoStyleStack.h>
#include <KoOdfLoadingContext.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>

KoTableCellStylePrivate::KoTableCellStylePrivate()
    : parentStyle(0)
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
}

KoTableCellStyle::KoTableCellStyle(const QTextTableCellFormat &format, QObject *parent)
    : KoTableBorderStyle(*new KoTableCellStylePrivate(), format, parent)
{
    Q_D(KoTableCellStyle);
    d->stylesPrivate = format.properties();
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

void KoTableCellStyle::loadOdf(const KoXmlElement *element, KoOdfLoadingContext &context)
{
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

    // Alignment
    const QString verticalAlign(styleStack.property(KoXmlNS::style, "vertical-align"));
    if (!verticalAlign.isEmpty()) {
        setAlignment(KoText::valignmentFromString(verticalAlign));
    }
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
