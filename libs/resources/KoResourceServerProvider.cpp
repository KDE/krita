/*  This file is part of the KDE project

    Copyright (c) 1999 Matthias Elter <elter@kde.org>
    Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
    Copyright (c) 2005 Sven Langkamp <sven.langkamp@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "KoResourceServerProvider.h"

#include <QFileInfo>
#include <QStringList>
#include <QThread>
#include <QDir>

#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>

#include "KoSegmentGradient.h"
#include "KoStopGradient.h"

class GradientResourceServer : public KoResourceServer<KoAbstractGradient> {

public:

    GradientResourceServer() : KoResourceServer<KoAbstractGradient>()
    {
    }

private:
    virtual KoAbstractGradient* createResource( const QString & filename ) {

        QString fileExtension;
        int index = filename.lastIndexOf('.');

        if (index != -1)
            fileExtension = filename.mid(index).toLower();

        KoAbstractGradient* grad = 0;

        if(fileExtension == ".svg" || fileExtension == ".kgr")
            grad = new KoStopGradient(filename);
        else if(fileExtension == ".ggr" )
            grad = new KoSegmentGradient(filename);

        return grad;
    }
};

class ResourceLoaderThread : public QThread {

public:

    ResourceLoaderThread(KoResourceServerBase * server, const QStringList & files)
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

    KoResourceServerBase * m_server;
    QStringList m_fileNames;

};

KoResourceServerProvider *KoResourceServerProvider::m_singleton = 0;

KoResourceServerProvider::KoResourceServerProvider()
{
    KGlobal::mainComponent().dirs()->addResourceType("ko_patterns", "data", "krita/patterns/");
    KGlobal::mainComponent().dirs()->addResourceDir("ko_patterns", "/usr/share/create/patterns/gimp");
    KGlobal::mainComponent().dirs()->addResourceDir("ko_patterns", QDir::homePath() + QString("/.create/patterns/gimp"));

    KGlobal::mainComponent().dirs()->addResourceType("ko_gradients", "data", "krita/gradients/");
    KGlobal::mainComponent().dirs()->addResourceDir("ko_gradients", "/usr/share/create/gradients/gimp");
    KGlobal::mainComponent().dirs()->addResourceDir("ko_gradients", QDir::homePath() + QString("/.create/gradients/gimp"));

    m_patternServer = new KoResourceServer<KoPattern>();
    ResourceLoaderThread t1 (m_patternServer, getFileNames("*.pat", "ko_patterns"));
    t1.start();

    m_gradientServer = new GradientResourceServer();
    ResourceLoaderThread t2 (m_gradientServer, getFileNames("*.kgr:*.svg:*.ggr", "ko_gradients"));
    t2.start();

    m_paletteServer = new KoResourceServer<KoColorSet>();
    ResourceLoaderThread t3 (m_paletteServer, getFileNames("*.gpl:*.pal:*.act", "kis_palettes") );
    t3.start();

    t1.wait();
    t2.wait();
    t3.wait();
}

KoResourceServerProvider::~KoResourceServerProvider()
{
}

KoResourceServerProvider* KoResourceServerProvider::instance()
{
     if(KoResourceServerProvider::m_singleton == 0)
     {
         KoResourceServerProvider::m_singleton = new KoResourceServerProvider();
     }
    return KoResourceServerProvider::m_singleton;
}

KoResourceServer<KoPattern>* KoResourceServerProvider::patternServer()
{
    return m_patternServer;
}

KoResourceServer<KoAbstractGradient>* KoResourceServerProvider::gradientServer()
{
    return m_gradientServer;
}

KoResourceServer<KoColorSet>* KoResourceServerProvider::paletteServer()
{
    return m_paletteServer;
}


QStringList KoResourceServerProvider::getFileNames( const QString & extensions, const QString & type )
{
    QStringList extensionList = extensions.split(":");
    QStringList fileNames;

    foreach (QString extension, extensionList) {
        fileNames += KGlobal::mainComponent().dirs()->findAllResources(type.toAscii(), extension);
    }
    return fileNames;
}

