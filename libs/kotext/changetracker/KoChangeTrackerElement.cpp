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
    //These two elements are valid for delete changes. Need to move it to a sub-class
    QString deleteData;
    KoDeleteChangeMarker *marker;

    bool enabled;
    bool acceptedRejected;
    bool valid;
};

KoChangeTrackerElement::KoChangeTrackerElement(const QString& title, KoGenChange::Type type)
    :d(new Private())
{
    d->title = title;
    d->type = type;
    d->acceptedRejected = false;
    d->valid = true;
}

KoChangeTrackerElement::KoChangeTrackerElement()
    :d(new Private())
{
}

KoChangeTrackerElement::KoChangeTrackerElement(const KoChangeTrackerElement& other)
    :d(new Private())
{
    d->title = other.d->title;
    d->type = other.d->type;
    d->changeFormat = other.d->changeFormat;
    d->prevFormat = other.d->prevFormat;
    d->creator = other.d->creator;
    d->date = other.d->date;
    d->extraMetaData = other.d->extraMetaData;
    d->deleteData = other.d->deleteData;
    d->enabled = other.d->enabled;
    d->acceptedRejected = other.d->acceptedRejected;
    d->valid = other.d->valid;
}

KoChangeTrackerElement::~KoChangeTrackerElement()
{
    delete d;
}

void KoChangeTrackerElement::setEnabled(bool enabled)
{
    d->enabled = enabled;
}

bool KoChangeTrackerElement::isEnabled() const
{
    return d->enabled;
}

void KoChangeTrackerElement::setAcceptedRejected(bool set)
{
    d->acceptedRejected = set;
}

bool KoChangeTrackerElement::acceptedRejected()
{
    return d->acceptedRejected;
}

void KoChangeTrackerElement::setValid(bool valid)
{
    d->valid = valid;
}

bool KoChangeTrackerElement::isValid() const
{
    return d->valid;
}

void KoChangeTrackerElement::setChangeType(KoGenChange::Type type)
{
    d->type = type;
}

KoGenChange::Type KoChangeTrackerElement::getChangeType() const
{
    return d->type;
}

void KoChangeTrackerElement::setChangeTitle(const QString& title)
{
    d->title = title;
}

QString KoChangeTrackerElement::getChangeTitle() const
{
    return d->title;
}

void KoChangeTrackerElement::setChangeFormat(const QTextFormat &format)
{
    d->changeFormat = format;
}

QTextFormat KoChangeTrackerElement::getChangeFormat() const
{
    return d->changeFormat;
}

void KoChangeTrackerElement::setPrevFormat(const QTextFormat &format)
{
    d->prevFormat = format;
}

QTextFormat KoChangeTrackerElement::getPrevFormat() const
{
    return d->prevFormat;
}

bool KoChangeTrackerElement::hasCreator() const
{
    return !d->creator.isEmpty();
}

void KoChangeTrackerElement::setCreator(const QString& creator)
{
    d->creator = creator;
}

QString KoChangeTrackerElement::getCreator() const
{
    return d->creator;
}

bool KoChangeTrackerElement::hasDate() const
{
    return !d->date.isEmpty();
}

void KoChangeTrackerElement::setDate(const QString& date)
{
    d->date = date;
}

QString KoChangeTrackerElement::getDate() const
{
    return d->date;
}

bool KoChangeTrackerElement::hasExtraMetaData() const
{
    return !d->extraMetaData.isEmpty();
}

void KoChangeTrackerElement::setExtraMetaData(const QString& metaData)
{
    d->extraMetaData = metaData;
}

QString KoChangeTrackerElement::getExtraMetaData() const
{
    return d->extraMetaData;
}

bool KoChangeTrackerElement::hasDeleteData() const
{
    return !d->deleteData.isEmpty();
}

void KoChangeTrackerElement::setDeleteData(const QString& data)
{
    d->deleteData = data;
}

QString KoChangeTrackerElement::getDeleteData() const
{
    return d->deleteData;
}

void KoChangeTrackerElement::setDeleteChangeMarker(KoDeleteChangeMarker *marker)
{
    d->marker = marker;
}

KoDeleteChangeMarker *KoChangeTrackerElement::getDeleteChangeMarker()
{
    return d->marker;
}

