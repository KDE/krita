/*
 *  Copyright (C) 2009 Frederik Gladhorn <gladhorn@kde.org>
 *  Copyright (c) 2017 Aniketh Girish anikethgireesh@gmail.com
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

#include "entry.h"
#include "entry_p.h"

#include <QtCore/QStringList>

using namespace KNS3;

Entry::Entry()
    : d(new EntryPrivate)
{
}

Entry::Entry(const Entry &other)
    : d(other.d)
{
}

Entry &Entry::operator=(const Entry &other)
{
    d = other.d;
    return *this;
}

Entry::~Entry()
{
}

QString Entry::id() const
{
    return d->e.uniqueId();
}

QString Entry::providerId() const
{
    return d->e.providerId();
}

QString Entry::name() const
{
    return d->e.name();
}

QString Entry::category() const
{
    return d->e.category();
}

QString Entry::license() const
{
    return d->e.license();
}

QString Entry::summary() const
{
    return d->e.summary();
}

QString Entry::version() const
{
    return d->e.version();
}

Entry::Status Entry::status() const
{
    return static_cast<Entry::Status>(d->e.status());
}

QStringList Entry::installedFiles() const
{
    return d->e.installedFiles();
}

QStringList Entry::uninstalledFiles() const
{
    return d->e.uninstalledFiles();
}

QUrl Entry::url() const
{
    return d->e.homepage();
}

static void appendIfValid(QList<QUrl>& list, const QUrl &value)
{
    if (value.isValid() && !value.isEmpty())
        list << value;
}

QList<QUrl> Entry::previewImages() const
{
    QList<QUrl> ret;
    appendIfValid(ret, QUrl(d->e.previewUrl(KNSCore::EntryInternal::PreviewBig1)));
    appendIfValid(ret, QUrl(d->e.previewUrl(KNSCore::EntryInternal::PreviewBig2)));
    appendIfValid(ret, QUrl(d->e.previewUrl(KNSCore::EntryInternal::PreviewBig3)));
    return ret;
}

QList<QUrl> Entry::previewThumbnails() const
{
    QList<QUrl> ret;
    appendIfValid(ret, QUrl(d->e.previewUrl(KNSCore::EntryInternal::PreviewSmall1)));
    appendIfValid(ret, QUrl(d->e.previewUrl(KNSCore::EntryInternal::PreviewSmall2)));
    appendIfValid(ret, QUrl(d->e.previewUrl(KNSCore::EntryInternal::PreviewSmall3)));
    return ret;
}

quint64 Entry::size() const
{
    const auto downloadInfo = d->e.downloadLinkInformationList();
    return downloadInfo.isEmpty() ? 0 : downloadInfo.at(0).size;
}

uint Entry::numberOfComments() const
{
    return d->e.numberOfComments();
}

uint Entry::rating() const
{
    return d->e.rating();
}

QString Entry::updateVersion() const
{
    return d->e.updateVersion();
}

QString Entry::changelog() const
{
    return d->e.changelog();
}

QString Entry::shortSummary() const
{
    return d->e.shortSummary();
}
