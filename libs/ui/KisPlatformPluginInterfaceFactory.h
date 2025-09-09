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

class KRITAUI_EXPORT KisPlatformPluginInterfaceFactory {
public:
#if KRITA_USE_SURFACE_COLOR_MANAGEMENT_API
    /**
     * Creates an instance of the color manager interface using the platform plugin
     *
     * If the current platform plugin doesn't profide this interface, returns nullptr
     */
    static KisSurfaceColorManagerInterface* createSurfaceColorManager(QWindow *nativeWindow);
#endif

    /**
     * Creates an instance of the extended modifiers interface using the platform plugin
     *
     * If the current platform plugin doesn't profide this interface, returns nullptr
     */
    static KisExtendedModifiersMapperPluginInterface* createExtendedModifiersMapper();
};

#endif // KISPLATFORMPLUGININTERFACEFACTORY_H
