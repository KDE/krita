/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisCanvasSurfaceColorSpaceManager.h"

#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>

#include <kis_config.h>
#include <kis_config_notifier.h>

#include <KisDisplayConfig.h>
#include <KisProofingConfiguration.h>

#include <surfacecolormanagement/KisSurfaceColorManagerInterface.h>

KisCanvasSurfaceColorSpaceManager::KisCanvasSurfaceColorSpaceManager(KisSurfaceColorManagerInterface *interface, QObject *parent)
    : QObject(parent)
    , m_interface(interface)
{
    KisConfig cfg(true);
    m_currentConfig = KisDisplayConfig(-1, cfg);
    m_currentConfig.profile = KoColorSpaceRegistry::instance()->p709SRGBProfile();
    m_surfaceMode = cfg.canvasSurfaceColorSpaceManagementMode();

    connect(m_interface.data(), &KisSurfaceColorManagerInterface::sigReadyChanged, this, &KisCanvasSurfaceColorSpaceManager::slotInterfaceReadyChanged);
    connect(m_interface.data(), &KisSurfaceColorManagerInterface::sigPreferredSurfaceDescriptionChanged, this, &KisCanvasSurfaceColorSpaceManager::slotInterfacePreferredDescriptionChanged);
    connect(KisConfigNotifier::instance(), &KisConfigNotifier::configChanged, this, &KisCanvasSurfaceColorSpaceManager::slotConfigChanged);
}

KisCanvasSurfaceColorSpaceManager::~KisCanvasSurfaceColorSpaceManager()
{
}

QString KisCanvasSurfaceColorSpaceManager::colorManagementReport() const
{
    QString report;
    QDebug str(&report);

    str << "(canvas surface color manager)" << Qt::endl;
    str << Qt::endl;

    if (!m_interface->isReady()) {
        str << "WARNING: surface color management interface is not ready!" << Qt::endl;
        str << Qt::endl;
    }

    using KisSurfaceColorimetry::RenderIntent;
    using KisSurfaceColorimetry::SurfaceDescription;
    using KisSurfaceColorimetry::NamedPrimaries;
    using KisSurfaceColorimetry::NamedTransferFunction;

    RenderIntent preferredIntent = calculateConfigIntent(m_currentConfig);
    str << "Configured intent:" << preferredIntent << "supported:" << m_interface->supportsRenderIntent(preferredIntent) << Qt::endl;

    str << "Proofing intent:";
    if (m_proofingIntentOverride) {
        str << *m_proofingIntentOverride << "supported:" << m_interface->supportsRenderIntent(*m_proofingIntentOverride) << Qt::endl;
    } else {
        str << "<none>" << Qt::endl;
    }

    str << "Actual intent:";
    if (m_interface->renderingIntent()) {
        str << *m_interface->renderingIntent() << Qt::endl;
    } else {
        str << "<none>" << Qt::endl;
    }
    str << Qt::endl;

    str << "Active surface description:";
    if (m_interface->surfaceDescription()) {
        str << Qt::endl;
        str.noquote() << m_interface->surfaceDescription()->makeTextReport() << Qt::endl;
    } else {
        str << "<none>" << Qt::endl;
    }
    str << Qt::endl;

    str << "Selected Profile:";
    if (m_currentConfig.profile) {
        auto profile = m_currentConfig.profile;

        str << profile->name() << Qt::endl;
        str << "    primaries:" << KoColorProfile::getColorPrimariesName(profile->getColorPrimaries()) << Qt::endl;
        str << "    transfer: " << KoColorProfile::getTransferCharacteristicName(profile->getTransferCharacteristics()) << Qt::endl;
        str << Qt::endl;

        {
            auto colVec = profile->getColorantsxyY();
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(colVec.size() == 9, report);
            KisColorimetryUtils::xyY colR{colVec[0], colVec[1], colVec[2]};
            KisColorimetryUtils::xyY colG{colVec[3], colVec[4], colVec[5]};
            KisColorimetryUtils::xyY colB{colVec[6], colVec[7], colVec[8]};

            str << "    red:  " << colR << Qt::endl;
            str << "    green:" << colG << Qt::endl;
            str << "    blue: " << colB << Qt::endl;
        }

        {
            auto whiteVec = profile->getWhitePointxyY();
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(whiteVec.size() == 3, report);
            KisColorimetryUtils::xyY white{whiteVec[0], whiteVec[1], whiteVec[2]};

            str << "    white: " << white << Qt::endl;
            str << Qt::endl;
        }

    } else {
        str << "<none>" << Qt::endl;
    }

    str << "Compositor preferred surface description:";
    if (m_interface->preferredSurfaceDescription()) {
        str << Qt::endl;
        str.noquote() << m_interface->preferredSurfaceDescription()->makeTextReport() << Qt::endl;
    } else {
        str << "<none>" << Qt::endl;
    }
    str << Qt::endl;

    return report;
}

KisSurfaceColorimetry::RenderIntent KisCanvasSurfaceColorSpaceManager::calculateConfigIntent(int kritaIntent, bool useBlackPointCompensation)
{
    using KisSurfaceColorimetry::RenderIntent;

    KisConfig cfg(true);
    RenderIntent intent = RenderIntent::render_intent_perceptual;

    switch (kritaIntent) {
        case INTENT_PERCEPTUAL:
            // default value
            break;
        case INTENT_RELATIVE_COLORIMETRIC:
            intent =
                useBlackPointCompensation ?
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

KisSurfaceColorimetry::RenderIntent KisCanvasSurfaceColorSpaceManager::calculateConfigIntent(const KisDisplayConfig &config)
{
    return calculateConfigIntent(config.intent,
        config.conversionFlags.testFlag(KoColorConversionTransformation::BlackpointCompensation));
}

void KisCanvasSurfaceColorSpaceManager::slotConfigChanged()
{
    KisDisplayConfig oldDisplayConfig = m_currentConfig;

    KisConfig cfg(true);
    KisDisplayConfig config(-1, cfg);

    KisSurfaceColorimetry::RenderIntent newIntent =
        calculateConfigIntent(config.intent,
            config.conversionFlags.testFlag(KoColorConversionTransformation::BlackpointCompensation));

    if (newIntent != m_interface->renderingIntent() ||
        m_surfaceMode != cfg.canvasSurfaceColorSpaceManagementMode()) {

        m_currentConfig.intent = config.intent;
        m_currentConfig.conversionFlags = config.conversionFlags;
        m_surfaceMode = cfg.canvasSurfaceColorSpaceManagementMode();

        if (m_interface->isReady()) {
            reinitializeSurfaceDescription();
        }
    }

    config.profile = m_currentConfig.profile;

    if (config != oldDisplayConfig) {
        m_currentConfig = config;
        Q_EMIT sigDisplayConfigChanged(m_currentConfig);
    }
}

void KisCanvasSurfaceColorSpaceManager::setProofingConfiguration(KisProofingConfigurationSP proofingConfig)
{
    /**
     * WARNING: this code duplicates logic from KisOpenGLUpdateInfoBuilder::buildUpdateInfo()
     * and KisProofingConfiguration::determineDisplayIntent() and
     * KisProofingConfiguration::determineDisplayFlags. Ideally, they should be unified.
     */

    std::optional<KisSurfaceColorimetry::RenderIntent> newRenderingIntent;

    if (proofingConfig && proofingConfig->displayFlags.testFlag(KoColorConversionTransformation::SoftProofing)) {
        if (proofingConfig->displayMode == KisProofingConfiguration::Paper) {
            newRenderingIntent = KisSurfaceColorimetry::RenderIntent::render_intent_absolute;
        } else if (proofingConfig->displayMode == KisProofingConfiguration::Custom) {
            newRenderingIntent =
                calculateConfigIntent(proofingConfig->displayIntent,
                    proofingConfig->displayFlags.testFlag(KoColorConversionTransformation::BlackpointCompensation));
        }
    }

    if (m_proofingIntentOverride != newRenderingIntent) {
        m_proofingIntentOverride = newRenderingIntent;

        if (m_interface->isReady()) {
            reinitializeSurfaceDescription();
        }
    }
}

void KisCanvasSurfaceColorSpaceManager::slotInterfaceReadyChanged(bool isReady)
{
    if (isReady) {
        reinitializeSurfaceDescription();
    }
}

void KisCanvasSurfaceColorSpaceManager::slotInterfacePreferredDescriptionChanged()
{
    reinitializeSurfaceDescription();
}

#include <KoColorProfile.h>
#include <surfacecolormanagement/KisSurfaceColorimetryIccUtils.h>

void KisCanvasSurfaceColorSpaceManager::reinitializeSurfaceDescription()
{
    using KisSurfaceColorimetry::RenderIntent;
    using KisSurfaceColorimetry::SurfaceDescription;
    using KisSurfaceColorimetry::NamedPrimaries;
    using KisSurfaceColorimetry::NamedTransferFunction;

    RenderIntent preferredIntent =
        m_proofingIntentOverride ?
        *m_proofingIntentOverride :
        calculateConfigIntent(m_currentConfig);

    if (!m_interface->supportsRenderIntent(preferredIntent)) {
        qWarning() << "WARNING: failed to set user preferred rendering"
                   << "intent for the surface, intent \""
                   << preferredIntent << "\" is unsupported, falling back to \"perceptual\"";

        preferredIntent = RenderIntent::render_intent_perceptual;

        // perceptual intent is guaranteed to be supported by the compositor
        KIS_SAFE_ASSERT_RECOVER_RETURN(m_interface->supportsRenderIntent(preferredIntent));
    }

    std::optional<SurfaceDescription> requestedDescription;
    const KoColorProfile *profile = nullptr;

    if (m_surfaceMode == KisConfig::CanvasSurfaceMode::Unmanaged) {
        profile = KoColorSpaceRegistry::instance()->p709SRGBProfile();
    } else {
        requestedDescription = SurfaceDescription();

        using namespace KisSurfaceColorimetry;

        auto compositorPreferred = m_interface->preferredSurfaceDescription();
        KIS_SAFE_ASSERT_RECOVER_RETURN(compositorPreferred);

        if (m_surfaceMode == KisConfig::CanvasSurfaceMode::Preferred) {
            // we don't copy mastering properties, since they mean a different thing
            // for the preferred space and outputs
            requestedDescription->colorSpace = compositorPreferred->colorSpace;
        } else if (m_surfaceMode == KisConfig::CanvasSurfaceMode::Rec709g22) {
            requestedDescription->colorSpace.primaries = NamedPrimaries::primaries_srgb;
            requestedDescription->colorSpace.transferFunction = NamedTransferFunction::transfer_function_gamma22;
        } else if (m_surfaceMode == KisConfig::CanvasSurfaceMode::Rec709g10) {
            requestedDescription->colorSpace.primaries = NamedPrimaries::primaries_srgb;
            requestedDescription->colorSpace.transferFunction = NamedTransferFunction::transfer_function_ext_linear;
        }

        // TODO: should we also set the HDR lightness properties?

        if (std::holds_alternative<NamedPrimaries>(requestedDescription->colorSpace.primaries) &&
            std::get<NamedPrimaries>(requestedDescription->colorSpace.primaries) == NamedPrimaries::primaries_unknown) {

            requestedDescription->colorSpace.primaries = NamedPrimaries::primaries_srgb;
        }

        if (std::holds_alternative<NamedTransferFunction>(requestedDescription->colorSpace.transferFunction) &&
            std::get<NamedTransferFunction>(requestedDescription->colorSpace.transferFunction) == NamedTransferFunction::transfer_function_unknown) {

            requestedDescription->colorSpace.transferFunction = NamedTransferFunction::transfer_function_gamma22;
        }

        if (!m_interface->supportsSurfaceDescription(*requestedDescription)) {

            // try pure sRGB
            requestedDescription->colorSpace.primaries = NamedPrimaries::primaries_srgb;
            requestedDescription->colorSpace.transferFunction = NamedTransferFunction::transfer_function_srgb;

            if (!m_interface->supportsSurfaceDescription(*requestedDescription)) {
                requestedDescription->colorSpace.transferFunction = NamedTransferFunction::transfer_function_gamma22;

                if (!m_interface->supportsSurfaceDescription(*requestedDescription)) {
                    qWarning() << "WARNING: failed to find a suitable surface format for the compositor";
                    return; // TODO: extra signals?
                }
            }
        }

        {
            auto request = colorSpaceToRequest(requestedDescription->colorSpace);
            if (request.isValid()) {
                profile = KoColorSpaceRegistry::instance()->profileFor(request.colorants,
                                                                       request.colorPrimariesType,
                                                                       request.transferFunction);
            }
        }

        if (!profile) {
            // keep primaries, but change the transfer function to gamma22
            requestedDescription->colorSpace.transferFunction = NamedTransferFunction::transfer_function_gamma22;
            if (m_interface->supportsSurfaceDescription(*requestedDescription)) {
                auto request = colorSpaceToRequest(requestedDescription->colorSpace);
                if (request.isValid()) {
                    profile = KoColorSpaceRegistry::instance()->profileFor(request.colorants,
                                                                           request.colorPrimariesType,
                                                                           request.transferFunction);
                }
            }
        }

        if (!profile) {
            // keep primaries, but change the transfer function to srgb
            requestedDescription->colorSpace.transferFunction = NamedTransferFunction::transfer_function_srgb;
            if (m_interface->supportsSurfaceDescription(*requestedDescription)) {
                auto request = colorSpaceToRequest(requestedDescription->colorSpace);
                if (request.isValid()) {
                    profile = KoColorSpaceRegistry::instance()->profileFor(request.colorants,
                                                                           request.colorPrimariesType,
                                                                           request.transferFunction);
                }
            }
        }

        if (!profile) {
            qWarning() << "WARNING: failed to to create a profile for the compositor's preferred color space";
            qWarning() << "    " << ppVar(compositorPreferred);
            qWarning() << "    " << ppVar(*requestedDescription);
            // TODO: debug all supported values for the compositor
            return; // TODO: extra signals?
        }
    }

    KIS_SAFE_ASSERT_RECOVER_RETURN(profile);
    KIS_SAFE_ASSERT_RECOVER_RETURN(!requestedDescription || m_interface->supportsSurfaceDescription(*requestedDescription));

    if (true || m_interface->surfaceDescription() != requestedDescription ||
        m_interface->renderingIntent() != preferredIntent) {

        if (requestedDescription) {
            auto future = m_interface->setSurfaceDescription(*requestedDescription, preferredIntent);
            future.then([](QFuture<bool> result) {
                if (!result.isValid() || !result.result()) {
                    qWarning()
                        << "WARNING: failed to set color space for the surface, setSurfaceDescription() returned false";
                }
            });
        } else {
            m_interface->unsetSurfaceDescription();
        }
    }

    KIS_SAFE_ASSERT_RECOVER_RETURN(m_currentConfig.profile);

    if (*m_currentConfig.profile != *profile) {
        m_currentConfig.profile = profile;
        Q_EMIT sigDisplayConfigChanged(m_currentConfig);
    }

}

KisDisplayConfig KisCanvasSurfaceColorSpaceManager::displayConfig() const
{
    return m_currentConfig;
}
