/*
 *  Copyright (c) 2011 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoElementReference.h"

#include "KoXmlReader.h"
#include "KoXmlWriter.h"
#include <KoXmlNS.h>

KoElementReference::KoElementReference()
    : d(new KoElementReferenceData())
{
    d->xmlid = "id-" + d->xmlid;
}

KoElementReference::KoElementReference(const QString &prefix)
    : d(new KoElementReferenceData)
{
    d->xmlid = prefix + "-" + d->xmlid;
}

KoElementReference::KoElementReference(const QString &prefix, int counter)
    : d(new KoElementReferenceData)
{
    d->xmlid = QString("%1-%2").arg(prefix).arg(counter);
}

KoElementReference::KoElementReference(const KoElementReference &other)
    : d(other.d)
{
}

KoElementReference &KoElementReference::operator=(const KoElementReference &rhs)
{
    if (this == &rhs) return *this;
    d = rhs.d;

    return *this;
}

bool KoElementReference::operator==(const KoElementReference &other) const
{
    return d->xmlid == other.d->xmlid;
}

bool KoElementReference::operator!=(const KoElementReference &other) const
{
    return !(*this == other);
}

bool KoElementReference::isValid() const
{
    return (!d->xmlid.isEmpty());
}

void KoElementReference::saveOdf(KoXmlWriter *writer, SaveOption saveOptions) const
{
    if (d->xmlid.isEmpty()) return;

    writer->addAttribute("xml:id", d->xmlid);

    if (saveOptions & DrawId) {
        writer->addAttribute("draw:id", d->xmlid);
    }
    if (saveOptions & TextId) {
        writer->addAttribute("text:id", d->xmlid);
    }
}

QString KoElementReference::toString() const
{
    return d->xmlid;
}


KoElementReference KoElementReference::loadOdf(const KoXmlElement &element)
{
    QString xmlid;

    if (element.hasAttributeNS(KoXmlNS::xml, "id")) {
        xmlid = element.attributeNS(KoXmlNS::xml, "id");
    }
    else if (element.hasAttributeNS(KoXmlNS::draw, "id")) {
        xmlid = element.attributeNS(KoXmlNS::draw, "id");
    }
    else if (element.hasAttributeNS(KoXmlNS::text, "id")) {
        xmlid = element.attributeNS(KoXmlNS::text, "id");
    }

    d->xmlid = xmlid;

    return *this;
}

void KoElementReference::invalidate()
{
    d->xmlid.clear();
}
