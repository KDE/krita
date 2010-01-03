/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "KoColorTransformationFactory.h"

struct KoColorTransformationFactory::Private {
    QString id;
    QString name;
};

KoColorTransformationFactory::KoColorTransformationFactory(QString id, QString name) : d(new Private)
{
    d->id = id;
    d->name = name;
}

KoColorTransformationFactory::~KoColorTransformationFactory()
{
    delete d;
}

QString KoColorTransformationFactory::id() const
{
    return d->id;
}

QString KoColorTransformationFactory::name() const
{
    return d->name;
}
