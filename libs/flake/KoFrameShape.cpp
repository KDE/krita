/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2008 Thorsten Zachmann <zachmann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KoFrameShape.h"

#include <KoXmlReader.h>
#include <FlakeDebug.h>

class Q_DECL_HIDDEN KoFrameShape::Private
{
public:
    Private(const QString &ns, const QString &tag)
            : ns(ns)
            , tag(tag) {}

    Private(const Private &rhs)
            : ns(rhs.ns)
            , tag(rhs.tag) {}

    const QString ns;
    const QString tag;
};

KoFrameShape::KoFrameShape(const QString &ns, const QString &tag)
    : d(new Private(ns, tag))
{
}

KoFrameShape::KoFrameShape(const KoFrameShape &rhs)
    : d(new Private(*rhs.d))
{
}

KoFrameShape::~KoFrameShape()
{
    delete d;
}

bool KoFrameShape::loadOdfFrame(const QDomElement & element, KoShapeLoadingContext &context)
{
    const QDomElement & frameElement(KoXml::namedItemNS(element, d->ns, d->tag));
    if (frameElement.isNull()) {
        errorFlake << "frame element" << d->tag << "not found";
        return false;
    }

    return loadOdfFrameElement(frameElement, context);
}
