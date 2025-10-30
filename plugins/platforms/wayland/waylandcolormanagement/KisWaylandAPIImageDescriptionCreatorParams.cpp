/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisWaylandAPIImageDescriptionCreatorParams.h"

#include "KisWaylandAPIColorManager.h"
#include "KisWaylandAPIImageDescription.h"
#include "KisWaylandSurfaceColorimetry.h"

KisWaylandAPIImageDescriptionCreatorParams::KisWaylandAPIImageDescriptionCreatorParams(KisWaylandAPIColorManager *colorManager)
    : KisWaylandAPIImageDescriptionCreatorParams(colorManager->create_parametric_creator(), colorManager)
{
}

KisWaylandAPIImageDescriptionCreatorParams::KisWaylandAPIImageDescriptionCreatorParams(::wp_image_description_creator_params_v1 *params,
                                                                                 KisWaylandAPIColorManager *colorManager)
    : QtWayland::wp_image_description_creator_params_v1(params)
    , m_colorManager(colorManager)
{
}
KisWaylandAPIImageDescriptionCreatorParams::~KisWaylandAPIImageDescriptionCreatorParams()
{
    // object() is null if the information creation has succeeded
    if (object()) {
        wp_image_description_creator_params_v1_destroy(object());
    }
}

std::unique_ptr<KisWaylandAPIImageDescriptionNoInfo> KisWaylandAPIImageDescriptionCreatorParams::createImageDescription(const KisSurfaceColorimetry::WaylandSurfaceDescription &data)
{
    if (data.tfNamed) {
        set_tf_named(static_cast<uint32_t>(*data.tfNamed));
    } else if (data.tfGamma) {
        Q_ASSERT(m_colorManager->isFeatureSupported(KisWaylandAPIColorManager::feature_set_tf_power));
        set_tf_power(*data.tfGamma);
    }

    if (data.namedContainer) {
        set_primaries_named(static_cast<uint32_t>(*data.namedContainer));
    } else {
        Q_ASSERT(m_colorManager->isFeatureSupported(KisWaylandAPIColorManager::feature_set_primaries));
        set_primaries(std::rint(data.container->red.x * 1'000'000.00),
                      std::rint(data.container->red.y * 1'000'000.00),
                      std::rint(data.container->green.x * 1'000'000.00),
                      std::rint(data.container->green.y * 1'000'000.00),
                      std::rint(data.container->blue.x * 1'000'000.00),
                      std::rint(data.container->blue.y * 1'000'000.00),
                      std::rint(data.container->white.x * 1'000'000.00),
                      std::rint(data.container->white.y * 1'000'000.00));
    }

    if (data.target) {
        Q_ASSERT(m_colorManager->isFeatureSupported(KisWaylandAPIColorManager::feature_set_mastering_display_primaries));
        set_mastering_display_primaries(std::rint(data.target->red.x * 1'000'000.00),
                                        std::rint(data.target->red.y * 1'000'000.00),
                                        std::rint(data.target->green.x * 1'000'000.00),
                                        std::rint(data.target->green.y * 1'000'000.00),
                                        std::rint(data.target->blue.x * 1'000'000.00),
                                        std::rint(data.target->blue.y * 1'000'000.00),
                                        std::rint(data.target->white.x * 1'000'000.00),
                                        std::rint(data.target->white.y * 1'000'000.00));
    }

    if (data.luminances) {
        Q_ASSERT(m_colorManager->isFeatureSupported(KisWaylandAPIColorManager::feature_set_luminances));
        set_luminances(data.luminances->minLuminance,
                       data.luminances->maxLuminance,
                       data.luminances->referenceLuminance);
    }

    if (data.masteringLuminance) {
        Q_ASSERT(m_colorManager->isFeatureSupported(KisWaylandAPIColorManager::feature_extended_target_volume));
        set_mastering_luminance(data.masteringLuminance->minLuminance, data.masteringLuminance->maxLuminance);
    }

    if (data.targetMaxCLL) {
        set_max_cll(*data.targetMaxCLL);
    }

    if (data.targetMaxFALL) {
        set_max_fall(*data.targetMaxFALL);
    }

    if (data.iccFileIsPresent) {
        // We do not support ICC files yet
        qWarning() << "ICC file is not supported yet.";
    }

    auto result = std::make_unique<KisWaylandAPIImageDescriptionNoInfo>(create());

    init(nullptr);

    return result;
}

#include <moc_KisWaylandAPIImageDescriptionCreatorParams.cpp>