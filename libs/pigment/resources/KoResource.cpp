/*  This file is part of the KDE project
    Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
    Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
    Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

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
#include <QFileInfo>
#include <QDebug>
#include <QImage>

#include "KoHashGenerator.h"
#include "KoHashGeneratorProvider.h"

struct Q_DECL_HIDDEN KoResource::Private {
    QString name;
    QString filename;
    bool valid;
    bool removable;
    QByteArray md5;
    QImage image;
};

KoResource::KoResource(const QString& filename)
    : d(new Private)
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

KoResource::KoResource(const KoResource &rhs)
    : d(new Private(*rhs.d))
{
}

bool KoResource::saveToDevice(QIODevice *dev) const
{
    Q_UNUSED(dev)
    d->md5 = QByteArray();

    return true;
}

QImage KoResource::image() const
{
    return d->image;
}

void KoResource::setImage(const QImage &image)
{
    d->image = image;
}

QByteArray KoResource::md5() const
{
    if (d->md5.isEmpty()) {
        const_cast<KoResource*>(this)->setMD5(generateMD5());
    }
    return d->md5;
}

void KoResource::setMD5(const QByteArray &md5)
{
    d->md5 = md5;
}

QByteArray KoResource::generateMD5() const
{
    KoHashGenerator *hashGenerator = KoHashGeneratorProvider::instance()->getGenerator("MD5");
    return hashGenerator->generateHash(d->filename);
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

bool KoResource::removable() const
{
    return d->removable;
}

QString KoResource::defaultFileExtension() const
{
    return QString();
}

