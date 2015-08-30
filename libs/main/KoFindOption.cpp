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

#include "KoFindOption.h"

class Q_DECL_HIDDEN KoFindOption::Private
{
public:
    Private() { }
    ~Private() { }

    QString name;
    QString title;
    QString description;
    QVariant value;
};

KoFindOption::KoFindOption(const QString &name, QObject *parent)
        : QObject(parent), d(new Private)
{
    d->name = name;
}

KoFindOption::~KoFindOption()
{
    delete d;
}

QString KoFindOption::name() const
{
    return d->name;
}

QString KoFindOption::title() const
{
    return d->title;
}

QString KoFindOption::description() const
{
    return d->description;
}

QVariant KoFindOption::value() const
{
    return d->value;
}

void KoFindOption::setTitle(const QString &newTitle)
{
    d->title = newTitle;
}

void KoFindOption::setDescription(const QString &newDescription)
{
    d->description = newDescription;
}

void KoFindOption::setValue(const QVariant &newValue)
{
    d->value = newValue;
}
