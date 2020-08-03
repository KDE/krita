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

    m_d->brushServer = new KoResourceServerSimpleConstruction<KisMyPaintBrush>("mypaint_brushes", "*.myb");
    m_d->brushServer->loadResources(KoResourceServerProvider::blacklistFileNames(m_d->brushServer->fileNames(), m_d->brushServer->blackListedFiles()));
}

KisMyPaintOpFactory::~KisMyPaintOpFactory() {
}

KisPaintOp* KisMyPaintOpFactory::createOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP image) {

    KisPaintOp* op = new KisMyPaintOp(settings, painter, node, image);

    Q_CHECK_PTR(op);
    return op;
}

KisPaintOpSettingsSP KisMyPaintOpFactory::settings() {

    KisPaintOpSettingsSP settings = new KisMyPaintOpSettings();
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

    KisPaintOpPresetResourceServer *paintOpServer = KisResourceServerProvider::instance()->paintOpPresetServer();

    foreach(KisMyPaintBrush* brush, m_d->brushServer->resources()) {

        QFileInfo fileInfo(brush->filename());

        KisPaintOpSettingsSP s = new KisMyPaintOpSettings();
        s->setProperty("paintop", id());
        s->setProperty("filename", brush->filename());
        s->setProperty(MYPAINT_JSON, brush->getJsonData());
        s->setProperty(MYPAINT_DIAMETER, brush->getSize());
        s->setProperty(MYPAINT_HARDNESS, brush->getHardness());
        s->setProperty(MYPAINT_OPACITY, brush->getOpacity());
        s->setProperty(MYPAINT_ERASER, brush->isEraser());
        s->setProperty("EraserMode", qRound(brush->isEraser()));

        KisPaintOpPresetSP preset = new KisPaintOpPreset();
        preset->setName(fileInfo.baseName());
        preset->setSettings(s);
        KoID paintOpID(id(), name());
        preset->setPaintOp(paintOpID);        
        preset->setImage(brush->image());        
        preset->setValid(true);        

        paintOpServer->addResource(preset, false);
    }
}

#include "kis_my_paintop_factory.moc"
