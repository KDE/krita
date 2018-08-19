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
#include "kis_brush_registry.h"

#include <QString>

#include <QGlobalStatic>
#include <klocalizedstring.h>

#include <KoPluginLoader.h>

#include <kis_debug.h>

#include "kis_brush_server.h"
#include "kis_auto_brush_factory.h"
#include "kis_text_brush_factory.h"
#include "kis_predefined_brush_factory.h"

Q_GLOBAL_STATIC(KisBrushRegistry, s_instance)


KisBrushRegistry::KisBrushRegistry()
{
    KisBrushServer::instance();
}

KisBrushRegistry::~KisBrushRegistry()
{
    Q_FOREACH (const QString & id, keys()) {
        delete get(id);
    }
    dbgRegistry << "deleting KisBrushRegistry";
}

KisBrushRegistry* KisBrushRegistry::instance()
{
    if (!s_instance.exists()) {
        s_instance->add(new KisAutoBrushFactory());
        s_instance->add(new KisPredefinedBrushFactory("gbr_brush"));
        s_instance->add(new KisPredefinedBrushFactory("abr_brush"));
        s_instance->add(new KisTextBrushFactory());
        s_instance->add(new KisPredefinedBrushFactory("png_brush"));
        s_instance->add(new KisPredefinedBrushFactory("svg_brush"));
    }
    return s_instance;
}


KisBrushSP KisBrushRegistry::createBrush(const QDomElement& element)
{
    QString brushType = element.attribute("type");

    if (brushType.isEmpty()) return 0;

    KisBrushFactory* factory = get(brushType);
    if (!factory) return 0;

    return factory->createBrush(element);
}

