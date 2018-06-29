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

#include "KisTagLoader.h"

#include <QIODevice>
#include <QLocale>

#include "kconfigini_p.h"
#include "kconfigbackend_p.h"
#include "kconfigdata.h"

#include <kis_debug.h>

class KisTagLoader::Private {
public:
    QString url;
    QString name;
    QString comment;
};

KisTagLoader::KisTagLoader()
    : d(new Private)
{
}

KisTagLoader::~KisTagLoader()
{

}

QString KisTagLoader::name() const
{
    return d->name;
}

void KisTagLoader::setName(QString &name) const
{
    d->name = name;
}

QString KisTagLoader::url() const
{
    return d->url;
}

void KisTagLoader::setUrl(const QString &url) const
{
    d->url = url;
}

QString KisTagLoader::comment() const
{
    return d->comment;
}

void KisTagLoader::setComment(const QString &comment) const
{
    d->comment = comment;
}

bool KisTagLoader::load(QIODevice &io)
{
    if (!io.isOpen()) {
        io.open(QIODevice::ReadOnly);
    }
    KIS_ASSERT(io.isOpen());

    KEntryMap map;
    KConfigIniBackend ini;
    KConfigBackend::ParseInfo r = ini.parseConfigIO(io, QLocale().name().toUtf8(), map, KConfigBackend::ParseOption::ParseGlobal, false);
    if (!r == KConfigBackend::ParseInfo::ParseOk) {
        io.reset();
        qWarning() << "Could not load this tag file" << QString::fromUtf8(io.readAll());
        return false;
    }

    QString type = map.getEntry("Desktop Entry", "Type");
    if (type != "Tag") return false;

    d->url = map.getEntry("Desktop Entry", "URL");
    d->name = map.getEntry("Desktop Entry", "Name");
    d->comment = map.getEntry("Desktop Entry", "Comment");

    return true;
}

bool KisTagLoader::save(QIODevice &io)
{
    return false;
}

