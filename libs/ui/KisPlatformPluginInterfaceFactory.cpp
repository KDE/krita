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

#include <KisRootSurfaceInfoProxy.h>
#include <surfacecolormanagement/KisSurfaceColorManagerInterface.h>
#include <surfacecolormanagement/KisSurfaceColorManagementInfo.h>

#endif /* KRITA_USE_SURFACE_COLOR_MANAGEMENT_API */

Q_GLOBAL_STATIC(KisPlatformPluginInterfaceFactory, s_instance)

KisPlatformPluginInterfaceFactory::KisPlatformPluginInterfaceFactory()
{
#if KRITA_USE_SURFACE_COLOR_MANAGEMENT_API

    auto fetchSurfaceColorManagedByOS = []() {
        KPluginFactory *factory = KoPluginLoader::instance()->loadSinglePlugin(
            std::make_pair("X-Krita-PlatformId", QApplication::platformName()),
            "Krita/PlatformPlugin");

        if (factory) {
            QScopedPointer<KisSurfaceColorManagementInfo> interface(factory->create<KisSurfaceColorManagementInfo>());
            return interface && interface->surfaceColorManagedByOS();
        }

        return false;
    };

    m_surfaceColorManagedByOS = fetchSurfaceColorManagedByOS();

#endif /* KRITA_USE_SURFACE_COLOR_MANAGEMENT_API */
}

KisPlatformPluginInterfaceFactory* KisPlatformPluginInterfaceFactory::instance()
{
    return s_instance;
}

#if KRITA_USE_SURFACE_COLOR_MANAGEMENT_API

KisSurfaceColorManagerInterface* KisPlatformPluginInterfaceFactory::createSurfaceColorManager(QWindow *nativeWindow)
{
    if (!surfaceColorManagedByOS()) {
        return nullptr;
    }

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

bool KisPlatformPluginInterfaceFactory::surfaceColorManagedByOS()
{
    return m_surfaceColorManagedByOS;
}

QString KisPlatformPluginInterfaceFactory::colorManagementReport(QWidget *widget)
{
#if KRITA_USE_SURFACE_COLOR_MANAGEMENT_API
    if (KisPlatformPluginInterfaceFactory::instance()->surfaceColorManagedByOS()) {
        KisRootSurfaceInfoProxy proxy(widget);
        KIS_SAFE_ASSERT_RECOVER_NOOP(proxy.isReady());
        return proxy.colorManagementReport();
    } else {
        return "Surface color management is not supported on this platform\n";
    }
#else
    return "Surface color management is disabled\n";
#endif
}

QString KisPlatformPluginInterfaceFactory::osPreferredColorSpaceReport(QWidget *widget)
{
#if KRITA_USE_SURFACE_COLOR_MANAGEMENT_API
    if (KisPlatformPluginInterfaceFactory::instance()->surfaceColorManagedByOS()) {
        KisRootSurfaceInfoProxy proxy(widget);
        KIS_SAFE_ASSERT_RECOVER_NOOP(proxy.isReady());
        return proxy.osPreferredColorSpaceReport();
    } else {
        return "Surface color management is not supported on this platform\n";
    }
#else
    return "Surface color management is disabled\n";
#endif
}


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
