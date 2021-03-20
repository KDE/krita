/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "Resource.h"
#include <KoResource.h>
#include <QByteArray>
#include <QBuffer>

#include <KoPattern.h>
#include <KoAbstractGradient.h>
#include <kis_brush.h>
#include <kis_paintop_preset.h>
#include <KoColorSet.h>
#include <kis_workspace_resource.h>
#include <KisResourceLocator.h>

struct Resource::Private {
    Private() {}

    int id {-1};
    QString type;
    QString name;
    QString filename;
    QImage image;
};

Resource::Resource(int resourceId, const QString &type, const QString &name, const QString &filename, const QImage &image, QObject *parent)
    : QObject(parent)
    , d(new Private())
{
    d->id = resourceId;
    d->type = type;
    d->name = name;
    d->filename = filename;
    d->image = image;
}

Resource::Resource(KoResourceSP resource, const QString &type, QObject *parent)
    : QObject(parent)
    , d(new Private())
{
    d->id = resource->resourceId();
    d->type = type;
    d->name = resource->name();
    d->filename = resource->filename();
    d->image = resource->image();
}

Resource::~Resource()
{
}

Resource::Resource(const Resource &rhs)
    : QObject()
    , d(new Private())
{
    d->id = rhs.d->id;
    d->type = rhs.d->type;
    d->name = rhs.d->name;
    d->filename = rhs.d->filename;
    d->image = rhs.d->image;
}

bool Resource::operator==(const Resource &other) const
{
    return (d->id == other.d->id);
}

bool Resource::operator!=(const Resource &other) const
{
    return !(operator==(other));
}

Resource Resource::operator=(const Resource &rhs)
{
    Resource res(rhs.d->id,
                 rhs.d->type,
                 rhs.d->name,
                 rhs.d->filename,
                 rhs.d->image);
    return res;
}

QString Resource::type() const
{
    return d->type;
}

QString Resource::name() const
{
    return d->name;
}

void Resource::setName(QString value)
{
    d->name = value;
}

QString Resource::filename() const
{
    return d->filename;
}


QImage Resource::image() const
{
    return d->image;
}

void Resource::setImage(QImage image)
{
    d->image = image;
}

KoResourceSP Resource::resource() const
{
    KoResourceSP res = KisResourceLocator::instance()->resourceForId(d->id);
    Q_ASSERT(res);
    return res;
}






