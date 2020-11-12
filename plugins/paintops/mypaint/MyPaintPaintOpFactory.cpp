/*
 * Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.com>
 * Copyright (c) 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <qmath.h>
#include <QJsonObject>
#include <QJsonDocument>

#include <kis_icon.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>
#include <KisResourceServerProvider.h>

#include "MyPaintPaintOp.h"
#include "MyPaintPaintOpPreset.h"
#include "MyPaintPaintOpOption.h"
#include "MyPaintPaintOpFactory.h"
#include "MyPaintPaintOpSettings.h"
#include "MyPaintPaintOpSettingsWidget.h"



class KisMyPaintOpFactory::Private {
};

KisMyPaintOpFactory::KisMyPaintOpFactory()
    : m_d(new Private)
{
}

KisMyPaintOpFactory::~KisMyPaintOpFactory() {

    delete m_d;
}

KisPaintOp* KisMyPaintOpFactory::createOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP image) {

    KisPaintOp* op = new KisMyPaintPaintOp(settings, painter, node, image);
    Q_CHECK_PTR(op);
    return op;
}

KisPaintOpSettingsSP KisMyPaintOpFactory::createSettings(KisResourcesInterfaceSP resourcesInterface) {

    KisPaintOpSettingsSP settings = new KisMyPaintOpSettings(resourcesInterface);
    return settings;
}

KisPaintOpConfigWidget* KisMyPaintOpFactory::createConfigWidget(QWidget* parent) {

    return new KisMyPaintOpSettingsWidget(parent);
}

QString KisMyPaintOpFactory::id() const {

    return "mypaintbrush";
}

QString KisMyPaintOpFactory::name() const {

    return "MyPaint";
}

QIcon KisMyPaintOpFactory::icon() {

    return KisIconUtils::loadIcon(id());
}

QString KisMyPaintOpFactory::category() const {

    return KisPaintOpFactory::categoryStable();
}

QList<KoResourceSP> KisMyPaintOpFactory::prepareLinkedResources(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface)
{
    Q_UNUSED(settings)
    Q_UNUSED(resourcesInterface);

    return {};
}

QList<KoResourceSP> KisMyPaintOpFactory::prepareEmbeddedResources(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface)
{
    Q_UNUSED(settings)
    Q_UNUSED(resourcesInterface);

    return {};
}
#if 0
void KisMyPaintOpFactory::processAfterLoading() {

    KisPaintOpPresetResourceServer *paintOpServer = KisResourceServerProvider::instance()->paintOpPresetServer();

    Q_FOREACH(KisMyPaintBrush* brush, m_d->brushServer->resources()) {

        QFileInfo fileInfo(brush->filename());

        if(!brush->valid())
            continue;        

        KisPaintOpSettingsSP s = new KisMyPaintOpSettings();
        s->setProperty("paintop", id());
        s->setProperty("filename", brush->filename());
        s->setProperty(MYPAINT_JSON, brush->getJsonData());
        s->setProperty(MYPAINT_DIAMETER, brush->getSize());
        s->setProperty(MYPAINT_HARDNESS, brush->getHardness());
        s->setProperty(MYPAINT_OPACITY, brush->getOpacity());
        s->setProperty(MYPAINT_OFFSET_BY_RANDOM, brush->getOffset());
        s->setProperty(MYPAINT_ERASER, brush->isEraser());
        s->setProperty("EraserMode", qRound(brush->isEraser()));

        KisSharedPtr<KisMyPaintBrush> preset = new KisMyPaintBrush();
        preset->setName(fileInfo.baseName());
        preset->setSettings(s);
        KoID paintOpID(id(), name());
        preset->setPaintOp(paintOpID);
        preset->setImage(brush->image());
        preset->setValid(true);
        preset->setFilename(brush->filename());
        preset->load();

        paintOpServer->addResource(preset, false);
    }

}
#endif

#include "MyPaintPaintOpFactory.moc"
