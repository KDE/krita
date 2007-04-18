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
#include <QPixmap>
#include <QWidget>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kparts/plugin.h>
#include <kservice.h>
#include <kparts/componentfactory.h>
#include <kservicetypetrader.h>

#include <KoGenericRegistry.h>
#include <KoPluginLoader.h>

#include "kis_types.h"
#include "kis_paintop_registry.h"
#include "kis_paintop.h"
#include "KoID.h"
#include "kis_debug_areas.h"
#include "KoColorSpace.h"

KisPaintOpRegistry * KisPaintOpRegistry::m_singleton = 0;

KisPaintOpRegistry::KisPaintOpRegistry()
{
    Q_ASSERT(KisPaintOpRegistry::m_singleton == 0);
}

KisPaintOpRegistry::~KisPaintOpRegistry()
{
}

KisPaintOpRegistry* KisPaintOpRegistry::instance()
{
    if(KisPaintOpRegistry::m_singleton == 0)
    {
        KisPaintOpRegistry::m_singleton = new KisPaintOpRegistry();
        KoPluginLoader::instance()->load("Krita/Paintop", "(Type == 'Service') and ([X-Krita-Version] == 3)");
        Q_CHECK_PTR(KisPaintOpRegistry::m_singleton);
    }
    return KisPaintOpRegistry::m_singleton;
}

KisPaintOp * KisPaintOpRegistry::paintOp(const KoID & id, const KisPaintOpSettings * settings, KisPainter * painter) const
{
    return paintOp(id.id(), settings, painter);
}

KisPaintOp * KisPaintOpRegistry::paintOp(const QString & id, const KisPaintOpSettings * settings, KisPainter * painter) const
{
    if (painter == 0) {
        kWarning() << " KisPaintOpRegistry::paintOp painter is null";
        return 0;
    }
    KisPaintOpFactorySP f = value(id);
   if (f)
        return f->createOp(settings, painter);
   return 0;
}

KisPaintOpSettings * KisPaintOpRegistry::settings(const KoID& id, QWidget * parent, const KoInputDevice& inputDevice) const
{
    KisPaintOpFactorySP f = value(id.id());
    if (f)
        return f->settings( parent, inputDevice );

    return 0;
}

bool KisPaintOpRegistry::userVisible(const KoID & id, KoColorSpace* cs) const
{

    KisPaintOpFactorySP f = value(id.id());
    if (!f) {
        kDebug(DBG_AREA_REGISTRY) << "No paintop " << id.id() << "\n";
        return false;
    }
    return f->userVisible(cs);

}

QString KisPaintOpRegistry::pixmap(const KoID & id) const
{
    KisPaintOpFactorySP f = value(id.id());

    if (!f) {
        kDebug(DBG_AREA_REGISTRY) << "No paintop " << id.id() << "\n";
        return "";
    }

    return f->pixmap();
}

#include "kis_paintop_registry.moc"
