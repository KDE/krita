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

struct Q_DECL_HIDDEN KoResourceServerProvider::Private
{
    KoResourceServer<KoPattern>* patternServer;
    KoResourceServer<KoAbstractGradient>* gradientServer;
    KoResourceServer<KoColorSet>* paletteServer;
    KoResourceServer<KoSvgSymbolCollectionResource> *svgSymbolCollectionServer;
    KoResourceServer<KoGamutMask>* gamutMaskServer;
};

KoResourceServerProvider::KoResourceServerProvider() : d(new Private)
{
    d->patternServer = new KoResourceServerSimpleConstruction<KoPattern>("ko_patterns", "*.pat:*.jpg:*.gif:*.png:*.tif:*.xpm:*.bmp" );
    d->patternServer->loadResources(blacklistFileNames(d->patternServer->fileNames(), d->patternServer->blackListedFiles()));

    d->gradientServer = new GradientResourceServer("ko_gradients", "*.svg:*.ggr");
    d->gradientServer->loadResources(blacklistFileNames(d->gradientServer->fileNames(), d->gradientServer->blackListedFiles()));

    d->paletteServer = new KoResourceServerSimpleConstruction<KoColorSet>("ko_palettes", "*.kpl:*.gpl:*.pal:*.act:*.aco:*.css:*.colors:*.xml:*.sbz");
    d->paletteServer->loadResources(blacklistFileNames(d->paletteServer->fileNames(), d->paletteServer->blackListedFiles()));

    d->svgSymbolCollectionServer = new KoResourceServerSimpleConstruction<KoSvgSymbolCollectionResource>("symbols", "*.svg");
    d->svgSymbolCollectionServer->loadResources(blacklistFileNames(d->svgSymbolCollectionServer->fileNames(), d->svgSymbolCollectionServer->blackListedFiles()));

    d->gamutMaskServer = new KoResourceServerSimpleConstruction<KoGamutMask>("ko_gamutmasks", "*.kgm");
    d->gamutMaskServer->loadResources(blacklistFileNames(d->gamutMaskServer->fileNames(), d->gamutMaskServer->blackListedFiles()));
}

KoResourceServerProvider::~KoResourceServerProvider()
{
    delete d->patternServer;
    delete d->gradientServer;
    delete d->paletteServer;
    delete d->svgSymbolCollectionServer;
    delete d->gamutMaskServer;

    delete d;
}

Q_GLOBAL_STATIC(KoResourceServerProvider, s_instance)

KoResourceServerProvider* KoResourceServerProvider::instance()
{
    return s_instance;
}

QStringList KoResourceServerProvider::blacklistFileNames(QStringList fileNames, const QStringList &blacklistedFileNames)
{
    if (!blacklistedFileNames.isEmpty()) {
        foreach (const QString &s, blacklistedFileNames) {
            fileNames.removeAll(s);
        }
    }
    return fileNames;
}

KoResourceServer<KoPattern>* KoResourceServerProvider::patternServer()
{
    return d->patternServer;
}

KoResourceServer<KoAbstractGradient>* KoResourceServerProvider::gradientServer()
{
    return d->gradientServer;
}

KoResourceServer<KoColorSet>* KoResourceServerProvider::paletteServer()
{
    return d->paletteServer;
}

KoResourceServer<KoSvgSymbolCollectionResource> *KoResourceServerProvider::svgSymbolCollectionServer()
{
    return d->svgSymbolCollectionServer;
}

KoResourceServer<KoGamutMask>* KoResourceServerProvider::gamutMaskServer()
{
    return d->gamutMaskServer;
}


