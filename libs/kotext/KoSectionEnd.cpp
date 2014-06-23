/*
 *  Copyright (c) 2011 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2014 Denis Kuplyakov <dener.kup@gmail.com>
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

#include "KoSectionEnd.h"

#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>

class KoSectionEnd::Private
{
public:
    Private(KoSection *_section)
    : section(_section)
    {
        Q_ASSERT(section);
    }

    KoSection *section; //< pointer to the corresponding section
};

KoSectionEnd::KoSectionEnd(KoSection *_section)
    : d(new Private(_section))
{
    _section->setSectionEnd(this);
}

KoSectionEnd::~KoSectionEnd()
{
    delete d;
}

QString KoSectionEnd::name() const
{
    return d->section->name();
}

KoSection* KoSectionEnd::correspondingSection()
{
    return d->section;
}

void KoSectionEnd::saveOdf(KoShapeSavingContext &context)
{
    KoXmlWriter *writer = &context.xmlWriter();
    Q_ASSERT(writer);
    writer->endElement();
}
