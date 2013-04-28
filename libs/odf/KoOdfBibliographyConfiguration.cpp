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

#include <QList>
#include <QPair>

const QList<QString> KoOdfBibliographyConfiguration::bibTypes = QList<QString>() << "article" << "book" << "booklet" << "conference"
                                                                     << "email" << "inbook" << "incollection"
                                                                     << "inproceedings" << "journal" << "manual"
                                                                     << "mastersthesis" << "misc" << "phdthesis"
                                                                     << "proceedings" << "techreport" << "unpublished"
                                                                     << "www" << "custom1" << "custom2"
                                                                     << "custom3" << "custom4" << "custom5";

const QList<QString> KoOdfBibliographyConfiguration::bibDataFields = QList<QString>() << "address" << "annote" << "author"
                                                                          << "bibliography-type" << "booktitle"
                                                                          << "chapter" << "custom1" << "custom2"
                                                                          << "custom3" << "custom4" << "custom5"
                                                                          << "edition" << "editor" << "howpublished"
                                                                          << "identifier" << "institution" << "isbn"
                                                                          << "issn" << "journal" << "month" << "note"
                                                                          << "number" << "organizations" << "pages"
                                                                          << "publisher" << "report-type" << "school"
                                                                          << "series" << "title" << "url" << "volume"
                                                                          << "year";

class KoOdfBibliographyConfiguration::Private
{
public:
    QString prefix;
    QString suffix;
    bool numberedEntries;
    bool sortByPosition;
    QString sortAlgorithm;
    QList<SortKeyPair> sortKeys;
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
    d->sortKeys = other.d->sortKeys;

    return *this;
}


void KoOdfBibliographyConfiguration::loadOdf(const KoXmlElement &element)
{
    d->prefix = element.attributeNS(KoXmlNS::text, "prefix", QString());
    d->suffix = element.attributeNS(KoXmlNS::text, "suffix", QString());
    d->numberedEntries = (element.attributeNS(KoXmlNS::text, "numbered-entries", QString("false")) == "true")
                         ? true : false;
    d->sortByPosition = (element.attributeNS(KoXmlNS::text, "sort-by-position", QString("true")) == "true")
                        ? true : false;
    d->sortAlgorithm = element.attributeNS(KoXmlNS::text, "sort-algorithm", QString());

    for (KoXmlNode node = element.firstChild(); !node.isNull(); node = node.nextSibling())
    {
        KoXmlElement child = node.toElement();

        if (child.namespaceURI() == KoXmlNS::text && child.localName() == "sort-key") {
            QString key = child.attributeNS(KoXmlNS::text, "key", QString());
            Qt::SortOrder order = (child.attributeNS(KoXmlNS::text, "sort-ascending", "true") == "true")
                    ? (Qt::AscendingOrder): (Qt::DescendingOrder);
            if(!key.isNull() && KoOdfBibliographyConfiguration::bibDataFields.contains(key)) {
                d->sortKeys << QPair<QString, Qt::SortOrder>(key,order);
            }
        }
    }
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

    writer->addAttribute("text:numbered-entries", d->numberedEntries ? "true" : "false");
    writer->addAttribute("text:sort-by-position", d->sortByPosition ? "true" : "false");

    foreach (const SortKeyPair &key, d->sortKeys) {
            writer->startElement("text:sort-key");
            writer->addAttribute("text:key", key.first);
            writer->addAttribute("text:sort-ascending",key.second);
            writer->endElement();
    }
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

QList<SortKeyPair> KoOdfBibliographyConfiguration::sortKeys() const
{
    return d->sortKeys;
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

void KoOdfBibliographyConfiguration::setSortKeys(const QList<SortKeyPair> &sortKeys)
{
    d->sortKeys = sortKeys;
}
