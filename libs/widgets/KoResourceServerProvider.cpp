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
#include <QDir>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>

#include "KoSegmentGradient.h"
#include "KoStopGradient.h"
#include "KoColorSpaceRegistry.h"

class GradientResourceServer : public KoResourceServer<KoAbstractGradient> {

public:

    GradientResourceServer(const QString& type, const QString& extensions) :
            KoResourceServer<KoAbstractGradient>(type, extensions)
    {
        const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
        KoStopGradient* gradient = new KoStopGradient("");
        gradient->setType(QGradient::LinearGradient);
        gradient->setName("Foreground to Background");
        
        QList<KoGradientStop> stops;
        stops << KoGradientStop(0.0, KoColor(Qt::black, cs)) << KoGradientStop(1.0, KoColor(Qt::white, cs));
        gradient->setStops(stops);
        gradient->setValid(true);
        addResource(gradient, false);
        
        gradient = new KoStopGradient("");
        gradient->setType(QGradient::LinearGradient);
        gradient->setName("Foreground to Transparent");
        
        stops.clear();
        stops << KoGradientStop(0.0, KoColor(Qt::black, cs)) << KoGradientStop(1.0, KoColor(QColor(0, 0, 0, 0), cs));
        gradient->setStops(stops);
        gradient->setValid(true);
        addResource(gradient, false);
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

KoResourceLoaderThread::KoResourceLoaderThread(KoResourceServerBase * server)
    : QThread()
    , m_server(server)
{
    m_fileNames = getFileNames(m_server->extensions());
}

void KoResourceLoaderThread::run()
{
    m_server->loadResources(m_fileNames);
}

void KoResourceLoaderThread::cancel()
{
    m_server->cancel();
}

QStringList KoResourceLoaderThread::getFileNames( const QString & extensions)
{
    QStringList extensionList = extensions.split(':');
    QStringList fileNames;

    foreach (const QString &extension, extensionList) {
        fileNames += KGlobal::mainComponent().dirs()->findAllResources(m_server->type().toAscii(), extension);
    }
    return fileNames;
}

struct KoResourceServerProvider::Private
{
    KoResourceServer<KoPattern>* m_patternServer;
    KoResourceServer<KoAbstractGradient>* m_gradientServer;
    KoResourceServer<KoColorSet>* m_paletteServer;
    
    QThread * paletteThread;
    QThread * gradientThread;
    QThread * patternThread;
};

KoResourceServerProvider::KoResourceServerProvider() : d(new Private)
{
    KGlobal::mainComponent().dirs()->addResourceType("ko_patterns", "data", "krita/patterns/");
    KGlobal::mainComponent().dirs()->addResourceDir("ko_patterns", "/usr/share/create/patterns/gimp");
    KGlobal::mainComponent().dirs()->addResourceDir("ko_patterns", QDir::homePath() + QString("/.create/patterns/gimp"));

    KGlobal::mainComponent().dirs()->addResourceType("ko_gradients", "data", "krita/gradients/");
    KGlobal::mainComponent().dirs()->addResourceType("ko_gradients", "data", "karbon/gradients/");
    KGlobal::mainComponent().dirs()->addResourceDir("ko_gradients", "/usr/share/create/gradients/gimp");
    KGlobal::mainComponent().dirs()->addResourceDir("ko_gradients", QDir::homePath() + QString("/.create/gradients/gimp"));

    KGlobal::mainComponent().dirs()->addResourceType("ko_palettes", "data", "krita/palettes/");
    KGlobal::mainComponent().dirs()->addResourceDir("ko_palettes", "/usr/share/create/swatches");
    KGlobal::mainComponent().dirs()->addResourceDir("ko_palettes", QDir::homePath() + QString("/.create/swatches"));

    d->m_patternServer = new KoResourceServer<KoPattern>("ko_patterns", "*.pat:*.jpg:*.gif:*.png:*.tif:*.xpm:*.bmp" );
    d->patternThread = new KoResourceLoaderThread(d->m_patternServer);
    connect(d->patternThread, SIGNAL(finished()), this, SLOT(patternThreadDone()));
    d->patternThread->start();

    d->m_gradientServer = new GradientResourceServer("ko_gradients", "*.kgr:*.svg:*.ggr");
    d->gradientThread = new KoResourceLoaderThread(d->m_gradientServer);
    connect(d->gradientThread, SIGNAL(finished()), this, SLOT(gradientThreadDone()));
    d->gradientThread->start();

    d->m_paletteServer = new KoResourceServer<KoColorSet>("ko_palettes", "*.gpl:*.pal:*.act");
    d->paletteThread = new KoResourceLoaderThread(d->m_paletteServer);
    connect(d->paletteThread, SIGNAL(finished()), this, SLOT(paletteThreadDone()));
    d->paletteThread->start();

}

KoResourceServerProvider::~KoResourceServerProvider()
{
    delete d->m_patternServer;
    delete d->m_gradientServer;
    delete d->m_paletteServer;
    delete d;
}

KoResourceServerProvider* KoResourceServerProvider::instance()
{
    K_GLOBAL_STATIC(KoResourceServerProvider, s_instance);
    return s_instance;
}

KoResourceServer<KoPattern>* KoResourceServerProvider::patternServer()
{
    return d->m_patternServer;
}

KoResourceServer<KoAbstractGradient>* KoResourceServerProvider::gradientServer()
{
    return d->m_gradientServer;
}

KoResourceServer<KoColorSet>* KoResourceServerProvider::paletteServer()
{
    return d->m_paletteServer;
}


void KoResourceServerProvider::paletteThreadDone()
{
    delete d->paletteThread;
    d->paletteThread = 0;
}

void KoResourceServerProvider::patternThreadDone()
{
    delete d->patternThread;
    d->patternThread = 0;
}

void KoResourceServerProvider::gradientThreadDone()
{
    delete d->gradientThread;
    d->gradientThread = 0;
}
