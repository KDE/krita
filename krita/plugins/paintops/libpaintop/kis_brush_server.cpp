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

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>

#include <KoResource.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>

#include "kis_brush.h"
#include "kis_imagepipe_brush.h"
class BrushResourceServer : public KoResourceServer<KisBrush> {

public:

    BrushResourceServer() : KoResourceServer<KisBrush>("kis_brushes")
    {
    }

private:
    virtual KisBrush* createResource( const QString & filename ) {

        QString fileExtension;
        int index = filename.lastIndexOf('.');

        if (index != -1)
            fileExtension = filename.mid(index).toLower();

        KisBrush* brush = 0;

        if(fileExtension == ".gbr")
            brush = new KisBrush(filename);
        else if(fileExtension == ".gih" )
            brush = new KisImagePipeBrush(filename);

        return brush;
    }
};

KisBrushServer *KisBrushServer::m_singleton = 0;

KisBrushServer::KisBrushServer()
{
    KGlobal::mainComponent().dirs()->addResourceType("kis_brushes", "data", "krita/brushes/");
    KGlobal::mainComponent().dirs()->addResourceDir("kis_brushes", "/usr/share/create/brushes/gimp");
    KGlobal::mainComponent().dirs()->addResourceDir("kis_brushes", QDir::homePath() + QString("/.create/brushes/gimp"));

    m_brushServer = new BrushResourceServer();
    brushThread = new KoResourceLoaderThread(m_brushServer, "*.gbr:*.gih");
    connect(brushThread, SIGNAL(finished()), this, SLOT(brushThreadDone()));
    brushThread->start();
}

KisBrushServer::~KisBrushServer()
{
}

KisBrushServer* KisBrushServer::instance()
{
     if(KisBrushServer::m_singleton == 0)
     {
         KisBrushServer::m_singleton = new KisBrushServer();
     }
    return KisBrushServer::m_singleton;
}


KoResourceServer<KisBrush>* KisBrushServer::brushServer()
{
    return m_brushServer;
}

void KisBrushServer::brushThreadDone()
{
    delete brushThread;
    brushThread = 0;
}

#include "kis_brush_server.moc"
