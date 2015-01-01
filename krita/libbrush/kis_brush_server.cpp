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
#include "kis_brush_server.h"

#include <QDir>
#include <QApplication>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>

#include <KoResource.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>

#include <kis_debug.h>

#include "kis_abr_brush.h"
#include "kis_abr_brush_collection.h"
#include "kis_gbr_brush.h"
#include "kis_imagepipe_brush.h"
#include "kis_png_brush.h"
#include "kis_svg_brush.h"

class BrushResourceServer : public KisBrushResourceServer
{

public:

    BrushResourceServer()
        : KisBrushResourceServer("kis_brushes", "*.gbr:*.gih:*.abr:*.png:*.svg")
    {
    }

    ///Reimplemented
    virtual bool importResourceFile(const QString& filename, bool fileCreation = true) {
        QFileInfo fi(filename);
        if (fi.exists() == false)
            return false;

        if (fi.size() == 0) return false;

        if (fi.suffix().toLower() == "abr") {
            if (fileCreation) {
                QFile::copy(filename, saveLocation() + fi.fileName());
            }
            QList<KisBrushSP> collectionResources = createResources(filename);
            foreach(KisBrushSP brush, collectionResources) {
                addResource(brush);
            }
        }
        else {

            return KisBrushResourceServer::importResourceFile(filename, fileCreation);

        }
        qApp->processEvents(QEventLoop::AllEvents);
        return true;
    }

private:

    ///Reimplemented
    virtual QList<KisBrushSP> createResources(const QString & filename) {
        QList<KisBrushSP> brushes;

        QString fileExtension = QFileInfo(filename).suffix().toLower();
        if (fileExtension == "abr") {
            KisAbrBrushCollection collection(filename);
            collection.load();
            foreach(KisAbrBrush * abrBrush, collection.brushes()) {
//                abrBrush->setBrushTipImage(QImage());
                brushes.append(abrBrush);
            }
        }
        else {
            brushes.append(createResource(filename));
        }
        return brushes;
    }

    ///Reimplemented
    virtual KisBrushSP createResource(const QString & filename) {

        QString fileExtension = QFileInfo(filename).suffix().toLower();

        KisBrushSP brush;

        if (fileExtension == "gbr") {
            brush = new KisGbrBrush(filename);
        }
        else if (fileExtension == "gih") {
            brush = new KisImagePipeBrush(filename);
        }
        else if (fileExtension == "png") {
            brush = new KisPngBrush(filename);
        }
        else if (fileExtension == "svg") {
            brush = new KisSvgBrush(filename);
        }
        return brush;
    }
};

KisBrushServer::KisBrushServer()
{
    KGlobal::mainComponent().dirs()->addResourceType("kis_brushes", "data", "krita/brushes/");
    KGlobal::mainComponent().dirs()->addResourceDir("kis_brushes", "/usr/share/create/brushes/gimp");
    KGlobal::mainComponent().dirs()->addResourceDir("kis_brushes", QDir::homePath() + QString("/.create/brushes/gimp"));

    m_brushServer = new BrushResourceServer();
    if (!QFileInfo(m_brushServer->saveLocation()).exists()) {
        QDir().mkpath(m_brushServer->saveLocation());
    }
    m_brushThread = new KoResourceLoaderThread(m_brushServer);
    m_brushThread->start();
    foreach(KisBrushSP brush, m_brushServer->resources()) {
        if (!dynamic_cast<KisAbrBrush*>(brush.data())) {
            brush->setBrushTipImage(QImage());
        }
    }
}

KisBrushServer::~KisBrushServer()
{
    dbgRegistry << "deleting KisBrushServer";
    delete m_brushThread;
    delete m_brushServer;
}

KisBrushServer* KisBrushServer::instance()
{
    K_GLOBAL_STATIC(KisBrushServer, s_instance);
    return s_instance;
}


KisBrushResourceServer* KisBrushServer::brushServer(bool block)
{
    if (block) m_brushThread->barrier();
    return m_brushServer;
}

void KisBrushServer::slotRemoveBlacklistedResources()
{
    m_brushServer->removeBlackListedFiles();
}

#include "kis_brush_server.moc"
