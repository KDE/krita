/*
 * defaultpaintops_plugin.cc -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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

#include "defaultpaintops_plugin.h"
#include <klocale.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kgenericfactory.h>

#include "kis_simple_paintop_factory.h"
#include "kis_airbrushop.h"
#include "kis_airbrushop_settings.h"
#include "kis_airbrushop_settings_widget.h"
#include "kis_brushop.h"
#include "kis_brushop_settings.h"
#include "kis_brushop_settings_widget.h"
#include "kis_duplicateop_factory.h"
#include "kis_eraseop.h"
#include "kis_eraseop_settings.h"
#include "kis_penop.h"
#include "kis_penop_settings.h"
#include "kis_smudgeop.h"
#include "kis_smudgeop_settings.h"
#include "kis_global.h"
#include "kis_paintop_registry.h"

typedef KGenericFactory<DefaultPaintOpsPlugin> DefaultPaintOpsPluginFactory;
K_EXPORT_COMPONENT_FACTORY(kritadefaultpaintops, DefaultPaintOpsPluginFactory("krita"))


DefaultPaintOpsPlugin::DefaultPaintOpsPlugin(QObject *parent, const QStringList &)
        : QObject(parent)
{
    //setComponentData(DefaultPaintOpsPluginFactory::componentData());

    KisPaintOpRegistry *r = KisPaintOpRegistry::instance();
    r->add(new KisSimplePaintOpFactory<KisAirbrushOp, KisAirbrushOpSettings, KisAirbrushOpSettingsWidget>("airbrush", i18n("Pixel Airbrush"), "krita-airbrush.png"));
    r->add(new KisSimplePaintOpFactory<KisBrushOp, KisBrushOpSettings, KisBrushOpSettingsWidget>("paintbrush", i18n("Pixel Brush"), "krita-paintbrush.png"));
    r->add(new KisDuplicateOpFactory);
    r->add(new KisSimplePaintOpFactory<KisEraseOp, KisEraseOpSettings, KisEraseOpSettingsWidget>("eraser", i18n("Pixel Eraser"), "krita-eraser.png"));
    r->add(new KisSimplePaintOpFactory<KisPenOp, KisPenOpSettings, KisPenOpSettingsWidget>("pencil", "Pixel Pencil", "krita-pencil.png"));
    r->add(new KisSimplePaintOpFactory<KisSmudgeOp, KisSmudgeOpSettings, KisSmudgeOpSettingsWidget>("smudge", i18n("Smudge Brush"), "krita-smudgebrush.png"));
}

DefaultPaintOpsPlugin::~DefaultPaintOpsPlugin()
{
}

#include "defaultpaintops_plugin.moc"
