/* This file is part of the KDE project
 * Copyright (C) 2011 Smit Patel <smitpatel24@gmail.com>
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
#include "KoOdfBibliographyConfiguration.h"
#include <kdebug.h>
#include <KoXmlNS.h>
#include "KoUnit.h"

class KoOdfBibliographyConfiguration::Private
{
public:
    QString prefix;
    QString suffix;
    bool numberedEntries;
    bool sortByPosition;
    QString sortAlgorithm;
};

KoOdfBibliographyConfiguration::KoOdfBibliographyConfiguration()
    : d(new Private())
{
    d->prefix = "[";
    d->suffix = "]";
    d->numberedEntries = false;
    d->sortByPosition = true;
}

KoOdfBibliographyConfiguration::~KoOdfBibliographyConfiguration()
{
    delete d;
}

KoOdfBibliographyConfiguration::KoOdfBibliographyConfiguration(const KoOdfBibliographyConfiguration &other)
    : QObject(), d(new Private())
{
    *this = other;
}

KoOdfBibliographyConfiguration &KoOdfBibliographyConfiguration::operator=(const KoOdfBibliographyConfiguration &other)
{
    d->prefix = other.d->prefix;
    d->suffix = other.d->suffix;
    d->numberedEntries = other.d->numberedEntries;
    d->sortAlgorithm = other.d->sortAlgorithm;
    d->sortByPosition = other.d->sortByPosition;

    return *this;
}


void KoOdfBibliographyConfiguration::loadOdf(const KoXmlElement &element)
{
    d->prefix = element.attributeNS(KoXmlNS::text, "prefix", QString::null);
    d->suffix = element.attributeNS(KoXmlNS::text, "suffix", QString::null);
    d->numberedEntries = (element.attributeNS(KoXmlNS::text, "numbered-entries", QString("false"))=="true")?true:false;
    d->sortByPosition = (element.attributeNS(KoXmlNS::text, "sort-by-position", QString("false"))=="true")?true:false;
    d->sortAlgorithm = element.attributeNS(KoXmlNS::text, "sort-algorithm", QString::null);
}

void KoOdfBibliographyConfiguration::saveOdf(KoXmlWriter *writer) const
{
    writer->startElement("text:bibliography-configuration");

    if (!d->prefix.isNull()) {
        writer->addAttribute("text:prefix", d->prefix);
    }

    if (!d->suffix.isNull()) {
        writer->addAttribute("text:suffix", d->suffix);
    }

    if (!d->sortAlgorithm.isNull()) {
        writer->addAttribute("text:sort-algorithm", d->sortAlgorithm);
    }

    writer->addAttribute("text:numbered-entries",d->numberedEntries?"true":"false");
    writer->addAttribute("text:sort-by-position",d->sortByPosition?"true":"false");

    writer->endElement();
}

QString KoOdfBibliographyConfiguration::prefix() const
{
    return d->prefix;
}

QString KoOdfBibliographyConfiguration::suffix() const
{
    return d->suffix;
}

QString KoOdfBibliographyConfiguration::sortAlgorithm() const
{
    return d->sortAlgorithm;
}

bool KoOdfBibliographyConfiguration::sortByPosition() const
{
    return d->sortByPosition;
}

bool KoOdfBibliographyConfiguration::numberedEntries() const
{
    return d->numberedEntries;
}

void KoOdfBibliographyConfiguration::setNumberedEntries(bool enable)
{
    d->numberedEntries = enable;
}

void KoOdfBibliographyConfiguration::setPrefix(const QString &prefixValue)
{
    d->prefix = prefixValue;
}

void KoOdfBibliographyConfiguration::setSuffix(const QString &suffixValue)
{
    d->suffix = suffixValue;
}

void KoOdfBibliographyConfiguration::setSortAlgorithm(const QString &algorithm)
{
    d->sortAlgorithm = algorithm;
}

void KoOdfBibliographyConfiguration::setSortByPosition(bool enable)
{
    d->sortByPosition = enable;
}
