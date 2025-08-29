/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisSRGBSurfaceColorSpaceManager.h"

#include <kis_assert.h>
#include <kis_config.h>
#include <kis_config_notifier.h>

#include <surfacecolormanagement/KisSurfaceColorManagerInterface.h>


KisSRGBSurfaceColorSpaceManager::KisSRGBSurfaceColorSpaceManager(KisSurfaceColorManagerInterface *interface, QObject *parent)
    : QObject(parent)
    , m_interface(interface)
{
    connect(m_interface.data(), &KisSurfaceColorManagerInterface::sigReadyChanged, this, &KisSRGBSurfaceColorSpaceManager::slotInterfaceReadyChanged);
    connect(KisConfigNotifier::instance(), &KisConfigNotifier::configChanged, this, &KisSRGBSurfaceColorSpaceManager::slotConfigChanged);
}

KisSRGBSurfaceColorSpaceManager::~KisSRGBSurfaceColorSpaceManager()
{
}

KisSurfaceColorimetry::RenderIntent KisSRGBSurfaceColorSpaceManager::calculateConfigIntent() {
    using KisSurfaceColorimetry::RenderIntent;

    KisConfig cfg(true);
    RenderIntent intent = RenderIntent::render_intent_perceptual;

    switch (cfg.monitorRenderIntent()) {
        case INTENT_PERCEPTUAL:
            // default value
            break;
        case INTENT_RELATIVE_COLORIMETRIC:
            intent =
                cfg.useBlackPointCompensation() ?
                RenderIntent::render_intent_relative_bpc :
                RenderIntent::render_intent_relative;
            break;
        case INTENT_SATURATION:
            intent =
                RenderIntent::render_intent_saturation;
            break;
        case INTENT_ABSOLUTE_COLORIMETRIC:
            intent =
                RenderIntent::render_intent_absolute;
            break;
    }
    return intent;
}

void KisSRGBSurfaceColorSpaceManager::slotConfigChanged() {
    if (m_interface->isReady() && calculateConfigIntent() != m_interface->renderingIntent()) {
        reinitializeSurfaceDescription();
    }
}

void KisSRGBSurfaceColorSpaceManager::slotInterfaceReadyChanged(bool isReady) {
    if (isReady) {
        reinitializeSurfaceDescription();
    }
}

void KisSRGBSurfaceColorSpaceManager::reinitializeSurfaceDescription() {
    using KisSurfaceColorimetry::RenderIntent;
    using KisSurfaceColorimetry::SurfaceDescription;
    using KisSurfaceColorimetry::NamedPrimaries;
    using KisSurfaceColorimetry::NamedTransferFunction;

    RenderIntent preferredIntent = calculateConfigIntent();

    if (!m_interface->supportsRenderIntent(preferredIntent)) {
        qWarning() << "WARNING: failed to set user preferred rendering"
                   << "intent for the surface, intent \""
                   << preferredIntent << "\" is unsupported, falling back to \"perceptual\"";

        preferredIntent = RenderIntent::render_intent_perceptual;

        // perceptual intent is guaranteed to be supported by the compositor
        KIS_SAFE_ASSERT_RECOVER_RETURN(m_interface->supportsRenderIntent(preferredIntent));
    }

    SurfaceDescription preferredDescription;
    preferredDescription.colorSpace.primaries = NamedPrimaries::primaries_srgb;
    preferredDescription.colorSpace.transferFunction = NamedTransferFunction::transfer_function_srgb;

    if (m_interface->supportsSurfaceDescription(preferredDescription)) {
        auto future = m_interface->setSurfaceDescription(preferredDescription, preferredIntent);
        future.then([] (QFuture<bool> result) {
            if (!result.isValid() || !result.result()) {
                qWarning() << "WARNING: failed to set sRGB color space for the surface, setSurfaceDescription() returned false";
            }
        });
    } else {
        qWarning() << "WARNING: failed to set sRGB color space for the surface,"
                   << "the color space is unsupported by compositor";
    }
}

#include <moc_KisSRGBSurfaceColorSpaceManager.cpp>
