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
#include <KoOdfGraphicStyles.h>
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
    : QObject(parent)
    , d_ptr(new KoTableCellStylePrivate)
{
    Q_D(KoTableCellStyle);
    d->charStyle = new KoCharacterStyle(this);
}

KoTableCellStyle::KoTableCellStyle(const QTextTableCellFormat &format, QObject *parent)
    : QObject(parent)
    , d_ptr(new KoTableCellStylePrivate)
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
    const KoTableBorderStyle::Edge &leftEdge = getEdge(KoTableBorderStyle::Left);
    const KoTableBorderStyle::Edge &topEdge = getEdge(KoTableBorderStyle::Top);
    const KoTableBorderStyle::Edge &rightEdge = getEdge(KoTableBorderStyle::Right);
    const KoTableBorderStyle::Edge &bottomEdge = getEdge(KoTableBorderStyle::Bottom);

    return boundingRect.adjusted(
                leftEdge.outerPen.widthF() + leftEdge.spacing + leftEdge.innerPen.widthF() + propertyDouble(QTextFormat::TableCellLeftPadding),
                topEdge.outerPen.widthF() + topEdge .spacing + topEdge .innerPen.widthF() + propertyDouble(QTextFormat::TableCellTopPadding),
                - rightEdge.outerPen.widthF() - rightEdge.spacing - rightEdge.innerPen.widthF() - propertyDouble(QTextFormat::TableCellRightPadding),
                - bottomEdge.outerPen.widthF() - bottomEdge.spacing - bottomEdge.innerPen.widthF() - propertyDouble(QTextFormat::TableCellBottomPadding)
   );
}

QRectF KoTableCellStyle::boundingRect(const QRectF &contentRect) const
{
    const KoTableBorderStyle::Edge &leftEdge = getEdge(KoTableBorderStyle::Left);
    const KoTableBorderStyle::Edge &topEdge = getEdge(KoTableBorderStyle::Top);
    const KoTableBorderStyle::Edge &rightEdge = getEdge(KoTableBorderStyle::Right);
    const KoTableBorderStyle::Edge &bottomEdge = getEdge(KoTableBorderStyle::Bottom);

    return contentRect.adjusted(
                - leftEdge.outerPen.widthF() - leftEdge.spacing - leftEdge.innerPen.widthF() - propertyDouble(QTextFormat::TableCellLeftPadding),
                - topEdge.outerPen.widthF() - topEdge.spacing - topEdge.innerPen.widthF() - propertyDouble(QTextFormat::TableCellTopPadding),
                rightEdge.outerPen.widthF() + rightEdge.spacing + rightEdge.innerPen.widthF() + propertyDouble(QTextFormat::TableCellRightPadding),
                bottomEdge.outerPen.widthF() + bottomEdge.spacing + bottomEdge.innerPen.widthF() + propertyDouble(QTextFormat::TableCellBottomPadding)
   );
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

QPen KoTableCellStyle::propertyPen(int key) const
{
    const QVariant prop = value(key);
    if (prop.userType() != QVariant::Pen)
        return QPen(Qt::NoPen);
    return qvariant_cast<QPen>(prop);
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

KoBorder KoTableCellStyle::borders() const
{
    if (hasProperty(Borders))
        return value(Borders).value<KoBorder>();
    return KoBorder();
}

void KoTableCellStyle::setBorders(const KoBorder& borders)
{
    setProperty(Borders, QVariant::fromValue<KoBorder>(borders));
}

KoShadowStyle KoTableCellStyle::shadow() const
{
    if (hasProperty(Shadow))
        return value(Shadow).value<KoShadowStyle>();
    return KoShadowStyle();
}

void KoTableCellStyle::setShadow(const KoShadowStyle& shadow)
{
    setProperty(Shadow, QVariant::fromValue<KoShadowStyle>(shadow));
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
    loadOdfProperties(scontext, context.styleStack());
    
    KoCharacterStyle *charstyle = characterStyle();
    context.styleStack().setTypeProperties("text");   // load all style attributes from "style:text-properties"
    charstyle->loadOdf(scontext);   // load the KoCharacterStyle from the stylestack

    context.styleStack().setTypeProperties("graphic");
    loadOdfProperties(scontext, context.styleStack());

    context.styleStack().setTypeProperties("paragraph");
    loadOdfProperties(scontext, context.styleStack());
    context.styleStack().restore();

    // Borders - ugly mode enabled
    KoBorder styleBorders;
    KoXmlElement elem;
    KoXmlNode node = element->firstChild();
    do {
        if (node.nodeName() == "style:table-cell-properties") {
            elem = node.toElement();
            break;
        }
    } while (!(node = node.nextSibling()).isNull());
    if (styleBorders.loadOdf(elem)) {
        setBorders(styleBorders);
        kWarning(32500) << "Loaded borders successfully";
        KoBorder testBorders = borders();
        kWarning(32500) << (testBorders != KoBorder());
    }
}

void KoTableCellStyle::loadOdfProperties(KoShapeLoadingContext &context, KoStyleStack &styleStack)
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

    if (styleStack.hasProperty(KoXmlNS::style, "shadow")) {
        KoShadowStyle shadow;
        if (shadow.loadOdf(styleStack.property(KoXmlNS::style, "shadow"))) {
            setShadow(shadow);
        }
    }

    // Borders
    if (styleStack.hasProperty(KoXmlNS::fo, "border", "left")) {
        QString border = styleStack.property(KoXmlNS::fo, "border", "left");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KoXmlNS::calligra, "specialborder", "left")) {
            style = styleStack.property(KoXmlNS::calligra, "specialborder", "left");
        }
        if (!border.isEmpty() && border != "none" && border != "hidden") {
            setEdge(KoTableBorderStyle::Left, KoBorder::odfBorderStyle(style), KoUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "border", "top")) {
        QString border = styleStack.property(KoXmlNS::fo, "border", "top");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KoXmlNS::calligra, "specialborder", "top")) {
            style = styleStack.property(KoXmlNS::calligra, "specialborder", "top");
        }
        if (!border.isEmpty() && border != "none" && border != "hidden") {
            setEdge(KoTableBorderStyle::Top, KoBorder::odfBorderStyle(style), KoUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
        }
    }

    if (styleStack.hasProperty(KoXmlNS::fo, "border", "right")) {
        QString border = styleStack.property(KoXmlNS::fo, "border", "right");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KoXmlNS::calligra, "specialborder", "right")) {
            style = styleStack.property(KoXmlNS::calligra, "specialborder", "right");
        }
        if (!border.isEmpty() && border != "none" && border != "hidden") {
            setEdge(KoTableBorderStyle::Right, KoBorder::odfBorderStyle(style), KoUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "border", "bottom")) {
        QString border = styleStack.property(KoXmlNS::fo, "border", "bottom");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KoXmlNS::calligra, "specialborder", "bottom")) {
            style = styleStack.property(KoXmlNS::calligra, "specialborder", "bottom");
        }
        if (!border.isEmpty() && border != "none" && border != "hidden") {
            setEdge(KoTableBorderStyle::Bottom, KoBorder::odfBorderStyle(style), KoUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::style, "diagonal-tl-br")) {
        QString border = styleStack.property(KoXmlNS::style, "diagonal-tl-br");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KoXmlNS::calligra, "specialborder", "tl-br")) {
            style = styleStack.property(KoXmlNS::calligra, "specialborder", "tl-br");
        }
        setEdge(KoTableBorderStyle::TopLeftToBottomRight, KoBorder::odfBorderStyle(style), KoUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
    }
    if (styleStack.hasProperty(KoXmlNS::style, "diagonal-bl-tr")) {
        QString border = styleStack.property(KoXmlNS::style, "diagonal-bl-tr");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KoXmlNS::calligra, "specialborder", "bl-tr")) {
            style = styleStack.property(KoXmlNS::calligra, "specialborder", "bl-tr");
        }
        setEdge(KoTableBorderStyle::BottomLeftToTopRight, KoBorder::odfBorderStyle(style), KoUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
    }

    if (styleStack.hasProperty(KoXmlNS::style, "border-line-width", "left")) {
        QString borderLineWidth = styleStack.property(KoXmlNS::style, "border-line-width", "left");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setEdgeDoubleBorderValues(KoTableBorderStyle::Left, KoUnit::parseValue(blw[0], 1.0), KoUnit::parseValue(blw[1], 0.1));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::style, "border-line-width", "top")) {
        QString borderLineWidth = styleStack.property(KoXmlNS::style, "border-line-width", "top");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setEdgeDoubleBorderValues(KoTableBorderStyle::Top, KoUnit::parseValue(blw[0], 1.0), KoUnit::parseValue(blw[1], 0.1));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::style, "border-line-width", "right")) {
        QString borderLineWidth = styleStack.property(KoXmlNS::style, "border-line-width", "right");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setEdgeDoubleBorderValues(KoTableBorderStyle::Right, KoUnit::parseValue(blw[0], 1.0), KoUnit::parseValue(blw[1], 0.1));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::style, "border-line-width", "bottom")) {
        QString borderLineWidth = styleStack.property(KoXmlNS::style, "border-line-width", "bottom");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setEdgeDoubleBorderValues(KoTableBorderStyle::Bottom, KoUnit::parseValue(blw[0], 1.0), KoUnit::parseValue(blw[1], 0.1));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::style, "diagonal-tl-br-widths")) {
        QString borderLineWidth = styleStack.property(KoXmlNS::style, "diagonal-tl-br-widths");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setEdgeDoubleBorderValues(KoTableBorderStyle::TopLeftToBottomRight, KoUnit::parseValue(blw[0], 1.0), KoUnit::parseValue(blw[1], 0.1));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::style, "diagonal-bl-tr-widths")) {
        QString borderLineWidth = styleStack.property(KoXmlNS::style, "diagonal-bl-tr-widths");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setEdgeDoubleBorderValues(KoTableBorderStyle::BottomLeftToTopRight, KoUnit::parseValue(blw[0], 1.0), KoUnit::parseValue(blw[1], 0.1));
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

    QString fillStyle = styleStack.property(KoXmlNS::draw, "fill");
    if (fillStyle == "solid" || fillStyle == "hatch") {
        styleStack.save();
        QBrush brush = KoOdfGraphicStyles::loadOdfFillStyle(styleStack, fillStyle, context.odfLoadingContext().stylesReader());
        setBackground(brush);
        styleStack.restore();
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
        } else if (key == Borders) {
            borders().saveOdf(style, KoGenStyle::TableCellType);
        } else if (key == Shadow) {
            style.addProperty("style:shadow", shadow().saveOdf());
        }
    }
    if (d->charStyle) {
        d->charStyle->saveOdf(style);
    }

}

void KoTableCellStyle::setEdge(KoTableBorderStyle::Side side, KoBorder::BorderStyle style, qreal width, QColor color)
{
    KoTableBorderStyle::Edge edge;
    qreal innerWidth = 0;
    qreal middleWidth = 0;
    qreal space = 0;
    switch (style) {
    case KoBorder::BorderNone:
        width = 0.0;
        break;
    case KoBorder::BorderDouble:
        innerWidth = space = width/4; //some nice default look
        width -= (space + innerWidth);
        edge.outerPen.setStyle(Qt::SolidLine);
        break;
    case KoBorder::BorderDotted:
        edge.outerPen.setStyle(Qt::DotLine);
        break;
    case KoBorder::BorderDashed:
        edge.outerPen.setStyle(Qt::DashLine);
        break;
    case KoBorder::BorderDashedLong: {
        QVector<qreal> dashes;
        dashes << 6 << 6;
        edge.outerPen.setDashPattern(dashes);
        break;
    }
    case KoBorder::BorderTriple:
        innerWidth = middleWidth = space = width/6;
        width -= (space + innerWidth);
        edge.outerPen.setStyle(Qt::SolidLine);
        break;
    case KoBorder::BorderDashDot:
        edge.outerPen.setStyle(Qt::DashDotLine);
        break;
    case KoBorder::BorderDashDotDot:
        edge.outerPen.setStyle(Qt::DashDotDotLine);
        break;
    case KoBorder::BorderWave:
        edge.outerPen.setStyle(Qt::SolidLine);
        break;
    case KoBorder::BorderSlash:
        edge.outerPen.setStyle(Qt::SolidLine);
        break;
    case KoBorder::BorderDoubleWave:
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

    setEdge(side, edge, style);
}

void KoTableCellStyle::setEdge(KoTableBorderStyle::Side side, const KoTableBorderStyle::Edge &edge, KoBorder::BorderStyle style)
{
    switch (side)
    {
        case KoTableBorderStyle::Top:
            setProperty(KoTableBorderStyle::TopBorderOuterPen, edge.outerPen);
            setProperty(KoTableBorderStyle::TopBorderSpacing, edge.spacing);
            setProperty(KoTableBorderStyle::TopBorderInnerPen, edge.innerPen);
            setProperty(KoTableBorderStyle::TopBorderStyle, style);
            break;
        case KoTableBorderStyle::Left:
            setProperty(KoTableBorderStyle::LeftBorderOuterPen, edge.outerPen);
            setProperty(KoTableBorderStyle::LeftBorderSpacing, edge.spacing);
            setProperty(KoTableBorderStyle::LeftBorderInnerPen, edge.innerPen);
            setProperty(KoTableBorderStyle::LeftBorderStyle, style);
            break;
        case KoTableBorderStyle::Bottom:
            setProperty(KoTableBorderStyle::BottomBorderOuterPen, edge.outerPen);
            setProperty(KoTableBorderStyle::BottomBorderSpacing, edge.spacing);
            setProperty(KoTableBorderStyle::BottomBorderInnerPen, edge.innerPen);
            setProperty(KoTableBorderStyle::BottomBorderStyle, style);
            break;
        case KoTableBorderStyle::Right:
            setProperty(KoTableBorderStyle::RightBorderOuterPen, edge.outerPen);
            setProperty(KoTableBorderStyle::RightBorderSpacing, edge.spacing);
            setProperty(KoTableBorderStyle::RightBorderInnerPen, edge.innerPen);
            setProperty(KoTableBorderStyle::RightBorderStyle, style);
            break;
        case KoTableBorderStyle::TopLeftToBottomRight:
            setProperty(KoTableBorderStyle::TopLeftToBottomRightBorderOuterPen, edge.outerPen);
            setProperty(KoTableBorderStyle::TopLeftToBottomRightBorderSpacing, edge.spacing);
            setProperty(KoTableBorderStyle::TopLeftToBottomRightBorderInnerPen, edge.innerPen);
            setProperty(KoTableBorderStyle::TopLeftToBottomRightBorderStyle, style);
            break;
        case KoTableBorderStyle::BottomLeftToTopRight:
            setProperty(KoTableBorderStyle::BottomLeftToTopRightBorderOuterPen, edge.outerPen);
            setProperty(KoTableBorderStyle::BottomLeftToTopRightBorderSpacing, edge.spacing);
            setProperty(KoTableBorderStyle::BottomLeftToTopRightBorderInnerPen, edge.innerPen);
            setProperty(KoTableBorderStyle::BottomLeftToTopRightBorderStyle, style);
            break;
    }
}

void KoTableCellStyle::setEdgeDoubleBorderValues(KoTableBorderStyle::Side side, qreal innerWidth, qreal space)
{
    KoTableBorderStyle::Edge edge = getEdge(side);

    qreal totalWidth = edge.outerPen.widthF() + edge.spacing + edge.innerPen.widthF();
    if (edge.innerPen.widthF() > 0.0) {
        edge.outerPen.setWidthF(totalWidth - innerWidth - space);
        edge.spacing = space;
        edge.innerPen.setWidthF(innerWidth);
        setEdge(side, edge, getBorderStyle(side));
    }
}

bool KoTableCellStyle::hasBorders() const
{
    for (int i = KoTableBorderStyle::Top; i <= KoTableBorderStyle::BottomLeftToTopRight; i++)
        if (getEdge(KoTableBorderStyle::Side(i)).outerPen.widthF() > 0.0)
            return true;
    return false;
}

qreal KoTableCellStyle::leftBorderWidth() const
{
    const KoTableBorderStyle::Edge &edge = getEdge(KoTableBorderStyle::Left);
    return edge.spacing + edge.innerPen.widthF() + edge.outerPen.widthF();
}

qreal KoTableCellStyle::rightBorderWidth() const
{
    const KoTableBorderStyle::Edge &edge = getEdge(KoTableBorderStyle::Right);
    return edge.spacing + edge.innerPen.widthF() + edge.outerPen.widthF();
}

qreal KoTableCellStyle::topBorderWidth() const
{
    const KoTableBorderStyle::Edge &edge = getEdge(KoTableBorderStyle::Top);
    return edge.spacing + edge.innerPen.widthF() + edge.outerPen.widthF();
}

qreal KoTableCellStyle::bottomBorderWidth() const
{
    const KoTableBorderStyle::Edge &edge = getEdge(KoTableBorderStyle::Bottom);
    return edge.spacing + edge.innerPen.widthF() + edge.outerPen.widthF();
}

qreal KoTableCellStyle::leftInnerBorderWidth() const
{
    return propertyPen(KoTableBorderStyle::LeftBorderInnerPen).widthF();
}

qreal KoTableCellStyle::rightInnerBorderWidth() const
{
    return propertyPen(KoTableBorderStyle::RightBorderInnerPen).widthF();
}

qreal KoTableCellStyle::topInnerBorderWidth() const
{
    return propertyPen(KoTableBorderStyle::TopBorderInnerPen).widthF();
}

qreal KoTableCellStyle::bottomInnerBorderWidth() const
{
    return propertyPen(KoTableBorderStyle::BottomBorderInnerPen).widthF();
}

qreal KoTableCellStyle::leftOuterBorderWidth() const
{
    return propertyPen(KoTableBorderStyle::LeftBorderOuterPen).widthF();
}

qreal KoTableCellStyle::rightOuterBorderWidth() const
{
    return propertyPen(KoTableBorderStyle::RightBorderOuterPen).widthF();
}

qreal KoTableCellStyle::topOuterBorderWidth() const
{
    return propertyPen(KoTableBorderStyle::TopBorderOuterPen).widthF();
}

qreal KoTableCellStyle::bottomOuterBorderWidth() const
{
    return propertyPen(KoTableBorderStyle::BottomBorderOuterPen).widthF();
}

KoTableBorderStyle::Edge KoTableCellStyle::getEdge(KoTableBorderStyle::Side side) const
{
    KoTableBorderStyle::Edge result;
    switch (side)
    {
        case KoTableBorderStyle::Top:
            result.outerPen = propertyPen(KoTableBorderStyle::TopBorderOuterPen);
            result.spacing = propertyDouble(KoTableBorderStyle::TopBorderSpacing);
            result.innerPen = propertyPen(KoTableBorderStyle::TopBorderInnerPen);
            break;
        case KoTableBorderStyle::Left:
            result.outerPen = propertyPen(KoTableBorderStyle::LeftBorderOuterPen);
            result.spacing = propertyDouble(KoTableBorderStyle::LeftBorderSpacing);
            result.innerPen = propertyPen(KoTableBorderStyle::LeftBorderInnerPen);
            break;
        case KoTableBorderStyle::Bottom:
            result.outerPen = propertyPen(KoTableBorderStyle::BottomBorderOuterPen);
            result.spacing = propertyDouble(KoTableBorderStyle::BottomBorderSpacing);
            result.innerPen = propertyPen(KoTableBorderStyle::BottomBorderInnerPen);
            break;
        case KoTableBorderStyle::Right:
            result.outerPen = propertyPen(KoTableBorderStyle::RightBorderOuterPen);
            result.spacing = propertyDouble(KoTableBorderStyle::RightBorderSpacing);
            result.innerPen = propertyPen(KoTableBorderStyle::RightBorderInnerPen);
            break;
        case KoTableBorderStyle::TopLeftToBottomRight:
            result.outerPen = propertyPen(KoTableBorderStyle::TopLeftToBottomRightBorderOuterPen);
            result.spacing = propertyDouble(KoTableBorderStyle::TopLeftToBottomRightBorderSpacing);
            result.innerPen = propertyPen(KoTableBorderStyle::TopLeftToBottomRightBorderInnerPen);
            break;
        case KoTableBorderStyle::BottomLeftToTopRight:
            result.outerPen = propertyPen(KoTableBorderStyle::BottomLeftToTopRightBorderOuterPen);
            result.spacing = propertyDouble(KoTableBorderStyle::BottomLeftToTopRightBorderSpacing);
            result.innerPen = propertyPen(KoTableBorderStyle::BottomLeftToTopRightBorderInnerPen);
            break;
    }
    return result;
}

KoBorder::BorderStyle KoTableCellStyle::getBorderStyle(KoTableBorderStyle::Side side) const
{
    switch (side)
    {
        case KoTableBorderStyle::Top:
            return KoBorder::BorderStyle(propertyInt(KoTableBorderStyle::TopBorderStyle));
        case KoTableBorderStyle::Left:
            return KoBorder::BorderStyle(propertyInt(KoTableBorderStyle::LeftBorderStyle));
        case KoTableBorderStyle::Bottom:
            return KoBorder::BorderStyle(propertyInt(KoTableBorderStyle::BottomBorderStyle));
        case KoTableBorderStyle::Right:
            return KoBorder::BorderStyle(propertyInt(KoTableBorderStyle::RightBorderStyle));
        case KoTableBorderStyle::TopLeftToBottomRight:
            return KoBorder::BorderStyle(propertyInt(KoTableBorderStyle::TopLeftToBottomRightBorderStyle));
        case KoTableBorderStyle::BottomLeftToTopRight:
            return KoBorder::BorderStyle(propertyInt(KoTableBorderStyle::BottomLeftToTopRightBorderStyle));
    }
    return KoBorder::BorderNone;
}

#include <KoTableCellStyle.moc>
