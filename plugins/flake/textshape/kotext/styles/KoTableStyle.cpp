/* This file is part of the KDE project
 * Copyright (C) 2006-2009 Thomas Zander <zander@kde.org>
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
#include "KoTableStyle.h"

#include <KoGenStyle.h>
#include "Styles_p.h"

#include <QTextTable>
#include <QTextTableFormat>

#include <KoShadowStyle.h>
#include <KoUnit.h>
#include <KoStyleStack.h>
#include <KoOdfLoadingContext.h>
#include <KoXmlNS.h>
#include <KoXmlReader.h>

#include "TextDebug.h"

class Q_DECL_HIDDEN KoTableStyle::Private
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

QTextLength KoTableStyle::propertyLength(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return QTextLength(QTextLength::FixedLength, 0.0);
    if (!variant.canConvert<QTextLength>())
    {
        // Fake support, for compatibility sake
        if (variant.canConvert<qreal>())
        {
            return QTextLength(QTextLength::FixedLength, variant.toReal());
        }
        
        warnText << "This should never happen : requested property can't be converted to QTextLength";
        return QTextLength(QTextLength::FixedLength, 0.0);
    }
    return variant.value<QTextLength>();
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
        int key = keys[i];
        switch(key) {
            // Qt expects qreal's for the Frame*Margin's unlike the Block*Margin's
            case QTextFormat::FrameTopMargin:
            case QTextFormat::FrameBottomMargin:
            case QTextFormat::FrameLeftMargin:
            case QTextFormat::FrameRightMargin:
                variant = propertyLength(key).rawValue();
                break;
            default:
                break;
        }
        format.setProperty(key, variant);
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

bool KoTableStyle::keepWithNext() const
{
    return propertyBoolean(KeepWithNext);
}

void KoTableStyle::setShadow(const KoShadowStyle &shadow)
{
    d->setProperty(Shadow, QVariant::fromValue<KoShadowStyle>(shadow));
}

KoShadowStyle KoTableStyle::shadow() const
{
    if (hasProperty(Shadow))
        return value(Shadow).value<KoShadowStyle>();
    return KoShadowStyle();
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

void KoTableStyle::setBreakBefore(KoText::KoTextBreakProperty state)
{
    setProperty(BreakBefore, state);
}

KoText::KoTextBreakProperty KoTableStyle::breakBefore() const
{
    return (KoText::KoTextBreakProperty) propertyInt(BreakBefore);
}

void KoTableStyle::setBreakAfter(KoText::KoTextBreakProperty state)
{
    setProperty(BreakAfter, state);
}

KoText::KoTextBreakProperty KoTableStyle::breakAfter() const
{
    return (KoText::KoTextBreakProperty) propertyInt(BreakAfter);
}

void KoTableStyle::setCollapsingBorderModel(bool on)
{
    setProperty(CollapsingBorders, on);
}

bool KoTableStyle::collapsingBorderModel() const
{
    return propertyBoolean(CollapsingBorders);
}

void KoTableStyle::setTopMargin(QTextLength topMargin)
{
    setProperty(QTextFormat::FrameTopMargin, topMargin);
}

qreal KoTableStyle::topMargin() const
{
    if (parentStyle())
        return propertyLength(QTextFormat::FrameTopMargin).value(parentStyle()->topMargin());
    else
        return propertyLength(QTextFormat::FrameTopMargin).value(0);
}

void KoTableStyle::setBottomMargin(QTextLength margin)
{
    setProperty(QTextFormat::FrameBottomMargin, margin);
}

qreal KoTableStyle::bottomMargin() const
{
    if (parentStyle())
        return propertyLength(QTextFormat::FrameBottomMargin).value(parentStyle()->bottomMargin());
    else
        return propertyLength(QTextFormat::FrameBottomMargin).value(0);
}

void KoTableStyle::setLeftMargin(QTextLength margin)
{
    setProperty(QTextFormat::FrameLeftMargin, margin);
}

qreal KoTableStyle::leftMargin() const
{
    if (parentStyle())
        return propertyLength(QTextFormat::FrameLeftMargin).value(parentStyle()->leftMargin());
    else
        return propertyLength(QTextFormat::FrameLeftMargin).value(0);
}

void KoTableStyle::setRightMargin(QTextLength margin)
{
    setProperty(QTextFormat::FrameRightMargin, margin);
}

qreal KoTableStyle::rightMargin() const
{
    if (parentStyle())
        return propertyLength(QTextFormat::FrameRightMargin).value(parentStyle()->rightMargin());
    else
        return propertyLength(QTextFormat::FrameRightMargin).value(0);
}

void KoTableStyle::setMargin(QTextLength margin)
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
    QString align;
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

bool KoTableStyle::mayBreakBetweenRows() const
{
    return propertyBoolean(MayBreakBetweenRows);
}

void KoTableStyle::setPageNumber(int page)
{
    if (page >= 0)
        setProperty(PageNumber, page);
}

int KoTableStyle::pageNumber() const
{
    return propertyInt(PageNumber);
}

bool KoTableStyle::visible() const
{
    if (hasProperty(Visible))
        return propertyBoolean(Visible);
    return true;
}

void KoTableStyle::setVisible(bool on)
{
    setProperty(Visible, on);
}

KoText::Direction KoTableStyle::textDirection() const
{
    return (KoText::Direction) propertyInt(TextProgressionDirection);
}

void KoTableStyle::setTextDirection(KoText::Direction direction)
{
    setProperty(TextProgressionDirection, direction);
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

void KoTableStyle::loadOdfProperties(KoStyleStack &styleStack)
{
    if (styleStack.hasProperty(KoXmlNS::style, "writing-mode")) {     // http://www.w3.org/TR/2004/WD-xsl11-20041216/#writing-mode
        setTextDirection(KoText::directionFromString(styleStack.property(KoXmlNS::style, "writing-mode")));
    }

    if (styleStack.hasProperty(KoXmlNS::table, "display")) {
        setVisible(styleStack.property(KoXmlNS::table, "display") == "true");
    }
    
    // Width
    if (styleStack.hasProperty(KoXmlNS::style, "width")) {
        setWidth(QTextLength(QTextLength::FixedLength, KoUnit::parseValue(styleStack.property(KoXmlNS::style, "width"))));
    }
    if (styleStack.hasProperty(KoXmlNS::style, "rel-width")) {
        setWidth(QTextLength(QTextLength::PercentageLength, styleStack.property(KoXmlNS::style, "rel-width").remove('%').remove('*').toDouble()));
    }

    // Alignment
    if (styleStack.hasProperty(KoXmlNS::table, "align")) {
        setAlignment(alignmentFromString(styleStack.property(KoXmlNS::table, "align")));
    }

    // Margin
    bool hasMarginLeft = styleStack.hasProperty(KoXmlNS::fo, "margin-left");
    bool hasMarginRight = styleStack.hasProperty(KoXmlNS::fo, "margin-right");
    if (hasMarginLeft)
        setLeftMargin(KoText::parseLength(styleStack.property(KoXmlNS::fo, "margin-left")));
    if (hasMarginRight)
        setRightMargin(KoText::parseLength(styleStack.property(KoXmlNS::fo, "margin-right")));
    if (styleStack.hasProperty(KoXmlNS::fo, "margin-top"))
        setTopMargin(KoText::parseLength(styleStack.property(KoXmlNS::fo, "margin-top")));
    if (styleStack.hasProperty(KoXmlNS::fo, "margin-bottom"))
        setBottomMargin(KoText::parseLength(styleStack.property(KoXmlNS::fo, "margin-bottom")));
    if (styleStack.hasProperty(KoXmlNS::fo, "margin")) {
        setMargin(KoText::parseLength(styleStack.property(KoXmlNS::fo, "margin")));
    }

    // keep table with next paragraph? 
    if (styleStack.hasProperty(KoXmlNS::fo, "keep-with-next")) {
        // OASIS spec says it's "auto"/"always", not a boolean.
        QString val = styleStack.property(KoXmlNS::fo, "keep-with-next");
        setKeepWithNext(val == "true" || val == "always");
    }

    // The fo:break-before and fo:break-after attributes insert a page or column break before or after a table.
    if (styleStack.hasProperty(KoXmlNS::fo, "break-before")) {
        setBreakBefore(KoText::textBreakFromString(styleStack.property(KoXmlNS::fo, "break-before")));
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "break-after")) {
        setBreakAfter(KoText::textBreakFromString(styleStack.property(KoXmlNS::fo, "break-after")));
    }

    if (styleStack.hasProperty(KoXmlNS::style, "may-break-between-rows")) {
        setMayBreakBetweenRows(styleStack.property(KoXmlNS::style, "may-break-between-rows") == "true");
    }

    if (styleStack.hasProperty(KoXmlNS::style, "page-number")) {
        setPageNumber(styleStack.property(KoXmlNS::style, "page-number").toInt());
    }

    if (styleStack.hasProperty(KoXmlNS::style, "shadow")) {
        KoShadowStyle shadow;
        if (shadow.loadOdf(styleStack.property(KoXmlNS::style, "shadow")))
            setShadow(shadow);
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
    if (styleStack.hasProperty(KoXmlNS::table, "border-model")) {
        QString val = styleStack.property(KoXmlNS::table, "border-model");
        setCollapsingBorderModel(val == "collapsing");
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

bool KoTableStyle::isEmpty() const
{
    return d->stylesPrivate.isEmpty();
}

void KoTableStyle::saveOdf(KoGenStyle &style)
{
    QList<int> keys = d->stylesPrivate.keys();
    if ((hasProperty(QTextFormat::FrameLeftMargin)) && 
        (hasProperty(QTextFormat::FrameRightMargin)) && 
        (hasProperty(QTextFormat::FrameTopMargin)) && 
        (hasProperty(QTextFormat::FrameBottomMargin)) && 
        (rightMargin() == leftMargin()) && (leftMargin() == topMargin()) && (topMargin() == bottomMargin()))
    {
        style.addPropertyLength("fo:margin", propertyLength(QTextFormat::FrameBottomMargin), KoGenStyle::TableType);
        keys.removeAll(QTextFormat::FrameBottomMargin);
        keys.removeAll(QTextFormat::FrameTopMargin);
        keys.removeAll(QTextFormat::FrameRightMargin);
        keys.removeAll(QTextFormat::FrameLeftMargin);
    }
    Q_FOREACH (int key, keys) {
        if (key == QTextFormat::FrameWidth) {
            QTextLength width = propertyLength(QTextFormat::FrameWidth);
            if (width.type() == QTextLength::PercentageLength) {
                style.addPropertyLength("style:rel-width", width, KoGenStyle::TableType);
            } else if (width.type() == QTextLength::FixedLength) {
                style.addPropertyLength("style:width", width, KoGenStyle::TableType);
            }
        } else if (key == QTextFormat::BlockAlignment) {
            bool ok = false;
            int alignValue = value(QTextFormat::BlockAlignment).toInt(&ok);
            if (ok) {
                QString alignment = alignmentToString((Qt::Alignment) alignValue);
                if (!alignment.isEmpty())
                    style.addProperty("table:align", alignment, KoGenStyle::TableType);
            }
        } else if (key == KoTableStyle::BreakBefore) {
            style.addProperty("fo:break-before", KoText::textBreakToString(breakBefore()), KoGenStyle::TableType);
        } else if (key == KoTableStyle::BreakAfter) {
            style.addProperty("fo:break-after", KoText::textBreakToString(breakAfter()), KoGenStyle::TableType);
        } else if (key == KoTableStyle::MayBreakBetweenRows) {
            style.addProperty("style:may-break-between-rows", mayBreakBetweenRows(), KoGenStyle::TableType);
        } else if (key == QTextFormat::BackgroundBrush) {
            QBrush backBrush = background();
            if (backBrush.style() != Qt::NoBrush)
                style.addProperty("fo:background-color", backBrush.color().name(), KoGenStyle::TableType);
            else
                style.addProperty("fo:background-color", "transparent", KoGenStyle::TableType);
        } else if (key == QTextFormat::FrameLeftMargin) {
            style.addPropertyLength("fo:margin-left", propertyLength(QTextFormat::FrameLeftMargin), KoGenStyle::TableType);
        } else if (key == QTextFormat::FrameRightMargin) {
            style.addPropertyLength("fo:margin-right", propertyLength(QTextFormat::FrameRightMargin), KoGenStyle::TableType);
        } else if (key == QTextFormat::FrameTopMargin) {
            style.addPropertyLength("fo:margin-top", propertyLength(QTextFormat::FrameTopMargin), KoGenStyle::TableType);
        } else if (key == QTextFormat::FrameBottomMargin) {
            style.addPropertyLength("fo:margin-bottom", propertyLength(QTextFormat::FrameBottomMargin), KoGenStyle::TableType);
        } else if (key == KoTableStyle::CollapsingBorders) {
            if (collapsingBorderModel())
                style.addProperty("table:border-model", "collapsing", KoGenStyle::TableType);
            else
                style.addProperty("table:border-model", "separating", KoGenStyle::TableType);
        } else if (key == KoTableStyle::KeepWithNext) {
            if (keepWithNext())
                style.addProperty("fo:keep-with-next", "always", KoGenStyle::TableType);
            else
                style.addProperty("fo:keep-with-next", "auto", KoGenStyle::TableType);
        } else if (key == KoTableStyle::Visible) {
            style.addProperty("table:display", visible(), KoGenStyle::TableType);
        } else if (key == KoTableStyle::PageNumber) {
            if (pageNumber() > 0)
                style.addProperty("style:page-number", pageNumber(), KoGenStyle::TableType);
            else
                style.addProperty("style:page-number", "auto", KoGenStyle::TableType);
        } else if (key == TextProgressionDirection) {
            style.addProperty("style:writing-mode", KoText::directionToString(textDirection()), KoGenStyle::TableType);
        } else if (key == KoTableStyle::Shadow) {
            style.addProperty("style:shadow", shadow().saveOdf());
        }
    }
}
