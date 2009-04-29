/* This file is part of the KDE project
 * Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_koffice@gadz.org>
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

#include "KoChangeTrackerElement.h"

#include <KDebug>

class KoChangeTrackerElement::Private
{
public:
    Private() {}
    ~Private() {}

    QString title;
    KoGenChange::Type type;
    QTextFormat changeFormat;
    QTextFormat prevFormat;

    QString creator;
    QString date;
    QString extraMetaData;
    QString deleteData;

    bool enabled;
};

KoChangeTrackerElement::KoChangeTrackerElement(QString title, KoGenChange::Type type)
    :d(new Private())
{
    d->title = title;
    d->type = type;
}

KoChangeTrackerElement::KoChangeTrackerElement()
    :d(new Private())
{
}

KoChangeTrackerElement::~KoChangeTrackerElement()
{
}

void KoChangeTrackerElement::setEnabled(bool enabled)
{
    kDebug() << "in chage element set enabled: " << enabled;
    d->enabled = enabled;
}

bool KoChangeTrackerElement::isEnabled()
{
    return d->enabled;
}

void KoChangeTrackerElement::setChangeType(KoGenChange::Type type)
{
    d->type = type;
}

KoGenChange::Type KoChangeTrackerElement::getChangeType()
{
    return d->type;
}

void KoChangeTrackerElement::setChangeTitle(QString title)
{
    d->title = title;
}

QString KoChangeTrackerElement::getChangeTitle()
{
    return d->title;
}

void KoChangeTrackerElement::setChangeFormat(QTextFormat &format)
{
    d->changeFormat = format;
}

QTextFormat KoChangeTrackerElement::getChangeFormat()
{
    return d->changeFormat;
}

void KoChangeTrackerElement::setPrevFormat(QTextFormat &format)
{
    d->prevFormat = format;
}

QTextFormat KoChangeTrackerElement::getPrevFormat()
{
    return d->prevFormat;
}

bool KoChangeTrackerElement::hasCreator()
{
    return !d->creator.isEmpty();
}

void KoChangeTrackerElement::setCreator(QString creator)
{
    d->creator = creator;
}

QString KoChangeTrackerElement::getCreator()
{
    return d->creator;
}

bool KoChangeTrackerElement::hasDate()
{
    return !d->date.isEmpty();
}

void KoChangeTrackerElement::setDate(QString date)
{
    d->date = date;
}

QString KoChangeTrackerElement::getDate()
{
    return d->date;
}

bool KoChangeTrackerElement::hasExtraMetaData()
{
    return !d->extraMetaData.isEmpty();
}

void KoChangeTrackerElement::setExtraMetaData(QString metaData)
{
    d->extraMetaData = metaData;
}

QString KoChangeTrackerElement::getExtraMetaData()
{
    return d->extraMetaData;
}

bool KoChangeTrackerElement::hasDeleteData()
{
    return !d->deleteData.isEmpty();
}

void KoChangeTrackerElement::setDeleteData(QString data)
{
    d->deleteData = data;
}

QString KoChangeTrackerElement::getDeleteData()
{
    return d->deleteData;
}
