/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisSurfaceColorimetry.h"

#include <QDebug>

namespace KisSurfaceColorimetry {

QDebug operator<<(QDebug debug, const NamedPrimaries &value) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "NamedPrimaries(";
    switch (value) {
    case NamedPrimaries::primaries_unknown:
        debug.nospace() << "primaries_unknown";
        break;
    case NamedPrimaries::primaries_srgb:
        debug.nospace() << "primaries_srgb";
        break;
    case NamedPrimaries::primaries_bt2020:
        debug.nospace() << "primaries_bt2020";
        break;
    case NamedPrimaries::primaries_dci_p3:
        debug.nospace() << "primaries_dci_p3";
        break;
    case NamedPrimaries::primaries_display_p3:
        debug.nospace() << "primaries_display_p3";
        break;
    case NamedPrimaries::primaries_adobe_rgb:
        debug.nospace() << "primaries_adobe_rgb";
        break;
    }
    debug.nospace() << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const NamedTransferFunction &value) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "NamedTransferFunction(";
    switch (value) {
    case NamedTransferFunction::transfer_function_unknown:
        debug.nospace() << "transfer_function_unknown";
        break;
    case NamedTransferFunction::transfer_function_bt1886:
        debug.nospace() << "transfer_function_bt1886";
        break;
    case NamedTransferFunction::transfer_function_gamma22:
        debug.nospace() << "transfer_function_gamma22";
        break;
    case NamedTransferFunction::transfer_function_gamma28:
        debug.nospace() << "transfer_function_gamma28";
        break;
    case NamedTransferFunction::transfer_function_ext_linear:
        debug.nospace() << "transfer_function_ext_linear";
        break;
    case NamedTransferFunction::transfer_function_srgb:
        debug.nospace() << "transfer_function_srgb";
        break;
    case NamedTransferFunction::transfer_function_ext_srgb:
        debug.nospace() << "transfer_function_ext_srgb";
        break;
    case NamedTransferFunction::transfer_function_st2084_pq:
        debug.nospace() << "transfer_function_st2084_pq";
        break;
    case NamedTransferFunction::transfer_function_st428:
        debug.nospace() << "transfer_function_st428";
        break;
    }
    debug.nospace() << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const Luminance &value) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "Luminance(minLuminance: " << value.minLuminance
                    << ", maxLuminance: " << value.maxLuminance
                    << ", referenceLuminance: " << value.referenceLuminance << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const MasteringLuminance &value) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "MasteringLuminance(minLuminance: " << value.minLuminance
                    << ", maxLuminance: " << value.maxLuminance << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const ColorSpace &value) {
    QDebugStateSaver saver(debug);

    debug.nospace() << "ColorSpace(";
    std::visit([&] (auto &&v) {debug.nospace() << "primaries: " << v; }, value.primaries);
    std::visit([&] (auto &&v) {debug.nospace() << ", transferFunction: " << v; }, value.transferFunction);
    if (value.luminance) {
        debug.nospace() << ", luminance: " << value.luminance;
    } else {
        debug.nospace() << ", luminance: " << "<none>";
    }
    debug.nospace() << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const MasteringInfo &value) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "MasteringInfo(primaries: " << value.primaries
                    << ", luminance: " << value.luminance;
    if (value.maxCll) {
        debug.nospace() << ", maxCll: " << *value.maxCll;
    }
    if (value.maxFall) {
        debug.nospace() << ", maxFall: " << *value.maxFall;
    }
    debug.nospace() << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const SurfaceDescription &value) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "SurfaceDescription(colorSpace: " << value.colorSpace;
    if (value.masteringInfo) {
        debug.nospace() << ", masteringInfo: " << *value.masteringInfo;
    }
    debug.nospace() << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const KisSurfaceColorimetry::RenderIntent &value) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "RenderIntent(";
    switch (value) {
    case KisSurfaceColorimetry::RenderIntent::render_intent_perceptual:
        debug.nospace() << "render_intent_perceptual";
        break;
    case KisSurfaceColorimetry::RenderIntent::render_intent_relative:
        debug.nospace() << "render_intent_relative";
        break;
    case KisSurfaceColorimetry::RenderIntent::render_intent_saturation:
        debug.nospace() << "render_intent_saturation";
        break;
    case KisSurfaceColorimetry::RenderIntent::render_intent_absolute:
        debug.nospace() << "render_intent_absolute";
        break;
    case KisSurfaceColorimetry::RenderIntent::render_intent_relative_bpc:
        debug.nospace() << "render_intent_relative_bpc";
        break;
    }
    debug.nospace() << ")";
    return debug;
}

QString SurfaceDescription::makeTextReport() const
{
    QString report;
    QDebug str(&report);

    str << "  Color Space:" << Qt::endl;

    if (std::holds_alternative<KisSurfaceColorimetry::NamedPrimaries>(this->colorSpace.primaries)) {
        str << "    Primaries: " << std::get<KisSurfaceColorimetry::NamedPrimaries>(this->colorSpace.primaries) << Qt::endl;
    } else {
        auto col = std::get<KisSurfaceColorimetry::Colorimetry>(this->colorSpace.primaries);
        str << "    Primaries: " << Qt::endl;
        str << "        Red: " << col.red().toxy() << Qt::endl;
        str << "        Green: " << col.green().toxy() << Qt::endl;
        str << "        Blue: " << col.blue().toxy() << Qt::endl;
        str << "        White: " << col.white().toxy() << Qt::endl;
    }

    if (std::holds_alternative<KisSurfaceColorimetry::NamedTransferFunction>(this->colorSpace.transferFunction)) {
        str << "    Transfer Function: " << std::get<KisSurfaceColorimetry::NamedTransferFunction>(this->colorSpace.transferFunction) << Qt::endl;
    } else {
        const uint32_t rawValue = std::get<uint32_t>(this->colorSpace.transferFunction);
        str << "    Transfer Function (gamma): " << rawValue << "(" << qreal(rawValue) / 10000.0 << ")" << Qt::endl;
    }

    if (this->colorSpace.luminance) {
        str << "    Luminance: " << *this->colorSpace.luminance << Qt::endl;
    } else {
        str << "    Luminance: " << "<none>" << Qt::endl;
    }

    if (this->masteringInfo) {
        str << "  Mastering Info:" << Qt::endl;
        auto col = this->masteringInfo->primaries;
        str << "    Primaries: " << Qt::endl;
        str << "        Red: " << col.red().toxy() << Qt::endl;
        str << "        Green: " << col.green().toxy() << Qt::endl;
        str << "        Blue: " << col.blue().toxy() << Qt::endl;
        str << "        White: " << col.white().toxy() << Qt::endl;
        str << "    Luminance: " << this->masteringInfo->luminance << Qt::endl;
        str << "    Max CLL: " << (this->masteringInfo->maxCll ? QString::number(*this->masteringInfo->maxCll) : "<none>") << Qt::endl;
        str << "    Max FALL: " << (this->masteringInfo->maxFall ? QString::number(*this->masteringInfo->maxFall) : "<none>") << Qt::endl;
    } else {
        str << "  Mastering Info: <none>" << Qt::endl;
    }

    return report;
}

} // namespace KisSurfaceColorimetry

/* KISSURFACECOLORIMETRY_H */