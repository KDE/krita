/* This file is part of the KDE project
   Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoFrameShape.h"

#include <KoXmlReader.h>
#include <kdebug.h>

class KoFrameShape::Private
{
public:
    Private(const char * ns, const char * tag)
            : ns(ns)
            , tag(tag) {}

    const char * ns;
    const char * tag;
};

KoFrameShape::KoFrameShape(const char * ns, const char * tag)
        : d(new Private(ns, tag))
{
}

KoFrameShape::~KoFrameShape()
{
    delete d;
}

bool KoFrameShape::loadOdfFrame(const KoXmlElement & element, KoShapeLoadingContext &context)
{
    const KoXmlElement & frameElement(KoXml::namedItemNS(element, d->ns, d->tag));
    if (frameElement.isNull()) {
        kError(30006) << "frame element" << d->tag << "not found" << endl;
        return false;
    }

    return loadOdfFrameElement(frameElement, context);
}
