/*  This file is part of the KDE project

    Copyright (c) 1999 Matthias Elter <elter@kde.org>
    Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
    Copyright (c) 2005 Sven Langkamp <sven.langkamp@gmail.com>
    Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

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
#include <QStandardPaths>
#include <QGlobalStatic>

#include <resources/KoSegmentGradient.h>
#include <resources/KoStopGradient.h>
#include "KoColorSpaceRegistry.h"
#include "KoResourcePaths.h"
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

        KoStopGradient* gradient = new KoStopGradient();
        gradient->setType(QGradient::LinearGradient);
        gradient->setName("Foreground to Transparent");
        stops << KoGradientStop(0.0, KoColor(Qt::black, cs)) << KoGradientStop(1.0, KoColor(QColor(0, 0, 0, 0), cs));

        gradient->setStops(stops);
        gradient->setValid(true);
        gradient->setPermanent(true);
        addResource(gradient, false, true);
        m_foregroundToTransparent = gradient;

        gradient = new KoStopGradient();
        gradient->setType(QGradient::LinearGradient);
        gradient->setName("Foreground to Background");

        stops.clear();
        stops << KoGradientStop(0.0, KoColor(Qt::black, cs)) << KoGradientStop(1.0, KoColor(Qt::white, cs));

        gradient->setStops(stops);
        gradient->setValid(true);
        gradient->setPermanent(true);
        addResource(gradient, false, true);
        m_foregroundToBackground = gradient;
    }

private:

    friend class KoResourceBundle;

    KoAbstractGradient* createResource( const QString & filename ) override {

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

    QList< KoAbstractGradient* > sortedResources() override {
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

void KoResourceLoaderThread::loadSynchronously()
{
    m_server->loadResources(m_fileNames);
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


struct Q_DECL_HIDDEN KoResourceServerProvider::Private
{

    KoResourceServer<KoPattern>* patternServer;
    KoResourceServer<KoAbstractGradient>* gradientServer;
    KoResourceServer<KoColorSet>* paletteServer;
    KoResourceServer<KoSvgSymbolCollectionResource> *svgSymbolCollectionServer;

    KoResourceLoaderThread *paletteThread;
    KoResourceLoaderThread *gradientThread;
    KoResourceLoaderThread *patternThread;
    KoResourceLoaderThread *svgSymbolCollectionThread;
};

KoResourceServerProvider::KoResourceServerProvider() : d(new Private)
{

    d->patternServer = new KoResourceServerSimpleConstruction<KoPattern>("ko_patterns", "*.pat:*.jpg:*.gif:*.png:*.tif:*.xpm:*.bmp" );
    if (!QFileInfo(d->patternServer->saveLocation()).exists()) {
        QDir().mkpath(d->patternServer->saveLocation());
    }

    d->patternThread = new KoResourceLoaderThread(d->patternServer);
    d->patternThread->loadSynchronously();
//    if (qApp->applicationName().contains(QLatin1String("test"), Qt::CaseInsensitive)) {
//        d->patternThread->barrier();
//    }

    d->gradientServer = new GradientResourceServer("ko_gradients", "*.kgr:*.svg:*.ggr");
    if (!QFileInfo(d->gradientServer->saveLocation()).exists()) {
        QDir().mkpath(d->gradientServer->saveLocation());
    }

    d->gradientThread = new KoResourceLoaderThread(d->gradientServer);
    d->gradientThread->loadSynchronously();
//    if (qApp->applicationName().contains(QLatin1String("test"), Qt::CaseInsensitive)) {
//        d->gradientThread->barrier();
//    }

    d->paletteServer = new KoResourceServerSimpleConstruction<KoColorSet>("ko_palettes", "*.kpl:*.gpl:*.pal:*.act:*.aco:*.css:*.colors:*.xml:*.sbz");
    if (!QFileInfo(d->paletteServer->saveLocation()).exists()) {
        QDir().mkpath(d->paletteServer->saveLocation());
    }

    d->paletteThread = new KoResourceLoaderThread(d->paletteServer);
    d->paletteThread->loadSynchronously();
//    if (qApp->applicationName().contains(QLatin1String("test"), Qt::CaseInsensitive)) {
//        d->paletteThread->barrier();
//    }


    d->svgSymbolCollectionServer = new KoResourceServerSimpleConstruction<KoSvgSymbolCollectionResource>("symbols", "*.svg");
    if (!QFileInfo(d->svgSymbolCollectionServer->saveLocation()).exists()) {
        QDir().mkpath(d->svgSymbolCollectionServer->saveLocation());
    }
    d->svgSymbolCollectionThread = new KoResourceLoaderThread(d->svgSymbolCollectionServer);
    d->svgSymbolCollectionThread ->loadSynchronously();
}

KoResourceServerProvider::~KoResourceServerProvider()
{
    delete d->patternThread;
    delete d->gradientThread;
    delete d->paletteThread;
    delete d->svgSymbolCollectionThread;

    delete d->patternServer;
    delete d->gradientServer;
    delete d->paletteServer;
    delete d->svgSymbolCollectionServer;

    delete d;
}

Q_GLOBAL_STATIC(KoResourceServerProvider, s_instance);

KoResourceServerProvider* KoResourceServerProvider::instance()
{
    return s_instance;
}

KoResourceServer<KoPattern>* KoResourceServerProvider::patternServer(bool block)
{
    if (block) d->patternThread->barrier();
    return d->patternServer;
}

KoResourceServer<KoAbstractGradient>* KoResourceServerProvider::gradientServer(bool block)
{
    if (block) d->gradientThread->barrier();
    return d->gradientServer;
}

KoResourceServer<KoColorSet>* KoResourceServerProvider::paletteServer(bool block)
{
    if (block) d->paletteThread->barrier();
    return d->paletteServer;
}

KoResourceServer<KoSvgSymbolCollectionResource> *KoResourceServerProvider::svgSymbolCollectionServer(bool block)
{
    if (block) d->svgSymbolCollectionThread->barrier();
    return d->svgSymbolCollectionServer;
}

