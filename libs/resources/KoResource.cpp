/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2003 Patrick Julien <freak@codepimps.org>
    SPDX-FileCopyrightText: 2005 Boudewijn Rempt <boud@valdyas.org>
    SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>

    SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include <KoResource.h>

#include <QDomElement>
#include <QFileInfo>
#include <QDebug>
#include <QImage>
#include <QBuffer>

#include <kis_debug.h>
#include "KoMD5Generator.h"


struct KoResourceSPStaticRegistrar {
    KoResourceSPStaticRegistrar() {
        qRegisterMetaType<KoResourceSP>("KoResourceSP");
    }
};
static KoResourceSPStaticRegistrar __registrar1;


struct Q_DECL_HIDDEN KoResource::Private {
    int version {-1};
    int resourceId {-1};
    bool valid {false};
    bool permanent {false};
    bool dirty {false};
    QString name;
    QString filename;
    QString storageLocation;
    QByteArray md5;
    QImage image;
    QMap<QString, QVariant> metadata;
};

KoResource::KoResource()
    : d(new Private)
{
}

KoResource::KoResource(const QString& filename)
    : d(new Private)
{
    d->filename = filename;
    d->name = QFileInfo(filename).fileName();
}

KoResource::~KoResource()
{
    delete d;
}

KoResource::KoResource(const KoResource &rhs)
    : d(new Private(*rhs.d))
{
}



bool KoResource::load(KisResourcesInterfaceSP resourcesInterface)
{
    QFile file(filename());

    if (!file.exists()) {
        warnKrita << "File doesn't exist: " << filename();
        return false;
    }

    if (file.size() == 0) {
        warnKrita << "File is empty: " << filename();
        return false;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        warnKrita << "Can't open file for reading" << filename();
        return false;
    }

    const bool res = loadFromDevice(&file, resourcesInterface);
    file.close();

    return res;
}

bool KoResource::save()
{
    if (filename().isEmpty()) return false;

    QFile file(filename());

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        warnKrita << "Can't open file for writing" << filename();
        return false;
    }

    saveToDevice(&file);

    file.close();
    return true;
}

bool KoResource::saveToDevice(QIODevice *dev) const
{
    Q_UNUSED(dev);
    return true;
}

QImage KoResource::image() const
{
    return d->image;
}

void KoResource::updateThumbnail()
{
}

QImage KoResource::thumbnail() const
{
    return image();
}

QString KoResource::thumbnailPath() const
{
    return QString();
}

void KoResource::setImage(const QImage &image)
{
    d->image = image;
}

QByteArray KoResource::md5() const
{
    if (d->md5.isEmpty()) {
        dbgResources << "No MD5 for" << this << this->name();
        QByteArray ba;
        QBuffer buf(&ba);
        buf.open(QFile::WriteOnly);
        saveToDevice(&buf);
        buf.close();
        const_cast<KoResource*>(this)->setMD5(KoMD5Generator::generateHash(ba));
    }
    return d->md5;
}

void KoResource::setMD5(const QByteArray &md5)
{
    Q_ASSERT(!md5.isEmpty());
    d->md5 = md5;
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

QList<KoResourceSP> KoResource::requiredResources(KisResourcesInterfaceSP globalResourcesInterface) const
{
    return linkedResources(globalResourcesInterface) + embeddedResources(globalResourcesInterface);
}

QList<KoResourceSP> KoResource::linkedResources(KisResourcesInterfaceSP globalResourcesInterface) const
{
    Q_UNUSED(globalResourcesInterface);
    return {};
}

QList<KoResourceSP> KoResource::embeddedResources(KisResourcesInterfaceSP globalResourcesInterface) const
{
    Q_UNUSED(globalResourcesInterface);
    return {};
}

QList<int> KoResource::requiredCanvasResources() const
{
    return {};
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
