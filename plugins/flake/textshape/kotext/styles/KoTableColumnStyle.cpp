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
#include "KoTableColumnStyle.h"

#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include "Styles_p.h"
#include "KoTextDocument.h"

#include "TextDebug.h"

#include <QTextTable>
#include <QTextTableFormat>

#include <KoUnit.h>
#include <KoStyleStack.h>
#include <KoOdfLoadingContext.h>
#include <KoXmlNS.h>

class Q_DECL_HIDDEN KoTableColumnStyle::Private : public QSharedData
{
public:
    Private() : QSharedData(), parentStyle(0) {}

    ~Private() {
    }

    void setProperty(int key, const QVariant &value) {
        stylesPrivate.add(key, value);
    }

    QString name;
    KoTableColumnStyle *parentStyle;
    StylePrivate stylesPrivate;
};


KoTableColumnStyle::KoTableColumnStyle()
        :  d(new Private())
{
    Q_ASSERT (d);
}

KoTableColumnStyle::KoTableColumnStyle(const KoTableColumnStyle &rhs)
        : d(rhs.d)
{
}

KoTableColumnStyle &KoTableColumnStyle::operator=(const KoTableColumnStyle &rhs)
{
    d = rhs.d;
    return *this;
}

KoTableColumnStyle::~KoTableColumnStyle()
{
}

void KoTableColumnStyle::copyProperties(const KoTableColumnStyle *style)
{
    d->stylesPrivate = style->d->stylesPrivate;
    setName(style->name()); // make sure we emit property change
    d->parentStyle = style->d->parentStyle;
}

KoTableColumnStyle *KoTableColumnStyle::clone() const
{
    KoTableColumnStyle *newStyle = new KoTableColumnStyle();
    newStyle->copyProperties(this);
    return newStyle;
}

void KoTableColumnStyle::setParentStyle(KoTableColumnStyle *parent)
{
    d->parentStyle = parent;
}

void KoTableColumnStyle::setProperty(int key, const QVariant &value)
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

void KoTableColumnStyle::remove(int key)
{
    d->stylesPrivate.remove(key);
}

QVariant KoTableColumnStyle::value(int key) const
{
    QVariant var = d->stylesPrivate.value(key);
    if (var.isNull() && d->parentStyle)
        var = d->parentStyle->value(key);
    return var;
}

bool KoTableColumnStyle::hasProperty(int key) const
{
    return d->stylesPrivate.contains(key);
}

qreal KoTableColumnStyle::propertyDouble(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return 0.0;
    return variant.toDouble();
}

int KoTableColumnStyle::propertyInt(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return 0;
    return variant.toInt();
}

bool KoTableColumnStyle::propertyBoolean(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return false;
    return variant.toBool();
}

QColor KoTableColumnStyle::propertyColor(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull()) {
        QColor color;
        return color;
    }
    return qvariant_cast<QColor>(variant);
}

void KoTableColumnStyle::setColumnWidth(qreal width)
{
    setProperty(ColumnWidth, width);
}

qreal KoTableColumnStyle::columnWidth() const
{
    return propertyDouble(ColumnWidth);
}

void KoTableColumnStyle::setRelativeColumnWidth(qreal width)
{
    setProperty(RelativeColumnWidth, width);
}

qreal KoTableColumnStyle::relativeColumnWidth() const
{
    return propertyDouble(RelativeColumnWidth);
}

void KoTableColumnStyle::setBreakBefore(KoText::KoTextBreakProperty state)
{
    setProperty(BreakBefore, state);
}

KoText::KoTextBreakProperty KoTableColumnStyle::breakBefore() const
{
    return (KoText::KoTextBreakProperty) propertyInt(BreakBefore);
}

void KoTableColumnStyle::setBreakAfter(KoText::KoTextBreakProperty state)
{
    setProperty(BreakAfter, state);
}

KoText::KoTextBreakProperty KoTableColumnStyle::breakAfter() const
{
    return (KoText::KoTextBreakProperty) propertyInt(BreakAfter);
}

KoTableColumnStyle *KoTableColumnStyle::parentStyle() const
{
    return d->parentStyle;
}

QString KoTableColumnStyle::name() const
{
    return d->name;
}

void KoTableColumnStyle::setName(const QString &name)
{
    if (name == d->name)
        return;
    d->name = name;
}

int KoTableColumnStyle::styleId() const
{
    return propertyInt(StyleId);
}

void KoTableColumnStyle::setStyleId(int id)
{
    setProperty(StyleId, id);
}

QString KoTableColumnStyle::masterPageName() const
{
    return value(MasterPageName).toString();
}

void KoTableColumnStyle::setMasterPageName(const QString &name)
{
    setProperty(MasterPageName, name);
}

bool KoTableColumnStyle::optimalColumnWidth() const
{
    return propertyBoolean(OptimalColumnWidth);
}

void KoTableColumnStyle::setOptimalColumnWidth(bool state)
{
    setProperty(OptimalColumnWidth, state);
}

void KoTableColumnStyle::loadOdf(const KoXmlElement *element, KoOdfLoadingContext &context)
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
    QString family = element->attributeNS(KoXmlNS::style, "family", "table-column");
    context.addStyles(element, family.toLocal8Bit().constData());   // Load all parents - only because we don't support inheritance.

    context.styleStack().setTypeProperties("table-column");   // load all style attributes from "style:table-column-properties"
    loadOdfProperties(context.styleStack());   // load the KoTableColumnStyle from the stylestack
    context.styleStack().restore();
}


void KoTableColumnStyle::loadOdfProperties(KoStyleStack &styleStack)
{
    // Column width.
    if (styleStack.hasProperty(KoXmlNS::style, "column-width")) {
        setColumnWidth(KoUnit::parseValue(styleStack.property(KoXmlNS::style, "column-width")));
    }
    // Relative column width.
    if (styleStack.hasProperty(KoXmlNS::style, "rel-column-width")) {
        setRelativeColumnWidth(styleStack.property(KoXmlNS::style, "rel-column-width").remove('*').toDouble());
    }
    // Optimal column width
    if (styleStack.hasProperty(KoXmlNS::style, "use-optimal-column-width")) {
        setOptimalColumnWidth(styleStack.property(KoXmlNS::style, "use-optimal-column-width") == "true");
    }

    // The fo:break-before and fo:break-after attributes insert a page or column break before or after a column.
    if (styleStack.hasProperty(KoXmlNS::fo, "break-before")) {
        setBreakBefore(KoText::textBreakFromString(styleStack.property(KoXmlNS::fo, "break-before")));
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "break-after")) {
        setBreakAfter(KoText::textBreakFromString(styleStack.property(KoXmlNS::fo, "break-after")));
    }
}

bool KoTableColumnStyle::operator==(const KoTableColumnStyle &other) const
{
    return other.d == d;
}

bool KoTableColumnStyle::operator!=(const KoTableColumnStyle &other) const
{
    return other.d != d;
}

void KoTableColumnStyle::removeDuplicates(const KoTableColumnStyle &other)
{
    d->stylesPrivate.removeDuplicates(other.d->stylesPrivate);
}

void KoTableColumnStyle::saveOdf(KoGenStyle &style) const
{
    QList<int> keys = d->stylesPrivate.keys();
    Q_FOREACH (int key, keys) {
        if (key == KoTableColumnStyle::BreakBefore) {
            style.addProperty("fo:break-before", KoText::textBreakToString(breakBefore()), KoGenStyle::TableColumnType);
        } else if (key == KoTableColumnStyle::BreakAfter) {
            style.addProperty("fo:break-after", KoText::textBreakToString(breakAfter()), KoGenStyle::TableColumnType);
        } else if (key == KoTableColumnStyle::OptimalColumnWidth) {
            style.addProperty("style:use-optimal-column-width", optimalColumnWidth(), KoGenStyle::TableColumnType);
        } else if (key == KoTableColumnStyle::ColumnWidth) {
            style.addPropertyPt("style:column-width", columnWidth(), KoGenStyle::TableColumnType);
        } else if (key == KoTableColumnStyle::RelativeColumnWidth) {
            style.addProperty("style:rel-column-width", QString("%1*").arg(relativeColumnWidth()), KoGenStyle::TableColumnType);
        }
    }
}
