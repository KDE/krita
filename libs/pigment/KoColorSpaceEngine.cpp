/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
