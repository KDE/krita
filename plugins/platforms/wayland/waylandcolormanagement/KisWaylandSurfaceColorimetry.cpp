/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisWaylandSurfaceColorimetry.h"

#include <QDebug>

namespace KisSurfaceColorimetry {

SurfaceDescription WaylandSurfaceDescription::toSurfaceDescription() const
{
    KisSurfaceColorimetry::SurfaceDescription description;

    // Map transfer function
    if (this->tfNamed) {
        switch (*this->tfNamed) {
        case QtWayland::wp_color_manager_v1::transfer_function_bt1886:
            description.colorSpace.transferFunction = KisSurfaceColorimetry::NamedTransferFunction::transfer_function_bt1886;
            break;
        case QtWayland::wp_color_manager_v1::transfer_function_gamma22:
            description.colorSpace.transferFunction = KisSurfaceColorimetry::NamedTransferFunction::transfer_function_gamma22;
            break;
        case QtWayland::wp_color_manager_v1::transfer_function_gamma28:
            description.colorSpace.transferFunction = KisSurfaceColorimetry::NamedTransferFunction::transfer_function_gamma28;
            break;
        case QtWayland::wp_color_manager_v1::transfer_function_st240:
            description.colorSpace.transferFunction = KisSurfaceColorimetry::NamedTransferFunction::transfer_function_unknown;
            break;
        case QtWayland::wp_color_manager_v1::transfer_function_ext_linear:
            description.colorSpace.transferFunction = KisSurfaceColorimetry::NamedTransferFunction::transfer_function_ext_linear;
            break;
        case QtWayland::wp_color_manager_v1::transfer_function_log_100:
            description.colorSpace.transferFunction = KisSurfaceColorimetry::NamedTransferFunction::transfer_function_unknown;
            break;
        case QtWayland::wp_color_manager_v1::transfer_function_log_316:
            description.colorSpace.transferFunction = KisSurfaceColorimetry::NamedTransferFunction::transfer_function_unknown;
            break;
        case QtWayland::wp_color_manager_v1::transfer_function_xvycc:
            description.colorSpace.transferFunction = KisSurfaceColorimetry::NamedTransferFunction::transfer_function_unknown;
            break;
        case QtWayland::wp_color_manager_v1::transfer_function_srgb:
            description.colorSpace.transferFunction = KisSurfaceColorimetry::NamedTransferFunction::transfer_function_srgb;
            break;
        case QtWayland::wp_color_manager_v1::transfer_function_ext_srgb:
            description.colorSpace.transferFunction = KisSurfaceColorimetry::NamedTransferFunction::transfer_function_ext_srgb;
            break;
        case QtWayland::wp_color_manager_v1::transfer_function_st2084_pq:
            description.colorSpace.transferFunction = KisSurfaceColorimetry::NamedTransferFunction::transfer_function_st2084_pq;
            break;
        case QtWayland::wp_color_manager_v1::transfer_function_st428:
            description.colorSpace.transferFunction = KisSurfaceColorimetry::NamedTransferFunction::transfer_function_unknown;
            break;
        case QtWayland::wp_color_manager_v1::transfer_function_hlg:
            description.colorSpace.transferFunction = KisSurfaceColorimetry::NamedTransferFunction::transfer_function_unknown;
            break;
        }
    } else if (this->tfGamma) {
        description.colorSpace.transferFunction = *this->tfGamma;
    }

    // Map primaries
    if (this->namedContainer) {
        switch (*this->namedContainer) {
        case QtWayland::wp_color_manager_v1::primaries_srgb:
            description.colorSpace.primaries = KisSurfaceColorimetry::NamedPrimaries::primaries_srgb;
            break;
        case QtWayland::wp_color_manager_v1::primaries_pal_m:
            description.colorSpace.primaries = KisSurfaceColorimetry::NamedPrimaries::primaries_unknown;
            break;
        case QtWayland::wp_color_manager_v1::primaries_pal:
            description.colorSpace.primaries = KisSurfaceColorimetry::NamedPrimaries::primaries_unknown;
            break;
        case QtWayland::wp_color_manager_v1::primaries_ntsc:
            description.colorSpace.primaries = KisSurfaceColorimetry::NamedPrimaries::primaries_unknown;
            break;
        case QtWayland::wp_color_manager_v1::primaries_generic_film:
            description.colorSpace.primaries = KisSurfaceColorimetry::NamedPrimaries::primaries_unknown;
            break;
        case QtWayland::wp_color_manager_v1::primaries_bt2020:
            description.colorSpace.primaries = KisSurfaceColorimetry::NamedPrimaries::primaries_bt2020;
            break;
        case QtWayland::wp_color_manager_v1::primaries_cie1931_xyz:
            description.colorSpace.primaries = KisSurfaceColorimetry::NamedPrimaries::primaries_unknown;
            break;
        case QtWayland::wp_color_manager_v1::primaries_dci_p3:
            description.colorSpace.primaries = KisSurfaceColorimetry::NamedPrimaries::primaries_dci_p3;
            break;
        case QtWayland::wp_color_manager_v1::primaries_display_p3:
            description.colorSpace.primaries = KisSurfaceColorimetry::NamedPrimaries::primaries_display_p3;
            break;
        case QtWayland::wp_color_manager_v1::primaries_adobe_rgb:
            description.colorSpace.primaries = KisSurfaceColorimetry::NamedPrimaries::primaries_adobe_rgb;
            break;
        }
    } else if (this->container) {
        description.colorSpace.primaries = this->container->toColorimetry();
    }

    // Map luminance
    if (this->luminances) {
        description.colorSpace.luminance = this->luminances;
    }

    // Map mastering info
    if (this->target || this->masteringLuminance) {
        KisSurfaceColorimetry::MasteringInfo masteringInfo;

        if (this->target) {
            masteringInfo.primaries = this->target->toColorimetry();
        } else if (std::holds_alternative<KisSurfaceColorimetry::Colorimetry>(description.colorSpace.primaries)) {
            masteringInfo.primaries = std::get<KisSurfaceColorimetry::Colorimetry>(description.colorSpace.primaries);
        } else if (this->namedContainer) {
            switch (*this->namedContainer) {
            case QtWayland::wp_color_manager_v1::primaries_srgb:
                masteringInfo.primaries = KisColorimetryUtils::Colorimetry::BT709;
                break;
            case QtWayland::wp_color_manager_v1::primaries_pal_m:
                masteringInfo.primaries = KisColorimetryUtils::Colorimetry::PAL_M;
                break;
            case QtWayland::wp_color_manager_v1::primaries_pal:
                masteringInfo.primaries = KisColorimetryUtils::Colorimetry::PAL;
                break;
            case QtWayland::wp_color_manager_v1::primaries_ntsc:
                masteringInfo.primaries = KisColorimetryUtils::Colorimetry::NTSC;
                break;
            case QtWayland::wp_color_manager_v1::primaries_generic_film:
                masteringInfo.primaries = KisColorimetryUtils::Colorimetry::GenericFilm;
                break;
            case QtWayland::wp_color_manager_v1::primaries_bt2020:
                masteringInfo.primaries = KisColorimetryUtils::Colorimetry::BT2020;
                break;
            case QtWayland::wp_color_manager_v1::primaries_cie1931_xyz:
                masteringInfo.primaries = KisColorimetryUtils::Colorimetry::CIEXYZ;
                break;
            case QtWayland::wp_color_manager_v1::primaries_dci_p3:
                masteringInfo.primaries = KisColorimetryUtils::Colorimetry::DCIP3;
                break;
            case QtWayland::wp_color_manager_v1::primaries_display_p3:
                masteringInfo.primaries = KisColorimetryUtils::Colorimetry::DisplayP3;
                break;
            case QtWayland::wp_color_manager_v1::primaries_adobe_rgb:
                masteringInfo.primaries = KisColorimetryUtils::Colorimetry::AdobeRGB;
                break;
            }
        }

        if (this->masteringLuminance) {
            masteringInfo.luminance = *this->masteringLuminance;
        } else {
            if (description.colorSpace.luminance) {
                masteringInfo.luminance.fromLuminance(*description.colorSpace.luminance);
            } else {
                masteringInfo.luminance = KisSurfaceColorimetry::MasteringLuminance();
            }
        }

        if (this->targetMaxCLL) {
            masteringInfo.maxCll = *this->targetMaxCLL;
        }
        if (this->targetMaxFALL) {
            masteringInfo.maxFall = *this->targetMaxFALL;
        }

        description.masteringInfo = masteringInfo;
    }

    return description;
}

WaylandSurfaceDescription WaylandSurfaceDescription::fromSurfaceDescription(const SurfaceDescription &rhs)
{
    WaylandSurfaceDescription desc;

    using namespace KisColorimetryUtils;
    using namespace KisSurfaceColorimetry;

    if (std::holds_alternative<NamedPrimaries>(rhs.colorSpace.primaries)) {
        desc.namedContainer = primariesKritaToWayland(std::get<NamedPrimaries>(rhs.colorSpace.primaries));
    } else {
        desc.container = WaylandPrimaries::fromColorimetry(
            std::get<Colorimetry>(rhs.colorSpace.primaries));
    }

    if (std::holds_alternative<NamedTransferFunction>(rhs.colorSpace.transferFunction)) {
        desc.tfNamed = transferFunctionKritaToWayland(std::get<NamedTransferFunction>(rhs.colorSpace.transferFunction));
    } else {
        desc.tfGamma = std::get<uint32_t>(rhs.colorSpace.transferFunction);
    }

    if (rhs.colorSpace.luminance) {
        desc.luminances = rhs.colorSpace.luminance;
    }

    if (rhs.masteringInfo) {
        desc.target = WaylandPrimaries::fromColorimetry(rhs.masteringInfo->primaries);
        desc.masteringLuminance = rhs.masteringInfo->luminance;

        if (rhs.masteringInfo->maxCll) {
            desc.targetMaxCLL = *rhs.masteringInfo->maxCll;
        }

        if (rhs.masteringInfo->maxFall) {
            desc.targetMaxCLL = *rhs.masteringInfo->maxFall;
        }
    }

    return desc;
}

QDebug operator<<(QDebug dbg, const WaylandPrimaries &points) {
    dbg.nospace() << Qt::fixed << qSetRealNumberPrecision(5) << "KisColorPrimaries("
                  << "red: " << points.red << ", "
                  << "green: " << points.green << ", "
                  << "blue: " << points.blue << ", "
                  << "white: " << points.white
                  << ")";
    dbg.resetFormat();
    return dbg.space();
}

QDebug operator<<(QDebug dbg, const WaylandSurfaceDescription &data) {

    dbg.nospace() << "WaylandSurfaceDescription(";

    auto printOptionalTag = [&dbg](const char *tag, const auto &value) {
        dbg.nospace() << tag << ": ";
        if (value) {
            dbg << *value << ", ";
        } else {
            dbg << "none" << ", ";
        }
    };

    printOptionalTag("tfGamma", data.tfGamma);
    printOptionalTag("tfNamed", data.tfNamed);
    printOptionalTag("container", data.container);
    printOptionalTag("namedContainer", data.namedContainer);
    printOptionalTag("target", data.target);
    printOptionalTag("luminances", data.luminances);
    printOptionalTag("masteringLuminance", data.masteringLuminance);
    printOptionalTag("targetMaxCLL", data.targetMaxCLL);
    printOptionalTag("targetMaxFALL", data.targetMaxFALL);

    dbg.nospace() << "iccFileIsPresent: " << data.iccFileIsPresent;

    dbg.nospace() << ")";

    return dbg.space();
}

QtWayland::wp_color_manager_v1::primaries primariesKritaToWayland(KisSurfaceColorimetry::NamedPrimaries primaries)
{
    using namespace KisSurfaceColorimetry;
    using primaries_type = QtWayland::wp_color_manager_v1::primaries;

    switch (primaries) {
    case NamedPrimaries::primaries_srgb:
        return primaries_type::primaries_srgb;
    case NamedPrimaries::primaries_bt2020:
        return primaries_type::primaries_bt2020;
    case NamedPrimaries::primaries_dci_p3:
        return primaries_type::primaries_dci_p3;
    case NamedPrimaries::primaries_display_p3:
        return primaries_type::primaries_display_p3;
    case NamedPrimaries::primaries_adobe_rgb:
        return primaries_type::primaries_adobe_rgb;
    case NamedPrimaries::primaries_unknown:
        Q_ASSERT(0 && "trying to set unknown named primary to the surface");
    }
    return primaries_type::primaries_srgb;
}

QtWayland::wp_color_manager_v1::transfer_function transferFunctionKritaToWayland(KisSurfaceColorimetry::NamedTransferFunction transferFunction)
{
    using namespace KisSurfaceColorimetry;
    using transfer_function_type = QtWayland::wp_color_manager_v1::transfer_function;

    switch (transferFunction) {
    case NamedTransferFunction::transfer_function_bt1886:
        return transfer_function_type::transfer_function_bt1886;
    case NamedTransferFunction::transfer_function_gamma22:
        return transfer_function_type::transfer_function_gamma22;
    case NamedTransferFunction::transfer_function_gamma28:
        return transfer_function_type::transfer_function_gamma28;
    case NamedTransferFunction::transfer_function_ext_linear:
        return transfer_function_type::transfer_function_ext_linear;
    case NamedTransferFunction::transfer_function_srgb:
        return transfer_function_type::transfer_function_srgb;
    case NamedTransferFunction::transfer_function_ext_srgb:
        return transfer_function_type::transfer_function_ext_srgb;
    case NamedTransferFunction::transfer_function_st2084_pq:
        return transfer_function_type::transfer_function_st2084_pq;
    case NamedTransferFunction::transfer_function_st428:
        return transfer_function_type::transfer_function_st428;
    case NamedTransferFunction::transfer_function_unknown:
        Q_ASSERT(0 && "trying to set unknown named transfer function to the surface");
    }
    return transfer_function_type::transfer_function_srgb;
}

QtWayland::wp_color_manager_v1::render_intent renderIntentKritaToWayland(KisSurfaceColorimetry::RenderIntent intent)
{
    using namespace KisSurfaceColorimetry;

    switch (intent) {
    case RenderIntent::render_intent_perceptual:
        return QtWayland::wp_color_manager_v1::render_intent::render_intent_perceptual;
    case RenderIntent::render_intent_relative:
        return QtWayland::wp_color_manager_v1::render_intent::render_intent_relative;
    case RenderIntent::render_intent_saturation:
        return QtWayland::wp_color_manager_v1::render_intent::render_intent_saturation;
    case RenderIntent::render_intent_absolute:
        return QtWayland::wp_color_manager_v1::render_intent::render_intent_absolute;
    case RenderIntent::render_intent_relative_bpc:
        return QtWayland::wp_color_manager_v1::render_intent::render_intent_relative_bpc;
    default:
        Q_ASSERT(false && "Unknown RenderIntent value");
        return QtWayland::wp_color_manager_v1::render_intent::render_intent_perceptual; // Default fallback
    }
}

} // namespace KisSurfaceColorimetry

/**
 * The operators for the members of QtWayland::wp_color_manager_v1 are
 * placed **outside** the KisSurfaceColorimetry namespace
 */

QDebug operator<<(QDebug debug, QtWayland::wp_color_manager_v1::primaries p)
{
    QDebugStateSaver saver(debug);
    switch (p) {
    case QtWayland::wp_color_manager_v1::primaries_srgb:
        debug << "srgb";
        break;
    case QtWayland::wp_color_manager_v1::primaries_pal_m:
        debug << "pal_m";
        break;
    case QtWayland::wp_color_manager_v1::primaries_pal:
        debug << "pal";
        break;
    case QtWayland::wp_color_manager_v1::primaries_ntsc:
        debug << "ntsc";
        break;
    case QtWayland::wp_color_manager_v1::primaries_generic_film:
        debug << "generic_film";
        break;
    case QtWayland::wp_color_manager_v1::primaries_bt2020:
        debug << "bt2020";
        break;
    case QtWayland::wp_color_manager_v1::primaries_cie1931_xyz:
        debug << "cie1931_xyz";
        break;
    case QtWayland::wp_color_manager_v1::primaries_dci_p3:
        debug << "dci_p3";
        break;
    case QtWayland::wp_color_manager_v1::primaries_display_p3:
        debug << "display_p3";
        break;
    case QtWayland::wp_color_manager_v1::primaries_adobe_rgb:
        debug << "adobe_rgb";
        break;
    default:
        debug << "Unknown primaries:" << static_cast<int>(p);
        break;
    }
    return debug;
}

QDebug operator<<(QDebug debug, QtWayland::wp_color_manager_v1::feature f)
{
    QDebugStateSaver saver(debug);
    switch (f) {
    case QtWayland::wp_color_manager_v1::feature_icc_v2_v4:
        debug << "icc_v2_v4";
        break;
    case QtWayland::wp_color_manager_v1::feature_parametric:
        debug << "parametric";
        break;
    case QtWayland::wp_color_manager_v1::feature_set_primaries:
        debug << "set_primaries";
        break;
    case QtWayland::wp_color_manager_v1::feature_set_tf_power:
        debug << "set_tf_power";
        break;
    case QtWayland::wp_color_manager_v1::feature_set_luminances:
        debug << "set_luminances";
        break;
    case QtWayland::wp_color_manager_v1::feature_set_mastering_display_primaries:
        debug << "set_mastering_display_primaries";
        break;
    case QtWayland::wp_color_manager_v1::feature_extended_target_volume:
        debug << "extended_target_volume";
        break;
    case QtWayland::wp_color_manager_v1::feature_windows_scrgb:
        debug << "windows_scrgb";
        break;
    default:
        debug << "Unknown feature:" << static_cast<int>(f);
        break;
    }
    return debug;
}

QDebug operator<<(QDebug debug, QtWayland::wp_color_manager_v1::render_intent ri)
{
    QDebugStateSaver saver(debug);
    switch (ri) {
    case QtWayland::wp_color_manager_v1::render_intent_perceptual:
        debug << "perceptual";
        break;
    case QtWayland::wp_color_manager_v1::render_intent_relative:
        debug << "relative";
        break;
    case QtWayland::wp_color_manager_v1::render_intent_saturation:
        debug << "saturation";
        break;
    case QtWayland::wp_color_manager_v1::render_intent_absolute:
        debug << "absolute";
        break;
    case QtWayland::wp_color_manager_v1::render_intent_relative_bpc:
        debug << "relative_bpc";
        break;
    default:
        debug << "Unknown render intent:" << static_cast<int>(ri);
        break;
    }
    return debug;
}

QDebug operator<<(QDebug debug, QtWayland::wp_color_manager_v1::transfer_function tf)
{
    QDebugStateSaver saver(debug);
    switch (tf) {
    case QtWayland::wp_color_manager_v1::transfer_function_bt1886:
        debug << "bt1886";
        break;
    case QtWayland::wp_color_manager_v1::transfer_function_gamma22:
        debug << "gamma22";
        break;
    case QtWayland::wp_color_manager_v1::transfer_function_gamma28:
        debug << "gamma28";
        break;
    case QtWayland::wp_color_manager_v1::transfer_function_st240:
        debug << "st240";
        break;
    case QtWayland::wp_color_manager_v1::transfer_function_ext_linear:
        debug << "ext_linear";
        break;
    case QtWayland::wp_color_manager_v1::transfer_function_log_100:
        debug << "log_100";
        break;
    case QtWayland::wp_color_manager_v1::transfer_function_log_316:
        debug << "log_316";
        break;
    case QtWayland::wp_color_manager_v1::transfer_function_xvycc:
        debug << "xvycc";
        break;
    case QtWayland::wp_color_manager_v1::transfer_function_srgb:
        debug << "srgb";
        break;
    case QtWayland::wp_color_manager_v1::transfer_function_ext_srgb:
        debug << "ext_srgb";
        break;
    case QtWayland::wp_color_manager_v1::transfer_function_st2084_pq:
        debug << "st2084_pq";
        break;
    case QtWayland::wp_color_manager_v1::transfer_function_st428:
        debug << "st428";
        break;
    case QtWayland::wp_color_manager_v1::transfer_function_hlg:
        debug << "hlg";
        break;
    default:
        debug << "Unknown transfer function:" << static_cast<int>(tf);
        break;
    }
    return debug;
}

/* KISSURFACECOLORIMETRY_H */