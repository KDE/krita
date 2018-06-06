/*
 * Copyright (C) 2018 Boudewijn Rempt <boud@valdyas.org>
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

#include "KisResourceLoader.h"

class KisResourceLoader::Private
{
public:
    QString folder;
    QString type;
    QString name;
    QImage thumbnail {QString("UNKNOWN")};
    QStringList mimetypes;
    KoResource *resource {0};
};


KisResourceLoader::KisResourceLoader(const QString &folder, const QStringList &mimetypes)
    : d(new Private())
{
    d->folder = folder;
    d->mimetypes = mimetypes;
}

KisResourceLoader::~KisResourceLoader()
{
}

QStringList KisResourceLoader::mimetypes() const
{
    return d->mimetypes;
}

QString KisResourceLoader::folder() const
{
    return d->folder;
}

KoResource *KisResourceLoader::resource() const
{
    return d->resource;
}

void KisResourceLoader::setType(const QString &type)
{
    d->type = type;
}

void KisResourceLoader::setThumbnail(const QImage &thumbnail)
{
    d->thumbnail = thumbnail;
}

void KisResourceLoader::setName(const QString &name)
{
    d->name = name;
}

void KisResourceLoader::setResource(KoResource *resource)
{
    d->resource = resource;
}
