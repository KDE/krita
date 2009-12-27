/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "KoInlineObjectFactory.h"

#include <KoProperties.h>

class InlineObjectFactoryPrivate
{
public:
    InlineObjectFactoryPrivate(const QString &identifier)
            : id(identifier) {
    }

    ~InlineObjectFactoryPrivate() {
        foreach(const KoInlineObjectTemplate & t, templates)
            delete t.properties;
        templates.clear();
    }

    const QString id;
    QString iconName;
    QList<KoInlineObjectTemplate> templates;
};

KoInlineObjectFactory::KoInlineObjectFactory(QObject *parent, const QString &id)
        : QObject(parent)
        , d(new InlineObjectFactoryPrivate(id))
{
}

KoInlineObjectFactory::~KoInlineObjectFactory()
{
    delete d;
}

const QString &KoInlineObjectFactory::id() const
{
    return d->id;
}

const QList<KoInlineObjectTemplate> KoInlineObjectFactory::templates() const
{
    return d->templates;
}

void KoInlineObjectFactory::addTemplate(const KoInlineObjectTemplate &params)
{
    d->templates.append(params);
}

#include <KoInlineObjectFactory.moc>
