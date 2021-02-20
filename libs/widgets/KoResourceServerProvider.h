/*  This file is part of the KDE project

    SPDX-FileCopyrightText: 1999 Matthias Elter <elter@kde.org>
    SPDX-FileCopyrightText: 2003 Patrick Julien <freak@codepimps.org>
    SPDX-FileCopyrightText: 2005 Sven Langkamp <sven.langkamp@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KORESOURCESERVERPROVIDER_H
#define KORESOURCESERVERPROVIDER_H

#include <kritawidgets_export.h>

#include <QThread>

#include <WidgetsDebug.h>

#include "KoResourceServer.h"
#include <resources/KoPattern.h>
#include <resources/KoAbstractGradient.h>
#include <resources/KoColorSet.h>
#include <resources/KoSvgSymbolCollectionResource.h>
#include <resources/KoGamutMask.h>
#include <config-seexpr.h>
#if defined HAVE_SEEXPR
#include <resources/KisSeExprScript.h>
#endif

/**
 * Provides default resource servers for gradients, patterns and palettes
 */
class KRITAWIDGETS_EXPORT KoResourceServerProvider : public QObject
{
    Q_OBJECT

public:
    KoResourceServerProvider();
    ~KoResourceServerProvider() override;

    static KoResourceServerProvider* instance();

    static KoResourceServer<KoPattern> *patternServer();
    static KoResourceServer<KoAbstractGradient> *gradientServer();
    static KoResourceServer<KoColorSet> *paletteServer();
    static KoResourceServer<KoSvgSymbolCollectionResource> *svgSymbolCollectionServer();
    static KoResourceServer<KoGamutMask> *gamutMaskServer();
#if defined HAVE_SEEXPR
    static KoResourceServer<KisSeExprScript> *seExprScriptServer();
#endif

private:
    KoResourceServerProvider(const KoResourceServerProvider&);
    KoResourceServerProvider operator=(const KoResourceServerProvider&);

private:
    struct Private;
    Private* const d;
};

#endif // KORESOURCESERVERPROVIDER_H
