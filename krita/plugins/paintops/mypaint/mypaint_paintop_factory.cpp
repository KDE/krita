/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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
#include "mypaint_paintop_factory.h"

#include <QApplication>

#include <kis_painter.h>
#include <kis_paintop_settings.h>
#include <kis_image.h>
#include <KoInputDevice.h>

#include "mypaint_paintop_settings_widget.h"
#include "mypaint_paintop_settings.h"
#include "mypaint_paintop.h"

#include "mypaint_brush_resource.h"
#include <kis_paintop_registry.h>
#include <kis_resource_server_provider.h>

class MyPaintFactory::Private {
public:

    KoResourceServer<MyPaintBrushResource> *brushServer;
    QMap<QString, MyPaintBrushResource*> brushes;
};



MyPaintFactory::MyPaintFactory()
    : m_d( new Private )
{
    KGlobal::mainComponent().dirs()->addResourceType("mypaint_brushes", "data", "krita/mypaintbrushes/");
    KGlobal::mainComponent().dirs()->addResourceDir("mypaint_brushes", "/usr/share/mypaint/brushes/");

    m_d->brushServer = new KoResourceServer<MyPaintBrushResource>("mypaint_brushes", "*.myb");

    QStringList extensionList = m_d->brushServer->extensions().split(':');
    QStringList fileNames;

    foreach (const QString &extension, extensionList) {
        fileNames += KGlobal::mainComponent().dirs()->findAllResources(m_d->brushServer->type().toAscii(), extension,
                                                                       KStandardDirs::Recursive | KStandardDirs::NoDuplicates);;
    }

    m_d->brushServer->loadResources(fileNames);
    foreach(MyPaintBrushResource* brush, m_d->brushServer->resources()) {
        QFileInfo info(brush->filename());
        m_d->brushes[info.baseName()] = brush;
    }

}

MyPaintFactory::~MyPaintFactory()
{
    delete m_d->brushServer;
    delete m_d;
}

KisPaintOp * MyPaintFactory::createOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisImageWSP image = 0)
{
    const MyPaintSettings *mypaintSettings = dynamic_cast<const MyPaintSettings *>(settings.data());
    Q_ASSERT(settings == 0 || mypaintSettings != 0);
    if (!settings || !mypaintSettings) {
        return 0;
    }
    KisPaintOp * op = new MyPaint(mypaintSettings, painter, image);
    Q_CHECK_PTR(op);
    return op;
}

KisPaintOpSettingsSP MyPaintFactory::settings(KisImageWSP image)
{
    Q_UNUSED(image);
    return new MyPaintSettings();
}

KisPaintOpSettingsWidget* MyPaintFactory::createSettingsWidget(QWidget * parent)
{
    return new MyPaintSettingsWidget(parent);
}

QList<MyPaintBrushResource*> MyPaintFactory::brushes() const
{
    return m_d->brushes.values();
}

MyPaintBrushResource* MyPaintFactory::brush(const QString& fileName) const
{
    if (m_d->brushes.contains(fileName)) {
        return m_d->brushes[fileName];
    }
    else {
        return m_d->brushes.values()[0];
    }
}

void MyPaintFactory::processAfterLoading()
{
    KoResourceServer<KisPaintOpPreset>* rserver = KisResourceServerProvider::instance()->paintOpPresetServer();
    QStringList blackList = rserver->blackListedFiles();

    QMapIterator<QString, MyPaintBrushResource*> i(m_d->brushes);
    while (i.hasNext()) {
        i.next();

        if (blackList.contains(i.key())) continue;

        //Create a preset for every loaded brush
        KisPaintOpSettingsSP s = settings(0);
        s->setProperty("paintop", id());
        s->setProperty("filename", i.key());

        KisPaintOpPreset* preset = new KisPaintOpPreset();
        preset->setName(i.key());
        preset->setSettings(s);
        KoID paintOpID(id(), name());
        preset->setPaintOp(paintOpID);
        preset->setValid(true);
        preset->setImage(i.value()->image());

        rserver->addResource(preset, false);
    }
}

#include "mypaint_paintop_factory.moc"
