/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "generator/kis_generator_registry.h"

#include <math.h>

#include <QString>
#include <QApplication>

#include <klocalizedstring.h>

#include <KoPluginLoader.h>

#include "filter/kis_filter_configuration.h"
#include "kis_debug.h"
#include "kis_types.h"
#include "kis_paint_device.h"
#include "generator/kis_generator.h"

KisGeneratorRegistry::KisGeneratorRegistry(QObject *parent)
    : QObject(parent)
{
}

KisGeneratorRegistry::~KisGeneratorRegistry()
{
    Q_FOREACH (KisGeneratorSP generator, values()) {
        remove(generator->id());
        generator.clear();
    }
    dbgRegistry << "deleting KisGeneratorRegistry";
}

KisGeneratorRegistry* KisGeneratorRegistry::instance()
{
    KisGeneratorRegistry *reg = qApp->findChild<KisGeneratorRegistry *>(QString());
    if (!reg) {
        dbgRegistry << "initializing KisGeneratorRegistry";
        reg = new KisGeneratorRegistry(qApp);
        KoPluginLoader::instance()->load("Krita/Generator", "Type == 'Service' and ([X-Krita-Version] == 28)");
    }
    return reg;
}

void KisGeneratorRegistry::add(KisGeneratorSP item)
{
    dbgPlugins << "adding " << item->name();
    add(item->id(), item);
}

void KisGeneratorRegistry::add(const QString &id, KisGeneratorSP item)
{
    dbgPlugins << "adding " << item->name() << " with id " << id;
    KoGenericRegistry<KisGeneratorSP>::add(id, item);
    emit(generatorAdded(id));
}

