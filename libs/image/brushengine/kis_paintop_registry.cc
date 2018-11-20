/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_paintop_registry.h"

#include <QGlobalStatic>

#include <klocalizedstring.h>

#include <KoGenericRegistry.h>
#include <KoPluginLoader.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoCompositeOp.h>
#include <KoID.h>


#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_debug.h"
#include "kis_layer.h"
#include "kis_image.h"
#include "kis_paintop_config_widget.h"

Q_GLOBAL_STATIC(KisPaintOpRegistry, s_registryInstance)

KisPaintOpRegistry::KisPaintOpRegistry()
{
}

KisPaintOpRegistry::~KisPaintOpRegistry()
{
    Q_FOREACH (const QString & id, keys()) {
        delete get(id);
    }
    dbgRegistry << "Deleting KisPaintOpRegistry";
}


void KisPaintOpRegistry::initRegistry()
{
    KoPluginLoader::instance()->load("Krita/Paintop", "(Type == 'Service') and ([X-Krita-Version] == 28)");

    QStringList toBeRemoved;

    Q_FOREACH (const QString & id, keys()) {
        KisPaintOpFactory *factory = get(id);
        if (!factory->settings()) {
            toBeRemoved << id;
        } else {
            factory->processAfterLoading();
        }
    }
    Q_FOREACH (const QString & id, toBeRemoved) {
        remove(id);
    }
}

KisPaintOpRegistry* KisPaintOpRegistry::instance()
{
    if (!s_registryInstance.exists()) {
        dbgRegistry << "initializing KisPaintOpRegistry";
        s_registryInstance->initRegistry();
    }
    return s_registryInstance;
}

#ifdef HAVE_THREADED_TEXT_RENDERING_WORKAROUND
void KisPaintOpRegistry::preinitializePaintOpIfNeeded(const KisPaintOpPresetSP preset)
{
    if (!preset) return;

    KisPaintOpFactory *f = value(preset->paintOp().id());
    f->preinitializePaintOpIfNeeded(preset->settings());
}
#endif /* HAVE_THREADED_TEXT_RENDERING_WORKAROUND */

KisPaintOp * KisPaintOpRegistry::paintOp(const QString & id, const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image) const
{
    if (painter == 0) {
        warnKrita << " KisPaintOpRegistry::paintOp painter is null";
        return 0;
    }

    Q_ASSERT(settings);

    KisPaintOpFactory* f = value(id);
    if (f) {
        KisPaintOp * op = f->createOp(settings, painter, node, image);
        if (op) {
            return op;
        }
    }
    warnKrita << "Could not create paintop for factory" << id << "with settings" << settings;
    return 0;
}

KisPaintOp * KisPaintOpRegistry::paintOp(const KisPaintOpPresetSP preset, KisPainter * painter, KisNodeSP node, KisImageSP image) const
{
    Q_ASSERT(preset);
    Q_ASSERT(painter);

    if (!preset) return 0;

    return paintOp(preset->paintOp().id(), preset->settings(), painter, node, image);
}

KisPaintOpSettingsSP KisPaintOpRegistry::createSettings(const KoID& id) const
{
    KisPaintOpFactory *f = value(id.id());
    Q_ASSERT(f);
    if (f) {
        KisPaintOpSettingsSP settings = f->settings();
        settings->setProperty("paintop", id.id());
        return settings;
    }
    return 0;
}

KisPaintOpPresetSP KisPaintOpRegistry::defaultPreset(const KoID& id) const
{
    KisPaintOpSettingsSP s = createSettings(id);
    if (s.isNull()) {
        return KisPaintOpPresetSP();
    }

    KisPaintOpPresetSP preset(new KisPaintOpPreset());
    preset->setName(i18n("default"));

    preset->setSettings(s);
    preset->setPaintOp(id);
    Q_ASSERT(!preset->paintOp().id().isEmpty());
    preset->setValid(true);
    return preset;
}

QIcon KisPaintOpRegistry::icon(const KoID &id) const
{
    KisPaintOpFactory* f = value(id.id());

    if (!f) {
        dbgRegistry << "No paintop" << id.id() << "";
        QPixmap p = QPixmap(22, 22);
        p.fill(Qt::transparent);
        return QIcon(p);
    }

    return f->icon();
}

QList<KoID> KisPaintOpRegistry::listKeys() const
{
    QList<KoID> answer;
    Q_FOREACH (const QString & key, keys()) {
        answer.append(KoID(key, get(key)->name()));
    }

    return answer;
}

