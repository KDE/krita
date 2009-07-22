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
#include "KoTableColumnStyle.h"
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

class KoTableColumnStyle::Private
{
public:
    Private() : parentStyle(0), next(0) {}

    ~Private() {
    }

    void setProperty(int key, const QVariant &value) {
        stylesPrivate.add(key, value);
    }

    QString name;
    KoTableColumnStyle *parentStyle;
    int next;
    StylePrivate stylesPrivate;
};

KoTableColumnStyle::KoTableColumnStyle(QObject *parent)
        : QObject(parent), d(new Private())
{
}

/*
KoTableColumnStyle::KoTableColumnStyle(const QTextTableFormat &tableFormat, QObject *parent)
        : QObject(parent),
        d(new Private())
{
    d->stylesPrivate = tableFormat.properties();
}

KoTableColumnStyle *KoTableColumnStyle::fromTable(const QTextTable &table, QObject *parent)
{
    QTextTableFormat tableFormat = table.format();
    return new KoTableColumnStyle(tableFormat, parent);
}
*/

KoTableColumnStyle::~KoTableColumnStyle()
{
    delete d;
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

/*
void KoTableColumnStyle::applyStyle(QTextTableFormat &format) const
{
    if (d->parentStyle) {
        d->parentStyle->applyStyle(format);
    }
    QList<int> keys = d->stylesPrivate.keys();
    for (int i = 0; i < keys.count(); i++) {
        QVariant variant = d->stylesPrivate.value(keys[i]);
        format.setProperty(keys[i], variant);
    }
}
*/

void KoTableColumnStyle::setColumnWidth(const QTextLength &width)
{
}

void KoTableColumnStyle::setBreakBefore(bool on)
{
    setProperty(BreakBefore, on);
}

bool KoTableColumnStyle::breakBefore()
{
    return propertyBoolean(BreakBefore);
}

void KoTableColumnStyle::setBreakAfter(bool on)
{
    setProperty(BreakAfter, on);
}

bool KoTableColumnStyle::breakAfter()
{
    return propertyBoolean(BreakAfter);
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
    emit nameChanged(name);
}

int KoTableColumnStyle::styleId() const
{
    return propertyInt(StyleId);
}

void KoTableColumnStyle::setStyleId(int id)
{
    setProperty(StyleId, id); if (d->next == 0) d->next = id;
}

QString KoTableColumnStyle::masterPageName() const
{
    return value(MasterPageName).toString();
}

void KoTableColumnStyle::setMasterPageName(const QString &name)
{
    setProperty(MasterPageName, name);
}

void KoTableColumnStyle::loadOdf(const KoXmlElement *element, KoOdfLoadingContext &context)
{
    if (element->hasAttributeNS(KoXmlNS::style, "display-name"))
        d->name = element->attributeNS(KoXmlNS::style, "display-name", QString());

    if (d->name.isEmpty()) // if no style:display-name is given us the style:name
        d->name = element->attributeNS(KoXmlNS::style, "name", QString());

    QString masterPage = element->attributeNS(KoXmlNS::style, "master-page-name", QString());
    if (! masterPage.isNull()) {
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
    // Column Width
    if (styleStack.hasProperty(KoXmlNS::style, "column-width")) {
        setColumnWidth(QTextLength(QTextLength::FixedLength, KoUnit::parseValue(styleStack.property(KoXmlNS::style, "column-width"))));
    }
    if (styleStack.hasProperty(KoXmlNS::style, "rel-column-width")) {
        setColumnWidth(QTextLength(QTextLength::PercentageLength, styleStack.property(KoXmlNS::style, "rel-column-width").remove('%').toDouble()));
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
}

void KoTableColumnStyle::copyProperties(const KoTableColumnStyle *style)
{
    d->stylesPrivate = style->d->stylesPrivate;
    setName(style->name()); // make sure we emit property change
    d->next = style->d->next;
    d->parentStyle = style->d->parentStyle;
}

KoTableColumnStyle *KoTableColumnStyle::clone(QObject *parent)
{
    KoTableColumnStyle *newStyle = new KoTableColumnStyle(parent);
    newStyle->copyProperties(this);
    return newStyle;
}


bool KoTableColumnStyle::operator==(const KoTableColumnStyle &other) const
{
    return other.d->stylesPrivate == d->stylesPrivate;
}

void KoTableColumnStyle::removeDuplicates(const KoTableColumnStyle &other)
{
    d->stylesPrivate.removeDuplicates(other.d->stylesPrivate);
}

void KoTableColumnStyle::saveOdf(KoGenStyle &style)
{
/*
    QList<int> keys = d->stylesPrivate.keys();
    foreach(int key, keys) {
        if (key == KoTableColumnStyle::BreakBefore) {
            if (breakBefore())
                style.addProperty("fo:break-before", "page", KoGenStyle::ParagraphType);
        } else if (key == KoTableColumnStyle::BreakAfter) {
            if (breakAfter())
                style.addProperty("fo:break-after", "page", KoGenStyle::ParagraphType);
        } 
*/
}

#include "KoTableColumnStyle.moc"
