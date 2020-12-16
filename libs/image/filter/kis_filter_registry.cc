/*
 *  SPDX-FileCopyrightText: 2003 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "filter/kis_filter_registry.h"

#include <math.h>

#include <QString>
#include <QApplication>

#include <klocalizedstring.h>

#include <KoPluginLoader.h>

#include "kis_debug.h"
#include "kis_types.h"

#include "kis_paint_device.h"
#include "filter/kis_filter.h"
#include "kis_filter_configuration.h"

KisFilterRegistry::KisFilterRegistry(QObject *parent)
    : QObject(parent)
{
}

KisFilterRegistry::~KisFilterRegistry()
{
    dbgRegistry << "deleting KisFilterRegistry";
    Q_FOREACH (KisFilterSP filter, values()) {
        remove(filter->id());
        filter.clear();
    }
}

KisFilterRegistry* KisFilterRegistry::instance()
{
    KisFilterRegistry *reg = qApp->findChild<KisFilterRegistry *>(QString());
    if (!reg) {
        dbgRegistry << "initializing KisFilterRegistry";
        reg = new KisFilterRegistry(qApp);
        KoPluginLoader::instance()->load("Krita/Filter", "Type == 'Service' and ([X-Krita-Version] == 28)");
    }
    return reg;
}

void KisFilterRegistry::add(KisFilterSP item)
{
    add(item->id(), item);
}

void KisFilterRegistry::add(const QString &id, KisFilterSP item)
{
    KoGenericRegistry<KisFilterSP>::add(id, item);
    emit(filterAdded(id));
}


