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
#include <QDirIterator>
#include <QJsonDocument>

#include <kis_icon.h>
#include <KoResourceServer.h>
#include <KisFolderStorage.h>
#include <KisResourceLocator.h>
#include <KoResourceServerProvider.h>
#include <KisResourceServerProvider.h>
#include <KisResourceLoaderRegistry.h>
#include <KisGlobalResourcesInterface.h>

#include "kis_my_paintop.h"
#include "kis_mypaint_brush.h"
#include "kis_my_paintop_option.h"
#include "kis_my_paintop_factory.h"
#include "kis_my_paintop_settings.h"
#include "kis_my_paintop_settings_widget.h"

class KisMyPaintOpFactory::Private {

    public:
        KoResourceServer<KisMyPaintBrush> *brushServer;
        QMap<QString, KisMyPaintBrush*> brushes;
};

KisMyPaintOpFactory::KisMyPaintOpFactory()
    : m_d(new Private)
{

    connect(KisResourceLocator::instance(), SIGNAL(initializationComplete()), this, SLOT(processAfterLoading()));
}

KisMyPaintOpFactory::~KisMyPaintOpFactory() {
}

KisPaintOp* KisMyPaintOpFactory::createOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP image) {

    KisPaintOp* op = new KisMyPaintOp(settings, painter, node, image);

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

void KisMyPaintOpFactory::processAfterLoading() {

    connect(this, SIGNAL(progressMessage(const QString&)), KisResourceLocator::instance(), SIGNAL(progressMessage(const QString&)));

    QStringList resourcesPaths = KoResourcePaths::findAllResources(ResourceType::MyPaintBrushes, "*.myb", KoResourcePaths::Recursive);
    KisPaintOpPresetResourceServer *paintOpServer = KisResourceServerProvider::instance()->paintOpPresetServer();

    emit progressMessage("Loading MyPaint Brushes");

    for(auto iter: resourcesPaths) {

        QSharedPointer<KisMyPaintBrush> brush(new KisMyPaintBrush(iter));

        if(!brush->load(KisGlobalResourcesInterface::instance())) {
            qDebug() << "Unable to load " << iter;
            continue;
        }

        QFileInfo fileInfo(brush->filename());

        KisPaintOpSettingsSP s = new KisMyPaintOpSettings(KisGlobalResourcesInterface::instance());
        s->setProperty("paintop", id());
        s->setProperty("filename", brush->filename());
        s->setProperty(MYPAINT_JSON, brush->getJsonData());
        s->setProperty(MYPAINT_DIAMETER, brush->getSize());
        s->setProperty(MYPAINT_HARDNESS, brush->getHardness());
        s->setProperty(MYPAINT_OPACITY, brush->getOpacity());
        s->setProperty(MYPAINT_ERASER, brush->isEraser());
        s->setProperty("EraserMode", qRound(brush->isEraser()));

        KisPaintOpPresetSP preset(new KisPaintOpPreset());
        preset->setName(fileInfo.baseName());
        preset->setSettings(s);
        KoID paintOpID(id(), name());
        preset->setPaintOp(paintOpID);        
        preset->setImage(brush->image());
        preset->setDirty(false);
        preset->setValid(true);        

        paintOpServer->addResource(preset, false);
    }
}
