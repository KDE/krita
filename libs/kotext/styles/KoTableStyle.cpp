/* This file is part of the KDE project
 * Copyright (C) 2006-2009 Thomas Zander <zander@kde.org>
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
#include "KoTableStyle.h"
#include "KoStyleManager.h"
#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include "Styles_p.h"
#include "KoTextDocument.h"

#include <KDebug>

#include <QTextTable>
#include <QTextTableFormat>

#include <KoUnit.h>
#include <KoStyleStack.h>
#include <KoOdfLoadingContext.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>

class KoTableStyle::Private
{
public:
    Private() : parentStyle(0), next(0) {}

    ~Private() {
    }

    void setProperty(int key, const QVariant &value) {
        stylesPrivate.add(key, value);
    }

    QString name;
    KoTableStyle *parentStyle;
    int next;
    StylePrivate stylesPrivate;
};

KoTableStyle::KoTableStyle(QObject *parent)
        : QObject(parent), d(new Private())
{
}

KoTableStyle::KoTableStyle(const QTextTableFormat &tableFormat, QObject *parent)
        : QObject(parent),
        d(new Private())
{
    d->stylesPrivate = tableFormat.properties();
}

KoTableStyle *KoTableStyle::fromTable(const QTextTable &table, QObject *parent)
{
    QTextTableFormat tableFormat = table.format();
    return new KoTableStyle(tableFormat, parent);
}

KoTableStyle::~KoTableStyle()
{
    delete d;
}

void KoTableStyle::setParentStyle(KoTableStyle *parent)
{
    d->parentStyle = parent;
}

void KoTableStyle::setProperty(int key, const QVariant &value)
{
    if (d->parentStyle) {
        QVariant var = d->parentStyle->value(key);
        if (!var.isNull() && var == value) { // same as parent, so its actually a reset.
            d->stylesPrivate.remove(key);
            return;
        }
    }
    d->stylesPrivate.add(key, value);
}

void KoTableStyle::remove(int key)
{
    d->stylesPrivate.remove(key);
}

QVariant KoTableStyle::value(int key) const
{
    QVariant var = d->stylesPrivate.value(key);
    if (var.isNull() && d->parentStyle)
        return d->parentStyle->value(key);
    return var;
}

bool KoTableStyle::hasProperty(int key) const
{
    return d->stylesPrivate.contains(key);
}

qreal KoTableStyle::propertyDouble(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return 0.0;
    return variant.toDouble();
}

int KoTableStyle::propertyInt(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return 0;
    return variant.toInt();
}

bool KoTableStyle::propertyBoolean(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return false;
    return variant.toBool();
}

QColor KoTableStyle::propertyColor(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull()) {
        return QColor();
    }
    return qvariant_cast<QColor>(variant);
}

void KoTableStyle::applyStyle(QTextTableFormat &format) const
{/*
    if (d->parentStyle) {
        d->parentStyle->applyStyle(format);
    }*/
    QList<int> keys = d->stylesPrivate.keys();
    for (int i = 0; i < keys.count(); i++) {
        QVariant variant = d->stylesPrivate.value(keys[i]);
        format.setProperty(keys[i], variant);
    }
}

void KoTableStyle::setWidth(const QTextLength &width)
{
    d->setProperty(QTextFormat::FrameWidth, width);
}

void KoTableStyle::setKeepWithNext(bool keep)
{
    d->setProperty(KeepWithNext, keep);
}

void KoTableStyle::setMayBreakBetweenRows(bool allow)
{
    d->setProperty(MayBreakBetweenRows, allow);
}

void KoTableStyle::setBackground(const QBrush &brush)
{
    d->setProperty(QTextFormat::BackgroundBrush, brush);
}

void KoTableStyle::clearBackground()
{
    d->stylesPrivate.remove(QTextCharFormat::BackgroundBrush);
}

QBrush KoTableStyle::background() const
{
    QVariant variant = d->stylesPrivate.value(QTextFormat::BackgroundBrush);

    if (variant.isNull()) {
        return QBrush();
    }
    return qvariant_cast<QBrush>(variant);
}

void KoTableStyle::setBreakBefore(bool on)
{
    setProperty(BreakBefore, on);
}

bool KoTableStyle::breakBefore()
{
    return propertyBoolean(BreakBefore);
}

void KoTableStyle::setBreakAfter(bool on)
{
    setProperty(BreakAfter, on);
}

bool KoTableStyle::breakAfter()
{
    return propertyBoolean(BreakAfter);
}

void KoTableStyle::setCollapsingBorderModel(bool on)
{
    setProperty(CollapsingBorders, on);
}

bool KoTableStyle::collapsingBorderModel()
{
    return propertyBoolean(CollapsingBorders);
}

void KoTableStyle::setTopMargin(qreal topMargin)
{
    setProperty(QTextFormat::FrameTopMargin, topMargin);
}

qreal KoTableStyle::topMargin() const
{
    return propertyDouble(QTextFormat::FrameTopMargin);
}

void KoTableStyle::setBottomMargin(qreal margin)
{
    setProperty(QTextFormat::FrameBottomMargin, margin);
}

qreal KoTableStyle::bottomMargin() const
{
    return propertyDouble(QTextFormat::FrameBottomMargin);
}

void KoTableStyle::setLeftMargin(qreal margin)
{
    setProperty(QTextFormat::FrameLeftMargin, margin);
}

qreal KoTableStyle::leftMargin() const
{
    return propertyDouble(QTextFormat::FrameLeftMargin);
}

void KoTableStyle::setRightMargin(qreal margin)
{
    setProperty(QTextFormat::FrameRightMargin, margin);
}

qreal KoTableStyle::rightMargin() const
{
    return propertyDouble(QTextFormat::FrameRightMargin);
}

void KoTableStyle::setMargin(qreal margin)
{
    setTopMargin(margin);
    setBottomMargin(margin);
    setLeftMargin(margin);
    setRightMargin(margin);
}

void KoTableStyle::setAlignment(Qt::Alignment alignment)
{

    setProperty(QTextFormat::BlockAlignment, (int) alignment);

}

Qt::Alignment KoTableStyle::alignment() const
{
    return static_cast<Qt::Alignment>(propertyInt(QTextFormat::BlockAlignment));
}

KoTableStyle *KoTableStyle::parentStyle() const
{
    return d->parentStyle;
}

QString KoTableStyle::name() const
{
    return d->name;
}

void KoTableStyle::setName(const QString &name)
{
    if (name == d->name)
        return;
    d->name = name;
    emit nameChanged(name);
}

int KoTableStyle::styleId() const
{
    return propertyInt(StyleId);
}

void KoTableStyle::setStyleId(int id)
{
    setProperty(StyleId, id); if (d->next == 0) d->next = id;
}

QString KoTableStyle::masterPageName() const
{
    return value(MasterPageName).toString();
}

void KoTableStyle::setMasterPageName(const QString &name)
{
    setProperty(MasterPageName, name);
}

void KoTableStyle::loadOdf(const KoXmlElement *element, KoOdfLoadingContext &context)
{
    if (element->hasAttributeNS(KoXmlNS::style, "display-name"))
        d->name = element->attributeNS(KoXmlNS::style, "display-name", QString());

    if (d->name.isEmpty()) // if no style:display-name is given us the style:name
        d->name = element->attributeNS(KoXmlNS::style, "name", QString());

    QString masterPage = element->attributeNS(KoXmlNS::style, "master-page-name", QString());
    if (! masterPage.isEmpty()) {
        setMasterPageName(masterPage);
    }
    context.styleStack().save();
    QString family = element->attributeNS(KoXmlNS::style, "family", "table");
    context.addStyles(element, family.toLocal8Bit().constData());   // Load all parents - only because we don't support inheritance.

    context.styleStack().setTypeProperties("table");   // load all style attributes from "style:table-properties"
    loadOdfProperties(context.styleStack());   // load the KoTableStyle from the stylestack
    context.styleStack().restore();
}

Qt::Alignment KoTableStyle::alignmentFromString(const QString &align)
{
    Qt::Alignment alignment = Qt::AlignLeft;
    if (align == "left")
        alignment = Qt::AlignLeft;
    else if (align == "right")
        alignment = Qt::AlignRight;
    else if (align == "center")
        alignment = Qt::AlignHCenter;
    else if (align == "margins") // in tables this is effectively the same as justify
        alignment = Qt::AlignJustify;
    return alignment;
}

QString KoTableStyle::alignmentToString(Qt::Alignment alignment)
{
    QString align = "";
    if (alignment == Qt::AlignLeft)
        align = "left";
    else if (alignment == Qt::AlignRight)
        align = "right";
    else if (alignment == Qt::AlignHCenter)
        align = "center";
    else if (alignment == Qt::AlignJustify)
        align = "margins";
    return align;
}

void KoTableStyle::loadOdfProperties(KoStyleStack &styleStack)
{
    if (styleStack.hasProperty(KoXmlNS::style, "writing-mode")) {     // http://www.w3.org/TR/2004/WD-xsl11-20041216/#writing-mode
        // KoText::directionFromString()
    }

    // Width
    if (styleStack.hasProperty(KoXmlNS::style, "width")) {
        setWidth(QTextLength(QTextLength::FixedLength, KoUnit::parseValue(styleStack.property(KoXmlNS::style, "width"))));
    }
    if (styleStack.hasProperty(KoXmlNS::style, "rel-width")) {
        setWidth(QTextLength(QTextLength::PercentageLength, styleStack.property(KoXmlNS::style, "rel-width").remove('%').toDouble()));
    }

    // Alignment
    if (styleStack.hasProperty(KoXmlNS::table, "align")) {
        setAlignment(alignmentFromString(styleStack.property(KoXmlNS::table, "align")));
    }

    // Margin
    bool hasMarginLeft = styleStack.hasProperty(KoXmlNS::fo, "margin-left");
    bool hasMarginRight = styleStack.hasProperty(KoXmlNS::fo, "margin-right");
    if (hasMarginLeft)
        setLeftMargin(KoUnit::parseValue(styleStack.property(KoXmlNS::fo, "margin-left")));
    if (hasMarginRight)
        setRightMargin(KoUnit::parseValue(styleStack.property(KoXmlNS::fo, "margin-right")));
    if (styleStack.hasProperty(KoXmlNS::fo, "margin-top"))
        setTopMargin(KoUnit::parseValue(styleStack.property(KoXmlNS::fo, "margin-top")));
    if (styleStack.hasProperty(KoXmlNS::fo, "margin-bottom"))
        setBottomMargin(KoUnit::parseValue(styleStack.property(KoXmlNS::fo, "margin-bottom")));
    if (styleStack.hasProperty(KoXmlNS::fo, "margin")) {
        setMargin(KoUnit::parseValue(styleStack.property(KoXmlNS::fo, "margin")));
        hasMarginLeft = true;
        hasMarginRight = true;
    }

    // keep table with next paragraph? 
    if (styleStack.hasProperty(KoXmlNS::fo, "keep-with-next")) {
        // OASIS spec says it's "auto"/"always", not a boolean.
        QString val = styleStack.property(KoXmlNS::fo, "keep-with-next");
        if (val == "true" || val == "always")
            setKeepWithNext(true);
    }

    // The fo:break-before and fo:break-after attributes insert a page or column break before or after a table.
    if (styleStack.hasProperty(KoXmlNS::fo, "break-before")) {
        if (styleStack.property(KoXmlNS::fo, "break-before") != "auto")
            setBreakBefore(true);
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "break-after")) {
        if (styleStack.property(KoXmlNS::fo, "break-after") != "auto")
            setBreakAfter(true);
    }

    if (styleStack.hasProperty(KoXmlNS::fo, "may-break-between-rows")) {
        if (styleStack.property(KoXmlNS::fo, "may-break-between-rows") == "true")
            setMayBreakBetweenRows(true);
    }


    // The fo:background-color attribute specifies the background color of a paragraph.
    if (styleStack.hasProperty(KoXmlNS::fo, "background-color")) {
        const QString bgcolor = styleStack.property(KoXmlNS::fo, "background-color");
        QBrush brush = background();
        if (bgcolor == "transparent")
            brush.setStyle(Qt::NoBrush);
        else {
            if (brush.style() == Qt::NoBrush)
                brush.setStyle(Qt::SolidPattern);
            brush.setColor(bgcolor); // #rrggbb format
        }
        setBackground(brush);
    }
    
    // border-model 
    if (styleStack.hasProperty(KoXmlNS::style, "border-model")) {
        // OASIS spec says it's "auto"/"always", not a boolean.
        QString val = styleStack.property(KoXmlNS::style, "border-model");
        setCollapsingBorderModel(val =="collapsing");
    }
}

void KoTableStyle::copyProperties(const KoTableStyle *style)
{
    d->stylesPrivate = style->d->stylesPrivate;
    setName(style->name()); // make sure we emit property change
    d->next = style->d->next;
    d->parentStyle = style->d->parentStyle;
}

KoTableStyle *KoTableStyle::clone(QObject *parent)
{
    KoTableStyle *newStyle = new KoTableStyle(parent);
    newStyle->copyProperties(this);
    return newStyle;
}


bool KoTableStyle::operator==(const KoTableStyle &other) const
{
    return other.d->stylesPrivate == d->stylesPrivate;
}

void KoTableStyle::removeDuplicates(const KoTableStyle &other)
{
    d->stylesPrivate.removeDuplicates(other.d->stylesPrivate);
}

void KoTableStyle::saveOdf(KoGenStyle &style)
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
        } else if (key == KoTableStyle::TextProgressionDirection) {
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
        } else if (key == KoTableStyle::BreakBefore) {
            if (breakBefore())
                style.addProperty("fo:break-before", "page", KoGenStyle::ParagraphType);
        } else if (key == KoTableStyle::BreakAfter) {
            if (breakAfter())
                style.addProperty("fo:break-after", "page", KoGenStyle::ParagraphType);
        } else if (key == KoTableStyle::CollapsingBorders) {
            if (collapsingBorderModel())
                style.addProperty("style:border-bodel", "collapsing", KoGenStyle::ParagraphType);
        } else if (key == QTextFormat::BackgroundBrush) {
            QBrush backBrush = background();
            if (backBrush.style() != Qt::NoBrush)
                style.addProperty("fo:background-color", backBrush.color().name(), KoGenStyle::ParagraphType);
            else
                style.addProperty("fo:background-color", "transparent", KoGenStyle::ParagraphType);
    // Margin
        } else if (key == QTextFormat::BlockLeftMargin) {
            style.addPropertyPt("fo:margin-left", leftMargin(), KoGenStyle::ParagraphType);
        } else if (key == QTextFormat::BlockRightMargin) {
            style.addPropertyPt("fo:margin-right", rightMargin(), KoGenStyle::ParagraphType);
        } else if (key == QTextFormat::BlockTopMargin) {
            style.addPropertyPt("fo:margin-top", topMargin(), KoGenStyle::ParagraphType);
        } else if (key == QTextFormat::BlockBottomMargin) {
            style.addPropertyPt("fo:margin-bottom", bottomMargin(), KoGenStyle::ParagraphType);
    }
*/
}

#include <KoTableStyle.moc>
