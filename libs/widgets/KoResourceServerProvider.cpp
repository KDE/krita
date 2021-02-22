/*  This file is part of the KDE project

    SPDX-FileCopyrightText: 1999 Matthias Elter <elter@kde.org>
    SPDX-FileCopyrightText: 2003 Patrick Julien <freak@codepimps.org>
    SPDX-FileCopyrightText: 2005 Sven Langkamp <sven.langkamp@gmail.com>
    SPDX-FileCopyrightText: 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
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
#include "klocalizedstring.h"
#include <iostream>

using namespace std;

class GradientResourceServer : public KoResourceServer<KoAbstractGradient> {

public:

    GradientResourceServer(const QString& type)
        : KoResourceServer<KoAbstractGradient>(type)
    {
        insertSpecialGradients();
    }

    void insertSpecialGradients()
    {
        const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
        QList<KoGradientStop> stops;

        KoStopGradientSP gradient(new KoStopGradient());
        gradient->setType(QGradient::LinearGradient);
        gradient->setName(i18n("Foreground to Transparent"));
        gradient->setFilename("Foreground to Transparent.svg");
        stops << KoGradientStop(0.0, KoColor(Qt::black, cs), FOREGROUNDSTOP);
        stops << KoGradientStop(1.0, KoColor(QColor(0, 0, 0, 0), cs), COLORSTOP);

        gradient->setStops(stops);
        gradient->setValid(true);
        gradient->setPermanent(true);
        addResource(gradient, false);
        m_foregroundToTransparent = gradient;

        gradient.reset(new KoStopGradient());
        gradient->setType(QGradient::LinearGradient);
        gradient->setName(i18n("Foreground to Background"));
        gradient->setFilename("Foreground to Background.svg");

        stops.clear();
        stops << KoGradientStop(0.0, KoColor(Qt::black, cs), FOREGROUNDSTOP);
        stops << KoGradientStop(1.0, KoColor(Qt::white, cs), BACKGROUNDSTOP);

        gradient->setStops(stops);
        gradient->setValid(true);
        gradient->setPermanent(true);
        addResource(gradient, false);

        m_foregroundToBackground = gradient;
    }

private:

    friend class KoResourceBundle;

    KoAbstractGradientSP createResource( const QString & filename ) {

        QString fileExtension;
        int index = filename.lastIndexOf('.');

        if (index != -1)
            fileExtension = filename.mid(index).toLower();

        KoAbstractGradientSP grad;

        if(fileExtension == ".svg") {
            grad.reset(new KoStopGradient(filename));
        }
        else if(fileExtension == ".ggr" ) {
            grad.reset(new KoSegmentGradient(filename));
        }

        return grad;
    }

    KoAbstractGradientSP m_foregroundToTransparent;
    KoAbstractGradientSP m_foregroundToBackground;
};

struct Q_DECL_HIDDEN KoResourceServerProvider::Private
{
    KoResourceServer<KoPattern> *patternServer;
    KoResourceServer<KoAbstractGradient> *gradientServer;
    KoResourceServer<KoColorSet> *paletteServer;
    KoResourceServer<KoSvgSymbolCollectionResource> *svgSymbolCollectionServer;
    KoResourceServer<KoGamutMask> *gamutMaskServer;
#if defined HAVE_SEEXPR
    KoResourceServer<KisSeExprScript>* seExprScriptServer;
#endif
};

KoResourceServerProvider::KoResourceServerProvider() : d(new Private)
{
    d->patternServer = new KoResourceServer<KoPattern>(ResourceType::Patterns);
    d->gradientServer = new GradientResourceServer(ResourceType::Gradients);
    d->paletteServer = new KoResourceServer<KoColorSet>(ResourceType::Palettes);
    d->svgSymbolCollectionServer = new KoResourceServer<KoSvgSymbolCollectionResource>(ResourceType::Symbols);
    d->gamutMaskServer = new KoResourceServer<KoGamutMask>(ResourceType::GamutMasks);
#if defined HAVE_SEEXPR
    d->seExprScriptServer = new KoResourceServer<KisSeExprScript>(ResourceType::SeExprScripts);
#endif
}

KoResourceServerProvider::~KoResourceServerProvider()
{
    delete d->patternServer;
    delete d->gradientServer;
    delete d->paletteServer;
    delete d->svgSymbolCollectionServer;
    delete d->gamutMaskServer;
#if defined HAVE_SEEXPR
    delete d->seExprScriptServer;
#endif    

    delete d;
}

Q_GLOBAL_STATIC(KoResourceServerProvider, s_instance)

KoResourceServerProvider *KoResourceServerProvider::instance()
{
    return s_instance;
}

KoResourceServer<KoPattern> *KoResourceServerProvider::patternServer()
{
    return KoResourceServerProvider::instance()->d->patternServer;
}

KoResourceServer<KoAbstractGradient> *KoResourceServerProvider::gradientServer()
{
    return KoResourceServerProvider::instance()->d->gradientServer;
}

KoResourceServer<KoColorSet> *KoResourceServerProvider::paletteServer()
{
    return KoResourceServerProvider::instance()->d->paletteServer;
}

KoResourceServer<KoSvgSymbolCollectionResource> *KoResourceServerProvider::svgSymbolCollectionServer()
{
    return KoResourceServerProvider::instance()->d->svgSymbolCollectionServer;
}

KoResourceServer<KoGamutMask> *KoResourceServerProvider::gamutMaskServer()
{
    return KoResourceServerProvider::instance()->d->gamutMaskServer;
}

#if defined HAVE_SEEXPR
KoResourceServer<KisSeExprScript> *KoResourceServerProvider::seExprScriptServer()
{
    return KoResourceServerProvider::instance()->d->seExprScriptServer;
}
#endif
