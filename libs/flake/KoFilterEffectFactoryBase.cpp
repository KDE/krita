/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "KoFilterEffectFactoryBase.h"
#include <QString>

class Q_DECL_HIDDEN KoFilterEffectFactoryBase::Private
{
public:
    Private(const QString &_id, const QString &_name) : id(_id), name(_name)
    {
    }
    const QString id;
    const QString name;
};

KoFilterEffectFactoryBase::KoFilterEffectFactoryBase(const QString &id, const QString &name)
    : d(new Private(id, name))
{
}

KoFilterEffectFactoryBase::~KoFilterEffectFactoryBase()
{
    delete d;
}

QString KoFilterEffectFactoryBase::name() const
{
    return d->name;
}

QString KoFilterEffectFactoryBase::id() const
{
    return d->id;
}
