/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisCanvasSurfaceColorSpaceManager.h"

#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>

#include <kis_config.h>

#include <KisDisplayConfig.h>
#include <KisProofingConfiguration.h>

#include <surfacecolormanagement/KisSurfaceColorimetry.h>
#include <surfacecolormanagement/KisSurfaceColorManagerInterface.h>

struct KRITAUI_NO_EXPORT KisCanvasSurfaceColorSpaceManager::Private
{
    Private(KisSurfaceColorManagerInterface *interface) : interface(interface) {}

    QScopedPointer<KisSurfaceColorManagerInterface> interface;
    KisDisplayConfig currentConfig;
    std::optional<KisSurfaceColorimetry::RenderIntent> proofingIntentOverride;
    KisConfig::CanvasSurfaceMode surfaceMode = KisConfig::CanvasSurfaceMode::Preferred;

    static KisSurfaceColorimetry::RenderIntent calculateConfigIntent(int intent, bool useBlackPointCompensation);
    static KisSurfaceColorimetry::RenderIntent calculateConfigIntent(const KisDisplayConfig::Options &options);
};

KisCanvasSurfaceColorSpaceManager::KisCanvasSurfaceColorSpaceManager(KisSurfaceColorManagerInterface *interface,
                                                                     const KisConfig::CanvasSurfaceMode surfaceMode,
                                                                     const KisDisplayConfig::Options &options,
                                                                     QObject *parent)
    : QObject(parent)
    , m_d(new Private(interface))
{
    m_d->currentConfig.profile = KoColorSpaceRegistry::instance()->p709SRGBProfile();
    m_d->currentConfig.setOptions(options);
    m_d->currentConfig.isHDR = false;
    m_d->surfaceMode = surfaceMode;

    connect(m_d->interface.data(), &KisSurfaceColorManagerInterface::sigReadyChanged, this, &KisCanvasSurfaceColorSpaceManager::slotInterfaceReadyChanged);
    connect(m_d->interface.data(), &KisSurfaceColorManagerInterface::sigPreferredSurfaceDescriptionChanged, this, &KisCanvasSurfaceColorSpaceManager::slotInterfacePreferredDescriptionChanged);

    if (m_d->interface->isReady()) {
        slotInterfaceReadyChanged(true);
    }
}

KisCanvasSurfaceColorSpaceManager::~KisCanvasSurfaceColorSpaceManager()
{
}

QString KisCanvasSurfaceColorSpaceManager::colorManagementReport() const
{
    QString report;
    QDebug str(&report);

    if (!m_d->interface->isReady()) {
        str << "WARNING: surface color management interface is not ready!" << Qt::endl;
        str << Qt::endl;
    }

    using KisSurfaceColorimetry::RenderIntent;
    using KisSurfaceColorimetry::SurfaceDescription;
    using KisSurfaceColorimetry::NamedPrimaries;
    using KisSurfaceColorimetry::NamedTransferFunction;

    str << "Configured mode:" << m_d->surfaceMode << Qt::endl;

    RenderIntent preferredIntent = Private::calculateConfigIntent(m_d->currentConfig.options());
    str << "Configured intent:" << preferredIntent << "supported:" << m_d->interface->supportsRenderIntent(preferredIntent) << Qt::endl;

    str << "Actual intent:";
    if (m_d->interface->renderingIntent()) {
        str << *m_d->interface->renderingIntent() << Qt::endl;
    } else {
        str << "<none>" << Qt::endl;
    }
    str << Qt::endl;

    str << "Active surface description:";
    if (m_d->interface->surfaceDescription()) {
        str << Qt::endl;
        str.noquote() << m_d->interface->surfaceDescription()->makeTextReport() << Qt::endl;
    } else {
        str << "<none>" << Qt::endl;
    }
    str << Qt::endl;

    str << "Selected Profile:";
    if (m_d->currentConfig.profile) {
        auto profile = m_d->currentConfig.profile;

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
    if (m_d->interface->preferredSurfaceDescription()) {
        str << Qt::endl;
        str.noquote() << m_d->interface->preferredSurfaceDescription()->makeTextReport() << Qt::endl;
    } else {
        str << "<none>" << Qt::endl;
    }
    str << Qt::endl;

    return report;
}

QString KisCanvasSurfaceColorSpaceManager::osPreferredColorSpaceReport() const
{
    QString report;
    QDebug str(&report);

    if (!m_d->interface->isReady()) {
        str << "WARNING: surface color management interface is not ready!" << Qt::endl;
        str << Qt::endl;
    }

    if (m_d->interface->preferredSurfaceDescription()) {
        str << Qt::endl;
        str.noquote() << m_d->interface->preferredSurfaceDescription()->makeTextReport() << Qt::endl;
    } else {
        str << "<none>" << Qt::endl;
    }

    return report;
}

KisSurfaceColorimetry::RenderIntent KisCanvasSurfaceColorSpaceManager::Private::calculateConfigIntent(const KisDisplayConfig::Options &options)
{
    using KisSurfaceColorimetry::RenderIntent;

    RenderIntent intent = RenderIntent::render_intent_perceptual;

    switch (options.first) {
        case INTENT_PERCEPTUAL:
            // default value
            break;
        case INTENT_RELATIVE_COLORIMETRIC:
            intent =
                options.second.testFlag(KoColorConversionTransformation::ConversionFlag::BlackpointCompensation) ?
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

void KisCanvasSurfaceColorSpaceManager::setDisplayConfigOptions(const KisConfig::CanvasSurfaceMode surfaceMode, const KisDisplayConfig::Options &options)
{
    if (!m_d->interface->isReady()) return;
    if (surfaceMode == m_d->surfaceMode && options == m_d->currentConfig.options()) return;

    m_d->surfaceMode = surfaceMode;
    reinitializeSurfaceDescription(options);
}

void KisCanvasSurfaceColorSpaceManager::setDisplayConfigOptions(const KisDisplayConfig::Options &options)
{
    setDisplayConfigOptions(m_d->surfaceMode, options);
}

void KisCanvasSurfaceColorSpaceManager::slotInterfaceReadyChanged(bool isReady)
{
    if (isReady) {
        reinitializeSurfaceDescription(m_d->currentConfig.options());
    }
}

void KisCanvasSurfaceColorSpaceManager::slotInterfacePreferredDescriptionChanged()
{
    reinitializeSurfaceDescription(m_d->currentConfig.options());
}

#include <KoColorProfile.h>
#include <surfacecolormanagement/KisSurfaceColorimetryIccUtils.h>

void KisCanvasSurfaceColorSpaceManager::reinitializeSurfaceDescription(const KisDisplayConfig::Options &newOptions)
{
    using KisSurfaceColorimetry::NamedPrimaries;
    using KisSurfaceColorimetry::NamedTransferFunction;
    using KisSurfaceColorimetry::RenderIntent;
    using KisSurfaceColorimetry::SurfaceDescription;

    RenderIntent preferredIntent =
        Private::calculateConfigIntent(newOptions);

    if (!m_d->interface->supportsRenderIntent(preferredIntent)) {
        qWarning() << "WARNING: failed to set user preferred rendering"
                   << "intent for the surface, intent \""
                   << preferredIntent << "\" is unsupported, falling back to \"perceptual\"";

        preferredIntent = RenderIntent::render_intent_perceptual;

        // perceptual intent is guaranteed to be supported by the compositor
        KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->interface->supportsRenderIntent(preferredIntent));
    }

    std::optional<SurfaceDescription> requestedDescription;
    const KoColorProfile *profile = nullptr;

    if (m_d->surfaceMode == KisConfig::CanvasSurfaceMode::Unmanaged) {
        profile = KoColorSpaceRegistry::instance()->p709SRGBProfile();
    } else {
        requestedDescription = SurfaceDescription();

        using namespace KisSurfaceColorimetry;

        const auto compositorPreferred = m_d->interface->preferredSurfaceDescription();
        KIS_SAFE_ASSERT_RECOVER_RETURN(compositorPreferred);

        if (m_d->surfaceMode == KisConfig::CanvasSurfaceMode::Preferred) {
            // we don't copy mastering properties, since they mean a different thing
            // for the preferred space and outputs
            requestedDescription->colorSpace = compositorPreferred->colorSpace;

            if (std::holds_alternative<NamedTransferFunction>(requestedDescription->colorSpace.transferFunction) &&
                std::get<NamedTransferFunction>(requestedDescription->colorSpace.transferFunction) == NamedTransferFunction::transfer_function_st2084_pq) {

                // we have our own definition of the Rec2020PQ space with
                // the reference point fixed to 80 cd/m2
                requestedDescription->colorSpace.luminance = Luminance();
                requestedDescription->colorSpace.luminance->minLuminance = 0;
                requestedDescription->colorSpace.luminance->referenceLuminance = 80;
                requestedDescription->colorSpace.luminance->maxLuminance = 10000;
            }

        } else if (m_d->surfaceMode == KisConfig::CanvasSurfaceMode::Rec709g22) {
            requestedDescription->colorSpace.primaries = NamedPrimaries::primaries_srgb;
            requestedDescription->colorSpace.transferFunction = NamedTransferFunction::transfer_function_gamma22;
            if (compositorPreferred->colorSpace.luminance) {
                // rec709-g22 is considered as SDR in lcms, so we should our space to SDR range
                requestedDescription->colorSpace.luminance = 
                    compositorPreferred->colorSpace.luminance->clipToSdr();
            }
        } else if (m_d->surfaceMode == KisConfig::CanvasSurfaceMode::Rec709g10) {
            requestedDescription->colorSpace.primaries = NamedPrimaries::primaries_srgb;
            requestedDescription->colorSpace.transferFunction = NamedTransferFunction::transfer_function_ext_linear;
            if (compositorPreferred->colorSpace.luminance) {
                // rec709-g22 is considered as SDR in lcms, so we should our space to SDR range
                requestedDescription->colorSpace.luminance = 
                    compositorPreferred->colorSpace.luminance->clipToSdr();
            }
        }

        if (std::holds_alternative<NamedPrimaries>(requestedDescription->colorSpace.primaries) &&
            std::get<NamedPrimaries>(requestedDescription->colorSpace.primaries) == NamedPrimaries::primaries_unknown) {

            requestedDescription->colorSpace.primaries = NamedPrimaries::primaries_srgb;
        }

        if (std::holds_alternative<NamedTransferFunction>(requestedDescription->colorSpace.transferFunction) &&
            std::get<NamedTransferFunction>(requestedDescription->colorSpace.transferFunction) == NamedTransferFunction::transfer_function_unknown) {

            requestedDescription->colorSpace.transferFunction = NamedTransferFunction::transfer_function_gamma22;
        }

        if (!m_d->interface->supportsSurfaceDescription(*requestedDescription)) {

            // try pure sRGB
            requestedDescription->colorSpace.primaries = NamedPrimaries::primaries_srgb;
            requestedDescription->colorSpace.transferFunction = NamedTransferFunction::transfer_function_srgb;

            if (!m_d->interface->supportsSurfaceDescription(*requestedDescription)) {
                requestedDescription->colorSpace.transferFunction = NamedTransferFunction::transfer_function_gamma22;

                if (!m_d->interface->supportsSurfaceDescription(*requestedDescription)) {
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
            if (m_d->interface->supportsSurfaceDescription(*requestedDescription)) {
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
            if (m_d->interface->supportsSurfaceDescription(*requestedDescription)) {
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
    KIS_SAFE_ASSERT_RECOVER_RETURN(!requestedDescription || m_d->interface->supportsSurfaceDescription(*requestedDescription));

    if (m_d->interface->surfaceDescription() != requestedDescription ||
        m_d->interface->renderingIntent() != preferredIntent) {

        if (requestedDescription) {
            auto future = m_d->interface->setSurfaceDescription(*requestedDescription, preferredIntent);
            future.then([](QFuture<bool> result) {
                if (!result.isValid() || !result.result()) {
                    qWarning()
                        << "WARNING: failed to set color space for the surface, setSurfaceDescription() returned false";
                }
            });
        } else {
            m_d->interface->unsetSurfaceDescription();
        }
    }

    const bool requestedDescriptionIsHDR = requestedDescription && requestedDescription->colorSpace.isHDR();

    KisDisplayConfig newDisplayConfig;
    newDisplayConfig.profile = profile;
    newDisplayConfig.setOptions(newOptions); // TODO: think about failure to set the intent and infinite update loop
    newDisplayConfig.isHDR = requestedDescriptionIsHDR;

    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->currentConfig.profile);

    if (m_d->currentConfig != newDisplayConfig) {
        m_d->currentConfig = newDisplayConfig;
        Q_EMIT sigDisplayConfigChanged(m_d->currentConfig);
    }
}

bool KisCanvasSurfaceColorSpaceManager::isReady() const
{
    return m_d->interface->isReady();
}

KisDisplayConfig KisCanvasSurfaceColorSpaceManager::displayConfig() const
{
    return m_d->currentConfig;
}
