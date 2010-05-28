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

#include "KoInlineObjectFactoryBase.h"

#include <KoProperties.h>

#include <QStringList>

class InlineObjectFactoryPrivate
{
public:
    InlineObjectFactoryPrivate(const QString &identifier)
            : id(identifier) {
    }

    ~InlineObjectFactoryPrivate() {
        foreach(const KoInlineObjectTemplate &t, templates)
            delete t.properties;
        templates.clear();
    }

    const QString id;
    QString iconName;
    QString odfNameSpace;
    QStringList odfElementNames;
    QList<KoInlineObjectTemplate> templates;
    KoInlineObjectFactoryBase::ObjectType type;
};

KoInlineObjectFactoryBase::KoInlineObjectFactoryBase(QObject *parent, const QString &id, ObjectType type)
        : QObject(parent)
        , d(new InlineObjectFactoryPrivate(id))
{
    d->type = type;
}

KoInlineObjectFactoryBase::~KoInlineObjectFactoryBase()
{
    delete d;
}

QString KoInlineObjectFactoryBase::id() const
{
    return d->id;
}

QList<KoInlineObjectTemplate> KoInlineObjectFactoryBase::templates() const
{
    return d->templates;
}

void KoInlineObjectFactoryBase::addTemplate(const KoInlineObjectTemplate &params)
{
    d->templates.append(params);
}

QStringList KoInlineObjectFactoryBase::odfElementNames() const
{
    return d->odfElementNames;
}

QString KoInlineObjectFactoryBase::odfNameSpace() const
{
    return d->odfNameSpace;
}

void KoInlineObjectFactoryBase::setOdfElementNames(const QString & nameSpace, const QStringList &names)
{
    d->odfNameSpace = nameSpace;
    d->odfElementNames = names;
}

KoInlineObjectFactoryBase::ObjectType KoInlineObjectFactoryBase::type() const
{
    return d->type;
}

#include <KoInlineObjectFactoryBase.moc>
