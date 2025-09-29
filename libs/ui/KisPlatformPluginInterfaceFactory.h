/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISPLATFORMPLUGININTERFACEFACTORY_H
#define KISPLATFORMPLUGININTERFACEFACTORY_H

#include <kritaui_export.h>

#include <config-use-surface-color-management-api.h>

class QWindow;
class KisSurfaceColorManagerInterface;
class KisExtendedModifiersMapperPluginInterface;

#if KRITA_USE_SURFACE_COLOR_MANAGEMENT_API
class KisSurfaceColorManagementInfo;
#endif /* KRITA_USE_SURFACE_COLOR_MANAGEMENT_API */

class KRITAUI_EXPORT KisPlatformPluginInterfaceFactory {
public:
    KisPlatformPluginInterfaceFactory();

    static KisPlatformPluginInterfaceFactory* instance();

#if KRITA_USE_SURFACE_COLOR_MANAGEMENT_API
    /**
     * Creates an instance of the color manager interface using the platform plugin
     *
     * If the current platform plugin doesn't profide this interface, returns nullptr
     */
    KisSurfaceColorManagerInterface *createSurfaceColorManager(QWindow *nativeWindow);
#endif /* KRITA_USE_SURFACE_COLOR_MANAGEMENT_API */

    bool surfaceColorManagedByOS();

    /**
     * Creates an instance of the extended modifiers interface using the platform plugin
     *
     * If the current platform plugin doesn't profide this interface, returns nullptr
     */
    KisExtendedModifiersMapperPluginInterface* createExtendedModifiersMapper();

private:
    bool m_surfaceColorManagedByOS {false};
};

#endif // KISPLATFORMPLUGININTERFACEFACTORY_H
