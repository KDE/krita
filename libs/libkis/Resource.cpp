/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

struct Resource::Private {
    Private(KoResourceSP _resource)
        : resource(_resource)
    {}

    KoResourceSP resource {0};
};

Resource::Resource(KoResourceSP resource, QObject *parent)
    : QObject(parent)
    , d(new Private(resource))
{
}

Resource::~Resource()
{
    delete d;
}

bool Resource::operator==(const Resource &other) const
{
    return (d->resource == other.d->resource);
}

bool Resource::operator!=(const Resource &other) const
{
    return !(operator==(other));
}

QString Resource::type() const
{
    if (!d->resource) return QString();
    if (d->resource.dynamicCast<KoPattern>()) return "pattern";
    else if (d->resource.dynamicCast<KoAbstractGradient>()) return "gradient";
    else if (d->resource.dynamicCast<KisBrush>()) return "brush";
    else if (d->resource.dynamicCast<KisPaintOpPreset>()) return "preset";
    else if (d->resource.dynamicCast<KoColorSet>()) return "palette";
    else if (d->resource.dynamicCast<KisWorkspaceResource>()) return "workspace";
    else return "";
}

QString Resource::name() const
{
    if (!d->resource) return QString();
    return d->resource->name();
}

void Resource::setName(QString value)
{
    if (!d->resource) return;
    d->resource->setName(value);
}


QString Resource::filename() const
{
    if (!d->resource) return QString();
    return d->resource->filename();
}


QImage Resource::image() const
{
    if (!d->resource) return QImage();
    return d->resource->image();
}

void Resource::setImage(QImage image)
{
    if (!d->resource) return;
    d->resource->setImage(image);
}

QByteArray Resource::data() const
{
    QByteArray ba;

    if (!d->resource) return ba;

    QBuffer buf(&ba);
    d->resource->saveToDevice(&buf);
    return ba;
}

bool Resource::setData(QByteArray data)
{
    if (!d->resource) return false;
    QBuffer buf(&data);
    return d->resource->loadFromDevice(&buf);
}

KoResourceSP Resource::resource() const
{
    return d->resource;
}






