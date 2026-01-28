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

namespace {
struct SurfaceFormatSelectionResult {
    const KoColorProfile *profile = nullptr;
    std::optional<KisSurfaceColorimetry::SurfaceDescription> requestedDescription;
    QString errorMessage; // empty when no error
};
} /* namespace */

struct KRITAUI_NO_EXPORT KisCanvasSurfaceColorSpaceManager::Private
{
    Private(KisSurfaceColorManagerInterface *interface) : interface(interface) {}

    QScopedPointer<KisSurfaceColorManagerInterface> interface;
    KisDisplayConfig currentConfig;
    std::optional<KisSurfaceColorimetry::RenderIntent> proofingIntentOverride;
    KisConfig::CanvasSurfaceMode surfaceMode = KisConfig::CanvasSurfaceMode::Preferred;
    QString lastErrorString;

    static KisSurfaceColorimetry::RenderIntent calculateConfigIntent(const KisDisplayConfig::Options &options);

    SurfaceFormatSelectionResult
    selectSurfaceDescription(KisConfig::CanvasSurfaceMode surfaceMode, const KisSurfaceColorimetry::SurfaceDescription &compositorPreferred);
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

    if (!m_d->lastErrorString.isEmpty()) {
        str.noquote().nospace()
            << "ERROR: Failed to set up color management for the surface: "
            << m_d->lastErrorString
            << Qt::endl;
        str << Qt::endl;
    }

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

SurfaceFormatSelectionResult
KisCanvasSurfaceColorSpaceManager::Private::
selectSurfaceDescription(KisConfig::CanvasSurfaceMode requestedSurfaceMode, const KisSurfaceColorimetry::SurfaceDescription &compositorPreferred)
{
    using KisSurfaceColorimetry::NamedPrimaries;
    using KisSurfaceColorimetry::NamedTransferFunction;
    using KisSurfaceColorimetry::RenderIntent;
    using KisSurfaceColorimetry::SurfaceDescription;
    using KisSurfaceColorimetry::Luminance;

    std::optional<SurfaceDescription> requestedDescription = SurfaceDescription();
    const KoColorProfile *profile = nullptr;

    // we have our own definition of the Rec2020PQ space with
    // the reference point fixed to 80 cd/m2
    auto makeKritaRec2020PQLuminance = []() {
        Luminance luminance;
        luminance.minLuminance = 0;
        luminance.referenceLuminance = 80;
        luminance.maxLuminance = 10000;
        return luminance;
    };

    if (requestedSurfaceMode == KisConfig::CanvasSurfaceMode::Preferred) {
        if (compositorPreferred.colorSpace.isHDR()) {
            // we support HDR only via Rec2020PQ, so reset the color space to that
            // (KWin 6.5+ reports gamma-2.2 as preferred color space for whatever
            //  reason)
            requestedDescription->colorSpace.primaries = NamedPrimaries::primaries_bt2020;
            requestedDescription->colorSpace.transferFunction = NamedTransferFunction::transfer_function_st2084_pq;
        } else {
            // we don't copy mastering properties, since they mean a different thing
            // for the preferred space and outputs
            requestedDescription->colorSpace = compositorPreferred.colorSpace;
        }

        if (std::holds_alternative<NamedTransferFunction>(requestedDescription->colorSpace.transferFunction)
            && std::get<NamedTransferFunction>(requestedDescription->colorSpace.transferFunction)
                == NamedTransferFunction::transfer_function_st2084_pq) {
            requestedDescription->colorSpace.luminance = makeKritaRec2020PQLuminance();
        }
    } else if (requestedSurfaceMode == KisConfig::CanvasSurfaceMode::Rec2020pq) {
        requestedDescription->colorSpace.primaries = NamedPrimaries::primaries_bt2020;
        requestedDescription->colorSpace.transferFunction = NamedTransferFunction::transfer_function_st2084_pq;
        requestedDescription->colorSpace.luminance = makeKritaRec2020PQLuminance();
    } else if (requestedSurfaceMode == KisConfig::CanvasSurfaceMode::Rec709g22) {
        requestedDescription->colorSpace.primaries = NamedPrimaries::primaries_srgb;
        requestedDescription->colorSpace.transferFunction = NamedTransferFunction::transfer_function_gamma22;
        if (compositorPreferred.colorSpace.luminance) {
            // rec709-g22 is considered as SDR in lcms, so we should our space to SDR range
            requestedDescription->colorSpace.luminance = compositorPreferred.colorSpace.luminance->clipToSdr();
        }
    } else if (requestedSurfaceMode == KisConfig::CanvasSurfaceMode::Rec709g10) {
        requestedDescription->colorSpace.primaries = NamedPrimaries::primaries_srgb;
        requestedDescription->colorSpace.transferFunction = NamedTransferFunction::transfer_function_ext_linear;
        if (compositorPreferred.colorSpace.luminance) {
            /**
             * Our definition of rec709-g10 is different from the one used in Wayland.
             * Krita defines it as "value 1.0 is reference white" and everything above is
             * HDR values. But Wayland declares it as "0.0...1.0 is the full HDR range" and
             * reference white value being put somewhere inbetween. It technically allows
             * Wayland to use 10-bit integer surfaces for rec709-g10 space, which is a bad
             * idea in general (due to potential resolution limit). But given that Wayland/Mesa
             * doesn't seem to support 16-bit float surfaces, that is the only option they have.
             *
             * Anyway, due to limitations of 10-bit integer surfaces in rec709-g10 mode,
             * we just refuse to support that.
             */
            requestedDescription->colorSpace.luminance = compositorPreferred.colorSpace.luminance->clipToSdr();
        }
    }

    if (std::holds_alternative<NamedPrimaries>(requestedDescription->colorSpace.primaries)
        && std::get<NamedPrimaries>(requestedDescription->colorSpace.primaries) == NamedPrimaries::primaries_unknown) {
        requestedDescription->colorSpace.primaries = NamedPrimaries::primaries_srgb;
    }

    if (std::holds_alternative<NamedTransferFunction>(requestedDescription->colorSpace.transferFunction)
        && std::get<NamedTransferFunction>(requestedDescription->colorSpace.transferFunction)
            == NamedTransferFunction::transfer_function_unknown) {
        requestedDescription->colorSpace.transferFunction = NamedTransferFunction::transfer_function_gamma22;
    }

    if (!this->interface->supportsSurfaceDescription(*requestedDescription)) {
        // try pure sRGB
        requestedDescription->colorSpace.primaries = NamedPrimaries::primaries_srgb;
        requestedDescription->colorSpace.transferFunction = NamedTransferFunction::transfer_function_srgb;

        if (!this->interface->supportsSurfaceDescription(*requestedDescription)) {
            requestedDescription->colorSpace.transferFunction = NamedTransferFunction::transfer_function_gamma22;

            if (!this->interface->supportsSurfaceDescription(*requestedDescription)) {
                QString errorMessage = "failed to find a suitable surface format for the compositor";
                qWarning().nospace().noquote() << "ERROR: " << errorMessage;
                return {KoColorSpaceRegistry::instance()->p709SRGBProfile(), std::nullopt, errorMessage};
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
        if (this->interface->supportsSurfaceDescription(*requestedDescription)) {
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
        if (this->interface->supportsSurfaceDescription(*requestedDescription)) {
            auto request = colorSpaceToRequest(requestedDescription->colorSpace);
            if (request.isValid()) {
                profile = KoColorSpaceRegistry::instance()->profileFor(request.colorants,
                                                                       request.colorPrimariesType,
                                                                       request.transferFunction);
            }
        }
    }

    if (!profile) {
        QString errorMessage = "failed to create a profile for the compositor's preferred color space";
        qWarning().nospace().noquote() << "ERROR: " << errorMessage;
        qWarning() << "    " << ppVar(compositorPreferred);
        qWarning() << "    " << ppVar(*requestedDescription);
        return {KoColorSpaceRegistry::instance()->p709SRGBProfile(), std::nullopt, errorMessage};
    }

    return {profile, requestedDescription, {}};
}

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
        m_d->lastErrorString.clear();
    } else {
        const auto compositorPreferred = m_d->interface->preferredSurfaceDescription();
        KIS_SAFE_ASSERT_RECOVER_RETURN(compositorPreferred);

        auto result = m_d->selectSurfaceDescription(m_d->surfaceMode, *compositorPreferred);
        profile = result.profile;
        requestedDescription = result.requestedDescription;
        m_d->lastErrorString = result.errorMessage;
    }

    KIS_SAFE_ASSERT_RECOVER_RETURN(profile);
    KIS_SAFE_ASSERT_RECOVER_RETURN(!requestedDescription || m_d->interface->supportsSurfaceDescription(*requestedDescription));

    if (m_d->interface->surfaceDescription() != requestedDescription ||
        m_d->interface->renderingIntent() != preferredIntent) {

        if (requestedDescription) {
            auto future = m_d->interface->setSurfaceDescription(*requestedDescription, preferredIntent);
            future.then([this](QFuture<bool> result) {
                if (!result.isValid() || !result.result()) {
                    QString errorMessage = "failed to set color space for the surface, setSurfaceDescription() returned false";
                    m_d->lastErrorString = errorMessage;
                    qWarning().nospace().noquote() << "ERROR: " << errorMessage;
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

    KIS_SAFE_ASSERT_RECOVER(m_d->currentConfig.profile) {
        m_d->currentConfig.profile = KoColorSpaceRegistry::instance()->p709SRGBProfile();
    }

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
