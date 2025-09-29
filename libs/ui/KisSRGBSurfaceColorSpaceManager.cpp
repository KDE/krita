/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisSRGBSurfaceColorSpaceManager.h"

#include <QWidget>
#include <QWindow>

#include <kis_assert.h>
#include <kis_config_notifier.h>

#include <KisPlatformPluginInterfaceFactory.h>
#include <surfacecolormanagement/KisSurfaceColorManagerInterface.h>


KisSRGBSurfaceColorSpaceManager::KisSRGBSurfaceColorSpaceManager(KisSurfaceColorManagerInterface *interface, QObject *parent)
    : KisCanvasSurfaceColorSpaceManager(interface, KisConfig::CanvasSurfaceMode::Rec709g22, KisDisplayConfig::optionsFromKisConfig(KisConfig(true)), parent)
{
    connect(KisConfigNotifier::instance(), &KisConfigNotifier::configChanged, this, &KisSRGBSurfaceColorSpaceManager::slotConfigChanged);
}

KisSRGBSurfaceColorSpaceManager::~KisSRGBSurfaceColorSpaceManager()
{
}

void KisSRGBSurfaceColorSpaceManager::slotConfigChanged()
{
    setDisplayConfigOptions(KisDisplayConfig::optionsFromKisConfig(KisConfig(true)));
}

KisSRGBSurfaceColorSpaceManager* KisSRGBSurfaceColorSpaceManager::tryCreateForCurrentPlatform(QWidget *widget)
{
    QWindow *nativeWindow = widget->windowHandle();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(widget->windowHandle(), nullptr);

    std::unique_ptr<KisSurfaceColorManagerInterface> iface(
        KisPlatformPluginInterfaceFactory::instance()->createSurfaceColorManager(widget->windowHandle()));

    if (iface) {
        return new KisSRGBSurfaceColorSpaceManager(iface.release(), nativeWindow);
    }

    return nullptr;
}

#include <moc_KisSRGBSurfaceColorSpaceManager.cpp>
