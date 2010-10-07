/* This file is part of the KDE project
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#include "KoShapeBorderFactoryBase.h"
#include <QString>
class KoShapeBorderFactoryBase::Private
{
public:
    Private(const QString &i) : id(i) { }
    const QString id;
};

KoShapeBorderFactoryBase::KoShapeBorderFactoryBase(const QString &id)
    : d(new Private(id))
{
}

KoShapeBorderFactoryBase::~KoShapeBorderFactoryBase()
{
    delete d;
}

QString KoShapeBorderFactoryBase::id() const
{
    return d->id;
}
