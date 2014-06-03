/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "generator/kis_generator_registry.h"

#include <math.h>

#include <QString>
#include <QApplication>

#include <kglobal.h>
#include <kaction.h>
#include <klocale.h>
#include <kservice.h>

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
    foreach(KisGeneratorSP generator, values()) {
        remove(generator->id());
        generator.clear();
    }
    dbgRegistry << "deleting KisGeneratorRegistry";
}

KisGeneratorRegistry* KisGeneratorRegistry::instance()
{
    KisGeneratorRegistry *reg = qApp->findChild<KisGeneratorRegistry *>("");
    if (!reg) {
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

KisFilterConfiguration* KisGeneratorRegistry::cloneConfiguration(KisFilterConfiguration* kfc)
{
    KisGeneratorSP filter = value(kfc->name());
    KisFilterConfiguration* newkfc = filter->defaultConfiguration(0);
    newkfc->fromXML(kfc->toXML());
    return newkfc;
}

#include "kis_generator_registry.moc"
