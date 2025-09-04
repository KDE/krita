/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisPlatformPluginInterfaceFactory.h"

#include <KoPluginLoader.h>
#include <kpluginfactory.h>
#include <QApplication>
#include <QWindow>

#include <kis_assert.h>

#include <input/KisExtendedModifiersMapperPluginInterface.h>

#if KRITA_USE_SURFACE_COLOR_MANAGEMENT_API

#include <surfacecolormanagement/KisSurfaceColorManagerInterface.h>

KisSurfaceColorManagerInterface* KisPlatformPluginInterfaceFactory::createSurfaceColorManager(QWindow *nativeWindow)
{
    KPluginFactory *factory = KoPluginLoader::instance()->loadSinglePlugin(
        std::make_pair("X-Krita-PlatformId", QApplication::platformName()),
        "Krita/PlatformPlugin");

    if (factory) {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(nativeWindow, nullptr);

        QVariantList args = {QVariant::fromValue(nativeWindow)};

        return factory->create<KisSurfaceColorManagerInterface>(nullptr, args);
    }

    return nullptr;
}

#endif

KisExtendedModifiersMapperPluginInterface* KisPlatformPluginInterfaceFactory::createExtendedModifiersMapper()
{
    KPluginFactory *factory = KoPluginLoader::instance()->loadSinglePlugin(
        std::make_pair("X-Krita-PlatformId", QApplication::platformName()),
        "Krita/PlatformPlugin");

    if (factory) {
        return factory->create<KisExtendedModifiersMapperPluginInterface>();
    }

    return nullptr;
}
