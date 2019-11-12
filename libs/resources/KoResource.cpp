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
#include <KoResource.h>

#include <QDomElement>
#include <QFileInfo>
#include <QDebug>
#include <QImage>
#include <QBuffer>

#include "KoHashGenerator.h"
#include "KoHashGeneratorProvider.h"

struct Q_DECL_HIDDEN KoResource::Private {
    QString name;
    QString filename;
    bool valid {false};
    QByteArray md5;
    QImage image;
    bool permanent {false};
    int resourceId {-1};
    QString storageLocation;
    bool dirty;
    QMap<QString, QVariant> metadata;
    int version {0};
};

KoResource::KoResource(const QString& filename)
    : d(new Private)
{
    d->filename = filename;
}

KoResource::~KoResource()
{
    delete d;
}

KoResource::KoResource(const KoResource &rhs)
    : d(new Private)
{
    *this = rhs;
}

KoResource &KoResource::operator=(const KoResource &rhs)
{
    if (this != &rhs) {
        d->name = rhs.d->name;
        d->filename= rhs.d->filename;
        d->valid = rhs.d->valid;
        d->md5 = rhs.d->md5;
        d->image = rhs.d->image;
        d->permanent = rhs.d->permanent;
        d->resourceId = rhs.d->resourceId;
        d->storageLocation = rhs.d->storageLocation;
        d->dirty = rhs.d->dirty;
        d->metadata = rhs.d->metadata;
        d->version = rhs.d->version;
    }
    return *this;
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
    QByteArray hash;
    QByteArray ba;
    QBuffer buf(&ba);
    buf.open(QBuffer::WriteOnly);
    if (saveToDevice(&buf)) {
        buf.close();
        hash = hashGenerator->generateHash(ba);
    }
    else {
        qWarning() << "Could not create md5sum for resource" << d->filename;
    }
    return hash;
}

QString KoResource::filename() const
{
    return d->filename;
}

void KoResource::setFilename(const QString& filename)
{
    d->filename = filename;
}

QString KoResource::name() const
{
    return (!d->name.isEmpty() ? d->name : filename());
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


QString KoResource::defaultFileExtension() const
{
    return QString();
}

bool KoResource::permanent() const
{
    return d->permanent;
}

void KoResource::setPermanent(bool permanent)
{
    d->permanent = permanent;
}

int KoResource::resourceId() const
{
    return d->resourceId;
}

QString KoResource::storageLocation() const
{
    return d->storageLocation;
}

void KoResource::setDirty(bool value)
{
    d->dirty = value;
}

bool KoResource::isDirty() const
{
    return d->dirty;
}

void KoResource::addMetaData(QString key, QVariant value)
{
    d->metadata.insert(key, value);
}

QMap<QString, QVariant> KoResource::metadata() const
{
    return d->metadata;
}

int KoResource::version() const
{
    return d->version;
}

void KoResource::setVersion(int version)
{
    d->version = version;
}

void KoResource::setResourceId(int id)
{
    d->resourceId = id;
}

void KoResource::setStorageLocation(const QString &location)
{
    d->storageLocation = location;
}

