/*  This file is part of the KDE project

    Copyright (c) 1999 Matthias Elter <elter@kde.org>
    Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
    Copyright (c) 2005 Sven Langkamp <sven.langkamp@gmail.com>
    Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>

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

#include <QApplication>
#include <QFileInfo>
#include <QStringList>
#include <QDir>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>

#include "KoSegmentGradient.h"
#include "KoStopGradient.h"
#include "KoColorSpaceRegistry.h"

#include <iostream>
using namespace std;

class GradientResourceServer : public KoResourceServer<KoAbstractGradient> {

public:

    GradientResourceServer(const QString& type, const QString& extensions) :
            KoResourceServer<KoAbstractGradient>(type, extensions) , m_foregroundToTransparent(0) , m_foregroundToBackground(0)
    {
        insertSpecialGradients();
    }

    void insertSpecialGradients()
    {
        const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
        QList<KoGradientStop> stops;

        KoStopGradient* gradient = new KoStopGradient("");
        gradient->setType(QGradient::LinearGradient);
        gradient->setName("Foreground to Transparent");
        stops << KoGradientStop(0.0, KoColor(Qt::black, cs)) << KoGradientStop(1.0, KoColor(QColor(0, 0, 0, 0), cs));

        gradient->setStops(stops);
        gradient->setValid(true);
        addResource(gradient, false, true);
        m_foregroundToTransparent = gradient;

        gradient = new KoStopGradient("");
        gradient->setType(QGradient::LinearGradient);
        gradient->setName("Foreground to Background");

        stops.clear();
        stops << KoGradientStop(0.0, KoColor(Qt::black, cs)) << KoGradientStop(1.0, KoColor(Qt::white, cs));

        gradient->setStops(stops);
        gradient->setValid(true);
        addResource(gradient, false, true);
        m_foregroundToBackground = gradient;
    }

private:

    friend class KoResourceBundle;

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

    virtual QList< KoAbstractGradient* > sortedResources() {
        QList< KoAbstractGradient* > resources = KoResourceServer<KoAbstractGradient>::sortedResources();
        QList< KoAbstractGradient* > sorted;
        if (m_foregroundToTransparent && resources.contains(m_foregroundToTransparent)) {
            sorted.append(resources.takeAt(resources.indexOf(m_foregroundToTransparent)));
        }
        if (m_foregroundToBackground && resources.contains(m_foregroundToBackground)) {
            sorted.append(resources.takeAt(resources.indexOf(m_foregroundToBackground)));
        }
        return sorted + resources;
    }

    KoAbstractGradient* m_foregroundToTransparent;
    KoAbstractGradient* m_foregroundToBackground;
};

KoResourceLoaderThread::KoResourceLoaderThread(KoResourceServerBase * server)
    : QThread()
    , m_server(server)
{
    m_fileNames = m_server->fileNames();
    QStringList fileNames = m_server->blackListedFiles();

    if (!fileNames.isEmpty()) {
        foreach (const QString &s, fileNames) {
            if (m_fileNames.contains(s)) {
               m_fileNames.removeAll(s);
            }
        }
    }
    connect(qApp, SIGNAL(aboutToQuit()), SLOT(barrier()));
}

KoResourceLoaderThread::~KoResourceLoaderThread()
{
}

void KoResourceLoaderThread::run()
{
    m_server->loadResources(m_fileNames);
}

void KoResourceLoaderThread::barrier()
{
    if(isRunning()) {
        wait();
    }
}


struct KoResourceServerProvider::Private
{
    KoResourceServer<KoPattern>* patternServer;
    KoResourceServer<KoAbstractGradient>* gradientServer;
    KoResourceServer<KoColorSet>* paletteServer;

    KoResourceLoaderThread *paletteThread;
    KoResourceLoaderThread *gradientThread;
    KoResourceLoaderThread *patternThread;
};

KoResourceServerProvider::KoResourceServerProvider() : d(new Private)
{
    KGlobal::mainComponent().dirs()->addResourceType("ko_patterns", "data", "krita/patterns/", true);
    KGlobal::mainComponent().dirs()->addResourceDir("ko_patterns", "/usr/share/create/patterns/gimp");
    KGlobal::mainComponent().dirs()->addResourceDir("ko_patterns", QDir::homePath() + QString("/.create/patterns/gimp"));

    KGlobal::mainComponent().dirs()->addResourceType("ko_gradients", "data", "karbon/gradients/");
    KGlobal::mainComponent().dirs()->addResourceType("ko_gradients", "data", "krita/gradients/", true);
    KGlobal::mainComponent().dirs()->addResourceDir("ko_gradients", "/usr/share/create/gradients/gimp");
    KGlobal::mainComponent().dirs()->addResourceDir("ko_gradients", QDir::homePath() + QString("/.create/gradients/gimp"));

    KGlobal::mainComponent().dirs()->addResourceType("ko_palettes", "data", "calligra/palettes/");
    KGlobal::mainComponent().dirs()->addResourceType("ko_palettes", "data", "karbon/palettes/");
    KGlobal::mainComponent().dirs()->addResourceType("ko_palettes", "data", "krita/palettes/", true);

    KGlobal::mainComponent().dirs()->addResourceDir("ko_palettes", "/usr/share/create/swatches");
    KGlobal::mainComponent().dirs()->addResourceDir("ko_palettes", QDir::homePath() + QString("/.create/swatches"));

    d->patternServer = new KoResourceServer<KoPattern>("ko_patterns", "*.pat:*.jpg:*.gif:*.png:*.tif:*.xpm:*.bmp" );
    if (!QFileInfo(d->patternServer->saveLocation()).exists()) {
        QDir().mkpath(d->patternServer->saveLocation());
    }

    d->patternThread = new KoResourceLoaderThread(d->patternServer);
    d->patternThread->start();
    if (qApp->applicationName().contains(QLatin1String("test"), Qt::CaseInsensitive)) {
        d->patternThread->wait();
    }

    d->gradientServer = new GradientResourceServer("ko_gradients", "*.kgr:*.svg:*.ggr");
    if (!QFileInfo(d->gradientServer->saveLocation()).exists()) {
        QDir().mkpath(d->gradientServer->saveLocation());
    }

    d->gradientThread = new KoResourceLoaderThread(d->gradientServer);
    d->gradientThread->start();

    d->paletteServer = new KoResourceServer<KoColorSet>("ko_palettes", "*.gpl:*.pal:*.act:*.aco:*.css:*.colors");
    if (!QFileInfo(d->paletteServer->saveLocation()).exists()) {
        QDir().mkpath(d->paletteServer->saveLocation());
    }

    d->paletteThread = new KoResourceLoaderThread(d->paletteServer);
    d->paletteThread->start();
}

KoResourceServerProvider::~KoResourceServerProvider()
{
    delete d->patternThread;
    delete d->gradientThread;
    delete d->paletteThread;

    delete d->patternServer;
    delete d->gradientServer;
    delete d->paletteServer;

    delete d;
}

KoResourceServerProvider* KoResourceServerProvider::instance()
{
    K_GLOBAL_STATIC(KoResourceServerProvider, s_instance);
    return s_instance;
}

KoResourceServer<KoPattern>* KoResourceServerProvider::patternServer(bool block)
{
    if (block) d->patternThread->barrier();
    return d->patternServer;
}

KoResourceServer<KoAbstractGradient>* KoResourceServerProvider::gradientServer(bool block)
{
    if (block) d->patternThread->barrier();
    return d->gradientServer;
}

KoResourceServer<KoColorSet>* KoResourceServerProvider::paletteServer(bool block)
{
    if (block) d->patternThread->barrier();
    return d->paletteServer;
}

