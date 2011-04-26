/*
 *  Copyright (c) 2011 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#include "KoSection.h"

#include "kdebug.h"

#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include <KoShapeLoadingContext.h>
#include <KoTextSharedLoadingData.h>
#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>
#include <KoSectionStyle.h>

class KoSection::Private
{
public:
    Private()
        : sectionStyle(0)
    {
    }

    QString condition;
    QString display;
    QString name;
    QString text_protected;
    QString protection_key;
    QString protection_key_digest_algorithm;
    QString style_name;
    KoSectionStyle *sectionStyle;
};

KoSection::KoSection()
    : d(new Private())
{
}

KoSection::~KoSection()
{
    delete d;
}

KoSection::KoSection(const KoSection& other)
    : d(new Private())
{
    d->condition = other.d->condition;
    d->display = other.d->display;
    d->name = other.d->name;
    d->text_protected = other.d->text_protected;
    d->protection_key = other.d->protection_key;
    d->protection_key_digest_algorithm = other.d->protection_key_digest_algorithm;
    d->style_name = other.d->style_name;
    d->sectionStyle = other.d->sectionStyle;
}

QString KoSection::name() const
{
    return d->name;
}

bool KoSection::loadOdf(const KoXmlElement &element, KoTextSharedLoadingData *sharedData, bool stylesDotXml)
{
    // check whether we really are a section
    if (element.namespaceURI() == KoXmlNS::text && element.localName() == "section") {
        // get all the attributes
        d->condition = element.attributeNS(KoXmlNS::text, "condition");
        d->display = element.attributeNS(KoXmlNS::text, "display");
        if (d->display == "condition" && d->condition.isEmpty()) {
            kWarning(32500) << "Section display is set to \"condition\", but condition is empty.";
        }
        d->name = element.attributeNS(KoXmlNS::text, "name");
        d->text_protected = element.attributeNS(KoXmlNS::text, "text-protected");
        d->protection_key = element.attributeNS(KoXmlNS::text, "protection-key");
        d->protection_key_digest_algorithm = element.attributeNS(KoXmlNS::text, "protection-key-algorithm");
        d->style_name = element.attributeNS(KoXmlNS::text, "style-name", "");

        if (!d->style_name.isEmpty()) {
            d->sectionStyle = sharedData->sectionStyle(d->style_name, stylesDotXml);
        }
        return true;
    }
    return false;
}

void KoSection::saveOdf(KoShapeSavingContext &context)
{
    KoXmlWriter *writer = &context.xmlWriter();
    Q_ASSERT(writer);
    writer->startElement("text:section", false);

    if (!d->condition.isEmpty()) writer->addAttribute("text:condition", d->condition);
    if (!d->display.isEmpty()) writer->addAttribute("text:display", d->condition);
    if (!d->name.isEmpty()) writer->addAttribute("text:name", d->name);
    if (!d->text_protected.isEmpty()) writer->addAttribute("text:text-protected", d->text_protected);
    if (!d->protection_key.isEmpty()) writer->addAttribute("text:protection-key", d->protection_key);
    if (!d->protection_key_digest_algorithm.isEmpty()) writer->addAttribute("text:protection-key-digest-algorihtm", d->protection_key_digest_algorithm);
    if (!d->style_name.isEmpty()) writer->addAttribute("text:style-name", d->style_name);
}

void KoSectionEnd::saveOdf(KoShapeSavingContext &context)
{
    KoXmlWriter *writer = &context.xmlWriter();
    Q_ASSERT(writer);
    writer->endElement();
}
