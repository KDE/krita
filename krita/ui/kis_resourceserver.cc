/*
 *  kis_resourceserver.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Sven Langkamp <longamp@reallygood.de>
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
#include <qfileinfo.h>
#include <qstringlist.h>
#include <qthread.h>
#include <qdir.h>
//Added by qt3to4:
#include <Q3ValueList>

#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kinstance.h>

#include "kis_resource.h"
#include "kis_factory.h"
#include "kis_generic_registry.h"
#include "kis_resourceserver.h"
#include "kis_brush.h"
#include "kis_imagepipe_brush.h"
#include "kis_gradient.h"
#include "kis_pattern.h"
#include "kis_palette.h"
#include <kogradientmanager.h>

KisResourceServerBase::KisResourceServerBase(QString type)
    : m_type(type), m_loaded(false)
{
}

KisResourceServerBase::~KisResourceServerBase()
{
}

void KisResourceServerBase::loadResources(QStringList filenames)
{
    QStringList uniqueFiles;

    while (!filenames.empty())
    {
        QString front = filenames.first();
        filenames.pop_front();

        QString fname = QFileInfo(front).fileName();
        //ebug() << "Loading " << fname << "\n";
        // XXX: Don't load resources with the same filename. Actually, we should look inside
        //      the resource to find out whether they are really the same, but for now this
        //      will prevent the same brush etc. showing up twice.
        if (uniqueFiles.empty() || uniqueFiles.indexOf(fname) == -1) {
            uniqueFiles.append(fname);
            KisResource *resource;
            resource = createResource(front);
            if (resource->load() && resource->valid())
            {
                m_resources.append(resource);
                Q_CHECK_PTR(resource);
                emit resourceAdded(resource);
            }
            else {
                delete resource;
            }
        }
    }
    m_loaded = true;
}

QList<KisResource*> KisResourceServerBase::resources()
{
    if(!m_loaded) {
        return QList<KisResource*>();
    }

    return m_resources;
}

void KisResourceServerBase::addResource(KisResource* resource)
{
    if (!resource->valid()) {
        kWarning(41001) << "Tried to add an invalid resource!" << endl;
        return;
    }

    m_resources.append(resource);
    emit resourceAdded(resource);
}


class ResourceLoaderThread : public QThread {

public:

    ResourceLoaderThread(KisResourceServerBase * server, QStringList files)
        : QThread()
        , m_server(server)
        , m_fileNames( files )
    {
    }


    void run()
    {
        m_server->loadResources(m_fileNames);
    }

private:

    KisResourceServerBase * m_server;
    QStringList m_fileNames;

};

QStringList getFileNames( QString extensions, QString type )
{
    QStringList extensionList = extensions.split(":");
    QStringList fileNames;

    foreach (QString extension, extensionList) {
        fileNames += KisFactory::instance()->dirs()->findAllResources(type.toAscii(), extension);
    }
    return fileNames;
}


KisResourceServerRegistry *KisResourceServerRegistry::m_singleton = 0;

KisResourceServerRegistry::KisResourceServerRegistry()
{

    KisResourceServer<KisBrush>* brushServer = new KisResourceServer<KisBrush>("kis_brushes");
    ResourceLoaderThread t1 (brushServer, getFileNames( "*.gbr","kis_brushes" ));
    t1.start();

    KisResourceServer<KisImagePipeBrush>* imagePipeBrushServer = new KisResourceServer<KisImagePipeBrush>("kis_brushes");
    ResourceLoaderThread t2 (imagePipeBrushServer, getFileNames( "*.gih", "kis_brushes"));
    t2.start();

    KisResourceServer<KisPattern>* patternServer = new KisResourceServer<KisPattern>("kis_patterns");
    ResourceLoaderThread t3 (patternServer, getFileNames("*.pat", "kis_patterns"));
    t3.start();

    KisResourceServer<KisGradient>* gradientServer = new KisResourceServer<KisGradient>("kis_gradients");
    ResourceLoaderThread t4 (gradientServer, getFileNames(KoGradientManager::filters().join( ":" ), "kis_gradients"));
    t4.start();


    KisResourceServer<KisPalette>* paletteServer = new KisResourceServer<KisPalette>("kis_palettes");
    ResourceLoaderThread t5 (paletteServer, getFileNames("*.gpl:*.pal:*.act", "kis_palettes") );
    t5.start();

    t1.wait();
    t2.wait();
    t3.wait();
    t4.wait();
    t5.wait();

    add( KisID( "BrushServer", ""), brushServer );
    add( KisID( "ImagePipeBrushServer", ""), imagePipeBrushServer );
    add( KisID( "PatternServer", ""), patternServer );
    add( KisID( "GradientServer", ""), gradientServer );
    add( KisID( "PaletteServer", ""), paletteServer );

}

KisResourceServerRegistry::~KisResourceServerRegistry()
{
}

KisResourceServerRegistry* KisResourceServerRegistry::instance()
{
     if(KisResourceServerRegistry::m_singleton == 0)
     {
         KisResourceServerRegistry::m_singleton = new KisResourceServerRegistry();
     }
    return KisResourceServerRegistry::m_singleton;
}


#include "kis_resourceserver.moc"

