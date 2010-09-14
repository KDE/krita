/*  This file is part of the KDE project
    Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
    Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
    Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "KoResource.h"

#include <QDomElement>
#include <QtCore/QFileInfo>

struct KoResource::Private {
    QString name;
    QString filename;
    bool valid;
    bool removable;
};

KoResource::KoResource(const QString& filename) : d(new Private)
{
    d->filename = filename;
    d->valid = false;
    QFileInfo fileInfo(filename);
    d->removable = fileInfo.isWritable();
}

KoResource::~KoResource()
{
    delete d;
}

QString KoResource::filename() const
{
    return d->filename;
}

void KoResource::setFilename(const QString& filename)
{
    d->filename = filename;
    QFileInfo fileInfo(filename);
    d->removable = ! fileInfo.exists() || fileInfo.isWritable();
}

QString KoResource::shortFilename() const
{
    QFileInfo fileInfo(d->filename);
    return fileInfo.fileName();
}

QString KoResource::name() const
{
    return d->name;
}

void KoResource::setName(const QString& name)
{
    d->name = name;
}

bool KoResource::valid() const
{
    return d->valid;
}

void KoResource::setValid(bool valid)
{
    d->valid = valid;
}

void KoResource::toXML(QDomDocument& , QDomElement& e) const
{
    e.setAttribute("name", name());
    e.setAttribute("filename", filename());
}

bool KoResource::removable() const
{
    return d->removable;
}

QString KoResource::defaultFileExtension() const
{
    return QString();
}

