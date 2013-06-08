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
#include <kis_resource_server_provider.h>

class BrushResourceServer : public KoResourceServer<KisBrush>, public KoResourceServerObserver<KisBrush>
{

public:

    BrushResourceServer() : KoResourceServer<KisBrush>("kis_brushes", "*.gbr:*.gih:*.abr:*.png:*.svg", false) {
        addObserver(this, true);
    }

    ~BrushResourceServer() {
        foreach(KisBrush* brush, m_brushes)
        {
            removeResourceFromServer(brush);
        }
    }

    virtual void resourceAdded(KisBrush* brush)
    {
        // Hack: This prevents the deletion of brushes in the resource server
        // Brushes outside the server use shared pointer, but not inside the server
        brush->ref();
        m_brushes.append(brush);
    }

    ///Reimplemented
    virtual void removingResource(KisBrush* brush)
    {
        if(!brush->deref()) {
            delete brush;
        }
        m_brushes.removeAll(brush);
    }

    virtual void resourceChanged(KisBrush* resource) {
        Q_UNUSED(resource);
    }

    virtual void syncTaggedResourceView(){}

    virtual void syncTagAddition(const QString& tag){}

    virtual void syncTagRemoval(const QString& tag){}

    ///Reimplemented
    virtual void importResourceFile(const QString& filename, bool fileCreation = true)
    {
        QFileInfo fi( filename );
        if( fi.exists() == false )
            return;

        if( fi.suffix().toLower() == "abr") {
            if(fileCreation) {
                QFile::copy(filename, saveLocation() + fi.fileName());
            }
            QList<KisBrush*> collectionResources = createResources( filename );
            foreach(KisBrush* brush, collectionResources) {
                addResource(brush);
            }
        } else {
            KoResourceServer<KisBrush>::importResourceFile(filename, fileCreation);
        }
    }

private:

    ///Reimplemented
    virtual QList<KisBrush*> createResources( const QString & filename )
    {
        QList<KisBrush*> brushes;

        QString fileExtension = QFileInfo(filename).suffix().toLower();
        if(fileExtension == "abr") {
            KisAbrBrushCollection collection(filename);
            collection.load();
            foreach(KisAbrBrush* abrBrush, collection.brushes()) {
                brushes.append(abrBrush);
            }
        } else {
            brushes.append(createResource(filename));
        }
        return brushes;
    }

    ///Reimplemented
    virtual KisBrush* createResource(const QString & filename) {

        QString fileExtension = QFileInfo(filename).suffix().toLower();

        KisBrush* brush = 0;

        if (fileExtension == "gbr") {
            brush = new KisGbrBrush(filename);
        } else if (fileExtension == "gih") {
            brush = new KisImagePipeBrush(filename);
        } else if (fileExtension == "png") {
            brush = new KisPngBrush(filename);
        } else if (fileExtension == "svg") {
            brush = new KisSvgBrush(filename);
        }
        return brush;
    }

    QList<KisBrush*> m_brushes;
};

KisBrushServer::KisBrushServer()
{
    KGlobal::mainComponent().dirs()->addResourceType("kis_brushes", "data", "krita/brushes/");
    KGlobal::mainComponent().dirs()->addResourceDir("kis_brushes", "/usr/share/create/brushes/gimp");
    KGlobal::mainComponent().dirs()->addResourceDir("kis_brushes", QDir::homePath() + QString("/.create/brushes/gimp"));

    m_brushServer = new BrushResourceServer();
    m_brushThread = new KoResourceLoaderThread(m_brushServer);
    m_brushThread->start();

    if (qApp->applicationName().toLower().contains("test")) {
        m_brushThread->wait();
    }

    connect(KisResourceServerProvider::instance(), SIGNAL(notifyBrushBlacklistCleanup()),
            this, SLOT(slotRemoveBlacklistedResources()));
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


KoResourceServer<KisBrush>* KisBrushServer::brushServer()
{
    m_brushThread->barrier();
    return m_brushServer;
}

void KisBrushServer::slotRemoveBlacklistedResources()
{
    m_brushServer->removeBlackListedFiles();
}

#include "kis_brush_server.moc"
