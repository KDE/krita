/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISPLATFORMPLUGININTERFACEFACTORY_H
#define KISPLATFORMPLUGININTERFACEFACTORY_H

#include <kritaui_export.h>

#include <config-use-surface-color-management-api.h>

class QString;
class QWidget;
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

    /**
     * Returns true if Krita is running on a system that manages the color
     * space of the underlying surface. If it is true, the canvas should use
     * KisCanvasSurfaceColorSpaceManager and all other windows should use
     * KisSRGBSurfaceColorSpaceManager to set up the color space properly.
     */
    bool surfaceColorManagedByOS();

    /**
     * Return user-facing information about color management status of the
     * main window surface. The information is intentionally left untranslated,
     * since it is supposed to be used for bugreports.
     */
    QString colorManagementReport(QWidget *widget);

    /**
     * Return user-facing information about the preferred color space of the
     * operating system. This information is supposed to be used in the
     * preferences dialog, so it is supposed to be translated.
     */
    QString osPreferredColorSpaceReport(QWidget *widget);

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
