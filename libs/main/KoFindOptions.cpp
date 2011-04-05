/* This file is part of the KDE project
 *
 * Copyright (c) 2010 Arjen Hiemstra <ahiemstra@heimr.nl>
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

#include "KoFindOptions.h"
#include "KoFindOption.h"

class KoFindOptions::Private
{
public:
    QList<KoFindOption> options;
};

KoFindOptions::KoFindOptions(QObject* parent)
    : QObject(parent), d(new Private)
{

}

KoFindOptions::~KoFindOptions()
{

}

KoFindOption KoFindOptions::option(int id)
{
    if(id > 0 && id < d->options.size()) {
        return d->options.at(id);
    }
}

const QList< KoFindOption > KoFindOptions::options() const
{
    return d->options;
}

KoFindOption KoFindOptions::addOption()
{
    KoFindOption newOption;
    d->options.append(newOption);
    return newOption;
}

KoFindOption KoFindOptions::addOption(const QString& title, const QString& description, const QVariant& value)
{
    KoFindOption newOption;
    newOption.setTitle(title);
    newOption.setDescription(description);
    newOption.setValue(value);
    d->options.append(newOption);
    return newOption;
}

void KoFindOptions::removeOption(int id)
{
    if(id >= 0 && id < d->options.size()) {
        d->options.removeAt(id);
    }
}

void KoFindOptions::removeOption(const KoFindOption& remove)
{
    d->options.removeOne(remove);
}

void KoFindOptions::setOptionValue(int id, const QVariant& value)
{
    if(id >= 0 && id < d->options.size()) {
        KoFindOption opt = d->options.at(id);
        opt.setValue(value);
        d->options.replace(id, opt);
    }
}

void KoFindOptions::setOption(const KoFindOption& newOption)
{
    if(newOption.id() >= 0 && newOption.id() < d->options.size()) {
        d->options.replace(newOption.id(), newOption);
    }
}
