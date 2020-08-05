/*
 *  Copyright (c) 2020 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef TESTRESOURCES_H
#define TESTRESOURCES_H

#include <KoConfig.h>
#include <QApplication>
#include <QTest>
#include <QLoggingCategory>
#include <QtTest/qtestsystem.h>
#include <set>

#if defined(QT_NETWORK_LIB)
#  include <QtTest/qtest_network.h>
#endif
#include <QtTest/qtest_widgets.h>

#ifdef QT_KEYPAD_NAVIGATION
#  define QTEST_DISABLE_KEYPAD_NAVIGATION QApplication::setNavigationMode(Qt::NavigationModeNone);
#else
#  define QTEST_DISABLE_KEYPAD_NAVIGATION
#endif

#if defined (TESTRESOURCES) || defined(TESTPIGMENT) || defined (TESTFLAKE) || defined(TESTBRUSH) || || defined(TESTIMAGE) || defined(TESTUI)
#include <KisResourceLoaderRegistry.h>
#include <QImageReader>
#include <QList>
#include <QByteArray>
#include <QStringList>
#include <KisMimeDatabase.h>
#include <KisResourceLoader.h>
#include <KoPattern>
#endif

#if defined(TESTPIGMENT) || defined (TESTFLAKE) || defined(TESTBRUSH) || || defined(TESTIMAGE) || defined(TESTUI)
#include <KoSegmentGradient.h>
#include <KoStopGradient.h>
#include <KoColorSet.h>
#endif

#if defined (TESTFLAKE) || defined(TESTBRUSH) || defined(TESTIMAGE) || defined(TESTUI)
#if defined HAVE_SEEXPR
#include <KisSeExprScript.h>
#endif
#include <KoGamutMask.h>
#include <KoSvgSymbolCollectionResource.h>
#endif

#if defined(TESTBRUSH) || defined(TESTIMAGE) || defined(TESTUI)
#include <kis_gbr_brush.h>
#include <kis_imagepipe_brush.h>
#include <kis_svg_brush.h>
#include <kis_png_brush.h>
#endif

#if defined(TESTIMAGE) || defined(TESTUI)
#include <kis_paintop_preset.h>
#include <kis_psd_layer_style.h>
#endif

#if defined(TESTUI)
#include <KisWindowLayoutResource.h>
#include <kis_workspace_resource.h>
#include <KisSessionResource.h>
#endif

void registerResources()
{

#if defined (TESTRESOURCES) || defined(TESTPIGMENT) || defined (TESTFLAKE) || defined(TESTBRUSH) || || defined(TESTIMAGE) || defined(TESTUI)

    KisResourceLoaderRegistry *reg = KisResourceLoaderRegistry::instance();

    QList<QByteArray> src = QImageReader::supportedMimeTypes();
    QStringList allImageMimes;
    Q_FOREACH(const QByteArray ba, src) {
        if (QImageWriter::supportedMimeTypes().contains(ba)) {
            allImageMimes << QString::fromUtf8(ba);
        }
    }
    allImageMimes << KisMimeDatabase::mimeTypeForSuffix("pat");

    reg->add(new KisResourceLoader<KoPattern>(ResourceType::Patterns, ResourceType::Patterns, i18n("Patterns"), allImageMimes));
#endif

#if defined(TESTPIGMENT) || defined (TESTFLAKE) || defined(TESTBRUSH) || || defined(TESTIMAGE) || defined(TESTUI)

    reg->add(new KisResourceLoader<KoSegmentGradient>(ResourceSubType::SegmentedGradients, ResourceType::Gradients, i18n("Gradients"), QStringList() << "application/x-gimp-gradient"));
    reg->add(new KisResourceLoader<KoStopGradient>(ResourceSubType::StopGradients, ResourceType::Gradients, i18n("Gradients"), QStringList() << "application/x-karbon-gradient" << "image/svg+xml"));

    reg->add(new KisResourceLoader<KoColorSet>(ResourceType::Palettes, ResourceType::Palettes, i18n("Palettes"),
                                     QStringList() << KisMimeDatabase::mimeTypeForSuffix("kpl")
                                               << KisMimeDatabase::mimeTypeForSuffix("gpl")
                                               << KisMimeDatabase::mimeTypeForSuffix("pal")
                                               << KisMimeDatabase::mimeTypeForSuffix("act")
                                               << KisMimeDatabase::mimeTypeForSuffix("aco")
                                               << KisMimeDatabase::mimeTypeForSuffix("css")
                                               << KisMimeDatabase::mimeTypeForSuffix("colors")
                                               << KisMimeDatabase::mimeTypeForSuffix("xml")
                                               << KisMimeDatabase::mimeTypeForSuffix("sbz")));
#endif

#if defined (TESTFLAKE) || defined(TESTBRUSH) || defined(TESTIMAGE) || defined(TESTUI)
#if defined HAVE_SEEXPR
    reg->add(new KisResourceLoader<KisSeExprScript>(ResourceType::SeExprScripts, ResourceType::SeExprScripts, i18n("SeExpr Scripts"), QStringList() << "application/x-krita-seexpr-script"));
#endif
    reg->add(new KisResourceLoader<KoGamutMask>(ResourceType::GamutMasks, ResourceType::GamutMasks, i18n("Gamut masks"), QStringList() << "application/x-krita-gamutmasks"));
    reg->add(new KisResourceLoader<KoSvgSymbolCollectionResource>(ResourceType::Symbols, ResourceType::Symbols, i18n("SVG symbol libraries"), QStringList() << "image/svg+xml"));
#endif

#if defined(TESTBRUSH) || defined(TESTIMAGE) || defined(TESTUI)

    reg->add(new KisResourceLoader<KisGbrBrush>(ResourceSubType::GbrBrushes, ResourceType::Brushes, i18n("Brush tips"), QStringList() << "image/x-gimp-brush"));
    reg->add(new KisResourceLoader<KisImagePipeBrush>(ResourceSubType::GihBrushes, ResourceType::Brushes, i18n("Brush tips"), QStringList() << "image/x-gimp-brush-animated"));
    reg->add(new KisResourceLoader<KisSvgBrush>(ResourceSubType::SvgBrushes, ResourceType::Brushes, i18n("Brush tips"), QStringList() << "image/svg+xml"));
    reg->add(new KisResourceLoader<KisPngBrush>(ResourceSubType::PngBrushes, ResourceType::Brushes, i18n("Brush tips"), QStringList() << "image/png"));

#endif

#if defined(TESTIMAGE) || defined(TESTUI)
     reg->add(new KisResourceLoader<KisPaintOpPreset>(ResourceType::PaintOpPresets, ResourceType::PaintOpPresets, i18n("Brush presets"), QStringList() << "application/x-krita-paintoppreset"));
     reg->add(new KisResourceLoader<KisPSDLayerStyle>(ResourceType::LayerStyles,
                                                     ResourceType::LayerStyles,
                                                     ResourceType::LayerStyles,
                                                     QStringList() << "application/x-photoshop-style"));
#endif


#if defined(TESTUI)
    reg->add(new KisResourceLoader<KisWindowLayoutResource>(ResourceType::WindowLayouts, ResourceType::WindowLayouts, i18n("Window layouts"), QStringList() << "application/x-krita-windowlayout"));
    reg->add(new KisResourceLoader<KisSessionResource>(ResourceType::Sessions, ResourceType::Sessions, i18n("Sessions"), QStringList() << "application/x-krita-session"));
    reg->add(new KisResourceLoader<KisWorkspaceResource>(ResourceType::Workspaces, ResourceType::Workspaces, i18n("Workspaces"), QStringList() << "application/x-krita-workspace"));
#endif

#if defined (TESTRESOURCES) || defined(TESTPIGMENT) || defined (TESTFLAKE) || defined(TESTBRUSH) || defined(TESTIMAGE) || defined(TESTUI)
    if (!KisResourceCacheDb::initialize(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))) {
        qFatal("Could not initialize the resource cachedb");
    }

    KisResourceLocator::LocatorError r = KisResourceLocator::instance()->initialize(KoResourcePaths::getApplicationRoot() + "/share/krita");
#endif

}

#endif // TESTRESOURCES_H
