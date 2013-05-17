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
#include "KoOdfStyleProperties.h"

// Qt
#include <QString>
#include <QHash>

// KDE
#include <kdebug.h>

// odflib
#include <KoXmlStreamReader.h>
#include <KoXmlWriter.h>


// ================================================================
//                 class KoOdfStyleProperties


class KoOdfStyleProperties::Private
{
public:
    Private() {};
    ~Private() {};

    QHash<QString, QString> attributes;  // name, value
};


// ----------------------------------------------------------------


KoOdfStyleProperties::KoOdfStyleProperties()
    : d(new KoOdfStyleProperties::Private())
{
}

KoOdfStyleProperties::~KoOdfStyleProperties()
{
    delete d;
}


QString KoOdfStyleProperties::attribute(QString &property) const
{
    return d->attributes.value(property, QString());
}

void KoOdfStyleProperties::setAttribute(QString &property, QString &value)
{
    d->attributes[property] = value;
}


void KoOdfStyleProperties::clear()
{
    d->attributes.clear();
}


bool KoOdfStyleProperties::readOdf(KoXmlStreamReader &reader)
{
    // The default implementation just read the attributes.  The
    // inheriting classes will also read various types of children.
    bool retval = readAttributes(reader);
    reader.skipCurrentElement();

    return retval;
}

bool KoOdfStyleProperties::saveOdf(const QString &propertySet, KoXmlWriter *writer)
{
    writer->startElement(propertySet.toLatin1()); // e.g. text-properties
    foreach(const QString &property, d->attributes.keys()) {
        writer->addAttribute(property.toLatin1(), d->attributes[property]);
    }
    writer->endElement(); // propertySet

    return true;
}


// ----------------------------------------------------------------
//                         protected functions


bool KoOdfStyleProperties::readAttributes(KoXmlStreamReader &reader)
{
    KoXmlStreamAttributes attrs = reader.attributes();
    foreach (const KoXmlStreamAttribute &attr, attrs) {
        d->attributes.insert(attr.qualifiedName().toString(), attr.value().toString());
    }

    //kDebug() << "read attributes: " << d->attributes;

    return true;
}

