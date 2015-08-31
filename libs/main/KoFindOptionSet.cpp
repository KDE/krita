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

#include "KoFindOptionSet.h"
#include "KoFindOption.h"

#include <QHash>

class Q_DECL_HIDDEN KoFindOptionSet::Private
{
public:
    Private() : nextID(0) { }
    QHash<QString, KoFindOption *> options;

    int nextID;
};

KoFindOptionSet::KoFindOptionSet(QObject *parent)
    : QObject(parent), d(new Private)
{
}

KoFindOptionSet::~KoFindOptionSet()
{
    qDeleteAll(d->options.values());
    delete d;
}

KoFindOption *KoFindOptionSet::option(const QString &name) const
{
    if(d->options.contains(name)) {
        return d->options.value(name);
    }
    return 0;
}

QList<KoFindOption *> KoFindOptionSet::options() const
{
    return d->options.values();
}

KoFindOption *KoFindOptionSet::addOption(const QString &name)
{
    KoFindOption *newOption = new KoFindOption(name);
    d->options.insert(name, newOption);
    return newOption;
}

KoFindOption *KoFindOptionSet::addOption(const QString &name, const QString &title, const QString &description, const QVariant &value)
{
    KoFindOption *newOption = new KoFindOption(name);
    newOption->setTitle(title);
    newOption->setDescription(description);
    newOption->setValue(value);
    d->options.insert(name, newOption);
    return newOption;
}

void KoFindOptionSet::removeOption(const QString &name)
{
    if(d->options.contains(name)) {
        d->options.remove(name);
    }
}

void KoFindOptionSet::setOptionValue(const QString &name, const QVariant &value)
{
    if(d->options.contains(name)) {
        d->options.value(name)->setValue(value);
    }
}

void KoFindOptionSet::replaceOption(const QString &name, KoFindOption *newOption)
{
    Q_ASSERT(newOption);
    if(d->options.contains(name)) {
        d->options.insert(name, newOption);
    }
}
