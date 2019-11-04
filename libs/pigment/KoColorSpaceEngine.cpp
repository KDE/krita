/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <KoColorSpaceEngine.h>
#include <QGlobalStatic>
#include <QString>


Q_GLOBAL_STATIC(KoColorSpaceEngineRegistry, s_instance)

struct Q_DECL_HIDDEN KoColorSpaceEngine::Private {
    QString id;
    QString name;
};

KoColorSpaceEngine::KoColorSpaceEngine(const QString& id, const QString& name) : d(new Private)
{
    d->id = id;
    d->name = name;
}

KoColorSpaceEngine::~KoColorSpaceEngine()
{
    delete d;
}

const QString& KoColorSpaceEngine::id() const
{
    return d->id;
}

const QString& KoColorSpaceEngine::name() const
{
    return d->name;
}

bool KoColorSpaceEngine::supportsColorSpace(const QString &colorModelId, const QString &colorDepthId, const KoColorProfile *profile) const
{
    Q_UNUSED(colorModelId);
    Q_UNUSED(colorDepthId);
    Q_UNUSED(profile);

    return true;
}

KoColorSpaceEngineRegistry::KoColorSpaceEngineRegistry()
{
}

KoColorSpaceEngineRegistry::~KoColorSpaceEngineRegistry()
{
    Q_FOREACH (KoColorSpaceEngine* item, values()) {
        delete item;
    }
}

KoColorSpaceEngineRegistry* KoColorSpaceEngineRegistry::instance()
{
    return s_instance;
}
