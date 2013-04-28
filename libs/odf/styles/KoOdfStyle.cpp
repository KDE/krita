/* This file is part of the KDE project
 *
 * Copyright (C) 2013 Inge Wallin <inge@lysator.liu.se>
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


// Own
#include "KoOdfStyle.h"

// Qt
#include <QString>

// KDE
#include <kdebug.h>

// Odflib
#include "KoXmlStreamReader.h"
#include "KoXmlWriter.h"
#include "KoOdfStyleProperties.h"


// ================================================================
//                         class KoOdfStyle


class KoOdfStyle::Private
{
public:
    Private();
    ~Private();

    QString name;
    QString family;
    QString parent;
    QString displayName;
    bool    isDefaultStyle;

    bool    inUse;

    // 
    QHash<QString, KoOdfStyleProperties*> properties;  // e.g. "text-properties", 
};

KoOdfStyle::Private::Private()
    : isDefaultStyle(false)
    , inUse(false)
{
}

KoOdfStyle::Private::~Private()
{
    qDeleteAll(properties);
}


// ----------------------------------------------------------------


KoOdfStyle::KoOdfStyle()
    : d(new KoOdfStyle::Private())
{
}

KoOdfStyle::~KoOdfStyle()
{
    delete d;
}


QString KoOdfStyle::name() const
{
    return d->name;
}

void KoOdfStyle::setName(QString &name)
{
    d->name = name;
}

QString KoOdfStyle::family() const
{
    return d->family;
}

void KoOdfStyle::setFamily(QString &family)
{
    d->family = family;
}

QString KoOdfStyle::parent() const
{
    return d->parent;
}

void KoOdfStyle::setParent(QString &parent)
{
    d->parent = parent;
}

QString KoOdfStyle::displayName() const
{
    return d->displayName;
}

void KoOdfStyle::setDisplayName(QString &name)
{
    d->displayName = name;
}


bool KoOdfStyle::isDefaultStyle() const
{
    return d->isDefaultStyle;
}

void KoOdfStyle::setIsDefaultStyle(bool isDefaultStyle)
{
    d->isDefaultStyle = isDefaultStyle;
}



bool KoOdfStyle::inUse() const
{
    return d->inUse;
}

void KoOdfStyle::setInUse(bool inUse)
{
    d->inUse = inUse;
}


QHash<QString, KoOdfStyleProperties*> KoOdfStyle::properties()
{
    return d->properties;
}

KoOdfStyleProperties *KoOdfStyle::properties(QString &name) const
{
    return d->properties.value(name, 0);
}

QString KoOdfStyle::property(QString &propertySet, QString &property) const
{
    KoOdfStyleProperties *props = d->properties.value(propertySet, 0);
    if (props)
        return props->value(property);
    else
        return QString();
}

void KoOdfStyle::setProperty(QString &propertySet, QString &property, QString &value)
{
    KoOdfStyleProperties *props = d->properties.value(propertySet);
    if (!props)
        props = new KoOdfStyleProperties();
    props->setValue(property, value);
}


bool KoOdfStyle::readOdf(KoXmlStreamReader &reader)
{
    // Load style attributes.
    KoXmlStreamAttributes  attrs = reader.attributes();
    QString dummy;              // Because the set*() methods take a QString &,

    dummy = attrs.value("style:family").toString();
    setFamily(dummy);
    dummy = attrs.value("style:name").toString();
    setName(dummy);
    dummy = attrs.value("style:parent-style-name").toString();
    setParent(dummy);
    dummy = attrs.value("style:display-name").toString();
    setDisplayName(dummy);

    kDebug() << "Style:" << name() << family() << parent() << displayName();

    // Load child elements: property sets and other children.
    while (reader.readNextStartElement()) {

        // So far we only have support for text-, paragaph- and graphic-properties
        QString propertiesType = reader.qualifiedName().toString();
        if (propertiesType == "style:text-properties"
            || propertiesType == "style:paragraph-properties"
            || propertiesType == "style:graphic-properties")
        {
            kDebug() << "properties type: " << propertiesType;

            // FIXME: In the future, create per type.
            KoOdfStyleProperties *properties = new KoOdfStyleProperties();
            if (!properties->readOdf(reader)) {
                return false;
            }
            d->properties[propertiesType] = properties;
        }
    }

    return true;
}

bool KoOdfStyle::saveOdf(KoXmlWriter *writer)
{
    if (d->isDefaultStyle) {
        writer->startElement("style:default-style");
        writer->addAttribute("style:name", d->name);
    }
    else {
        writer->startElement("style:style");
    }

    // Write style attributes
    writer->addAttribute("style:family", d->family);
    if (!d->parent.isEmpty()) {
        writer->addAttribute("style:parent-style-name", d->parent);
    }
    if (!d->displayName.isEmpty()) {
        writer->addAttribute("style:display-name", d->displayName);
    }

    // Write properties
    foreach(const QString &propertySet, d->properties.keys()) {
        d->properties.value(propertySet)->saveOdf(propertySet, writer);
    }

    writer->endElement();  // style:{default-,}style
    return true;
}



