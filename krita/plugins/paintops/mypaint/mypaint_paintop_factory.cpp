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

#include <QThread>

#include "mypaint_brush_resource.h"

class MyPaintFactory::Private {
public:

    KoResourceServer<MyPaintBrushResource>* brushServer;
    KoResourceLoaderThread* brushThread;
    QMap<QString, MyPaintBrushResource*> brushes;
};



MyPaintFactory::MyPaintFactory()
    : m_d( new Private )
{
    KGlobal::mainComponent().dirs()->addResourceType("mypaint_brushes", "data", "krita/brushes/");
    m_d->brushServer = new KoResourceServer<MyPaintBrushResource>("mypaint_brushes", "*.myb");
    m_d->brushThread = new KoResourceLoaderThread(m_d->brushServer);
    connect(m_d->brushThread, SIGNAL(finished()), this, SLOT(brushThreadDone()));
    m_d->brushThread->start();

}

MyPaintFactory::~MyPaintFactory()
{
    if (m_d->brushThread) m_d->brushThread->cancel();
    delete m_d->brushServer;
    delete m_d;
}

KisPaintOp * MyPaintFactory::createOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisImageWSP image = 0)
{
    const MyPaintSettings *mypaintSettings = dynamic_cast<const MyPaintSettings *>(settings.data());
    Q_ASSERT(settings == 0 || mypaintSettings != 0);
    KisPaintOp * op = new MyPaint(mypaintSettings, painter, image);
    qDebug() << "got op" << op;
    Q_CHECK_PTR(op);
    return op;
}

KisPaintOpSettingsSP MyPaintFactory::settings(KisImageWSP image)
{
    Q_UNUSED(image);
    return new MyPaintSettings();
}

void MyPaintFactory::brushThreadDone()
{
    delete m_d->brushThread;
    m_d->brushThread = 0;
    foreach(MyPaintBrushResource* brush, m_d->brushServer->resources()) {
        QFileInfo info(brush->filename());
        m_d->brushes[info.baseName()] = brush;
    }
}

KisPaintOpSettingsWidget* MyPaintFactory::createSettingsWidget(QWidget * parent)
{
    while(m_d->brushThread) {
        qApp->processEvents();
    }
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


#include "mypaint_paintop_factory.moc"
