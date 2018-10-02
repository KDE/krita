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

#include <QGlobalStatic>
#include <KoResourcePaths.h>

#include <KoResource.h>
#include <KoResourceServerProvider.h>

#include <kis_debug.h>

#include "kis_abr_brush.h"
#include "kis_abr_brush_collection.h"
#include "kis_gbr_brush.h"
#include "kis_imagepipe_brush.h"
#include "kis_png_brush.h"
#include "kis_svg_brush.h"

Q_GLOBAL_STATIC(KisBrushServer, s_instance)


class BrushResourceServer : public KisBrushResourceServer
{

public:

    BrushResourceServer()
        : KisBrushResourceServer("brushes", "*.gbr:*.gih:*.abr:*.png:*.svg")
    {
    }

    ///Reimplemented
    bool importResourceFile(const QString& filename, bool fileCreation = true) override {
        QFileInfo fi(filename);
        if (fi.exists() == false)
            return false;

        if (fi.size() == 0) return false;

        if (fi.suffix().toLower() == "abr") {
            if (fileCreation) {
                QFile::copy(filename, saveLocation() + fi.fileName());
            }
            QList<KisBrushSP> collectionResources = createResources(filename);
            Q_FOREACH (KisBrushSP brush, collectionResources) {
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
    QList<KisBrushSP> createResources(const QString & filename) override {
        QList<KisBrushSP> brushes;

        QString fileExtension = QFileInfo(filename).suffix().toLower();
        if (fileExtension == "abr") {
            KisAbrBrushCollection collection(filename);
            collection.load();
            Q_FOREACH (KisAbrBrush * abrBrush, collection.brushes()) {
//                abrBrush->setBrushTipImage(QImage());
                brushes.append(abrBrush);
                addTag(abrBrush, collection.filename());
            }
        }
        else {
            brushes.append(createResource(filename));
        }
        return brushes;
    }

    ///Reimplemented
    KisBrushSP createResource(const QString & filename) override {

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
    m_brushServer = new BrushResourceServer();
    m_brushServer->loadResources(KoResourceServerProvider::blacklistFileNames(m_brushServer->fileNames(), m_brushServer->blackListedFiles()));

    Q_FOREACH (KisBrushSP brush, m_brushServer->resources()) {
        if (!dynamic_cast<KisAbrBrush*>(brush.data())) {
            brush->setBrushTipImage(QImage());
        }
    }
}

KisBrushServer::~KisBrushServer()
{
    delete m_brushServer;
}

KisBrushServer* KisBrushServer::instance()
{
    return s_instance;
}

KisBrushResourceServer* KisBrushServer::brushServer()
{
    return m_brushServer;
}

void KisBrushServer::slotRemoveBlacklistedResources()
{
    m_brushServer->removeBlackListedFiles();
}

