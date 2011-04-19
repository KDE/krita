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

#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>

class KoSection::Private
{
public:
    Private()
    {
    }

    QString condition;
    QString display;
    QString name;
    QString text_protected;
    QString protection_key;
    QString protection_key_digest_algorithm;
    QString style_name;
    QString id;

};

KoSection::KoOdfSection()
    : d(new Private())
{
}

KoSection::~KoOdfSection()
{
    delete d;
}

bool KoSection::loadOdf(const KoXmlElement &element, KoShapeLoadingContext *context)
{
    // check whether we really are a section
    if (element.namespaceURI() == KoXmlNS::text && element.localName() == "section") {
        // get all the attributes
        d->condition = element.attributeNS(KoXmlNS::text, "condition");
        d->display = element.attributeNS(KoXmlNS::text, "display");
        d->name = element.attributeNS(KoXmlNS::text, "name");
        d->text_protected = element.attributeNS(KoXmlNS::text, "text-protected");
        d->protection_key = element.attributeNS(KoXmlNS::text, "protection-key");
        d->protection_key_digest_algorithm = element.attributeNS(KoXmlNS::text, "protection-key-algorithm");
        d->style_name = element.attributeNS(KoXmlNS::text, "style-name");
        d->id = element.attributeNS(KoXmlNS::text, "id");
        return true;
    }
    return false;
}

void KoSection::saveOdf(KoShapeShavingContext &context)
{

}
