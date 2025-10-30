/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSURFACECOLORIMETRY_H
#define KISSURFACECOLORIMETRY_H

#include <boost/operators.hpp>
#include <KisColorimetryUtils.h>


namespace KisSurfaceColorimetry
{
    using namespace KisColorimetryUtils;

    enum class NamedPrimaries {
        primaries_unknown = 0,
        primaries_srgb = 1,
        primaries_bt2020 = 6,
        primaries_dci_p3 = 8,
        primaries_display_p3 = 9,
        primaries_adobe_rgb = 10,
    };

    enum class NamedTransferFunction {
        transfer_function_unknown = 0,
        transfer_function_bt1886 = 1,
        transfer_function_gamma22 = 2, // supported by KDE
        transfer_function_gamma28 = 3,
        transfer_function_ext_linear = 5, // supported by KDE
        transfer_function_srgb = 9, // supported by KDE, but broken and deprecated
        transfer_function_ext_srgb = 10,
        transfer_function_st2084_pq = 11, // supported by KDE
        transfer_function_st428 = 12,
    };

    enum class RenderIntent {
        render_intent_perceptual = 0, // perceptual
        render_intent_relative = 1, // media-relative colorimetric
        render_intent_saturation = 2, // saturation
        render_intent_absolute = 3, // ICC-absolute colorimetric
        render_intent_relative_bpc = 4, // media-relative colorimetric + black point compensation
    };

    struct KRITASURFACECOLORMANAGEMENTAPI_EXPORT Luminance : boost::equality_comparable<Luminance> {
        Luminance() = default;
        Luminance(uint32_t minLuminanceArg, uint32_t maxLuminanceArg, uint32_t referenceLuminanceArg)
            : minLuminance(minLuminanceArg)
            , maxLuminance(maxLuminanceArg)
            , referenceLuminance(referenceLuminanceArg)
        {
        }

        uint32_t minLuminance = 2000; // (cd/m^2) * 10000
        uint32_t maxLuminance = 80; // (cd/m^2)
        uint32_t referenceLuminance = 80; // (cd/m^2)

        bool operator==(const Luminance &other) const {
            return minLuminance == other.minLuminance &&
                   maxLuminance == other.maxLuminance &&
                   referenceLuminance == other.referenceLuminance;
        }

        /**
         * When we need to to limit the luminance range to SDR-only,
         * we should use referenceLuminance for max luminance value
         */
        Luminance clipToSdr() const {
            return { minLuminance, referenceLuminance, referenceLuminance };
        }
    };

    struct KRITASURFACECOLORMANAGEMENTAPI_EXPORT MasteringLuminance : boost::equality_comparable<MasteringLuminance> {
        MasteringLuminance() = default;
        MasteringLuminance(uint32_t minLuminanceArg, uint32_t maxLuminanceArg)
            : minLuminance(minLuminanceArg)
            , maxLuminance(maxLuminanceArg)
        {}

        uint32_t minLuminance = 2000; // (cd/m^2) * 10000
        uint32_t maxLuminance = 80; // (cd/m^2)

        static MasteringLuminance fromLuminance(const Luminance &rhs) {
            return {rhs.minLuminance, rhs.maxLuminance};
        }

        bool operator==(const MasteringLuminance &other) const {
            return minLuminance == other.minLuminance &&
                   maxLuminance == other.maxLuminance;
        }
    };

    struct KRITASURFACECOLORMANAGEMENTAPI_EXPORT ColorSpace : boost::equality_comparable<ColorSpace> {
        std::variant<NamedPrimaries, Colorimetry> primaries;
        // named transfer function or <exponent * 10000>
        std::variant<NamedTransferFunction, uint32_t> transferFunction;
        std::optional<Luminance> luminance;

        bool operator==(const ColorSpace &other) const {
            return primaries == other.primaries &&
                   transferFunction == other.transferFunction &&
                   luminance == other.luminance;
        }

        bool isHDR() const {
            return luminance && luminance->maxLuminance > luminance->referenceLuminance;
        }
    };

    struct KRITASURFACECOLORMANAGEMENTAPI_EXPORT MasteringInfo : boost::equality_comparable<MasteringInfo> {
        Colorimetry primaries = Colorimetry::BT709;
        MasteringLuminance luminance;
        std::optional<uint32_t> maxCll;
        std::optional<uint32_t> maxFall;

        bool operator==(const MasteringInfo &other) const {
            return primaries == other.primaries &&
                   luminance == other.luminance &&
                   maxCll == other.maxCll &&
                   maxFall == other.maxFall;
        }
    };

    struct KRITASURFACECOLORMANAGEMENTAPI_EXPORT SurfaceDescription : boost::equality_comparable<SurfaceDescription> {
        ColorSpace colorSpace;
        std::optional<MasteringInfo> masteringInfo;

        bool operator==(const SurfaceDescription &other) const {
            return colorSpace == other.colorSpace &&
                   masteringInfo == other.masteringInfo;
        }

        QString makeTextReport() const;
    };

    KRITASURFACECOLORMANAGEMENTAPI_EXPORT QDebug operator<<(QDebug debug, const NamedPrimaries &value);
    KRITASURFACECOLORMANAGEMENTAPI_EXPORT QDebug operator<<(QDebug debug, const NamedTransferFunction &value);
    KRITASURFACECOLORMANAGEMENTAPI_EXPORT QDebug operator<<(QDebug debug, const Luminance &value);
    KRITASURFACECOLORMANAGEMENTAPI_EXPORT QDebug operator<<(QDebug debug, const MasteringLuminance &value);
    KRITASURFACECOLORMANAGEMENTAPI_EXPORT QDebug operator<<(QDebug debug, const ColorSpace &value);
    KRITASURFACECOLORMANAGEMENTAPI_EXPORT QDebug operator<<(QDebug debug, const MasteringInfo &value);
    KRITASURFACECOLORMANAGEMENTAPI_EXPORT QDebug operator<<(QDebug debug, const SurfaceDescription &value);
    KRITASURFACECOLORMANAGEMENTAPI_EXPORT QDebug operator<<(QDebug debug, const RenderIntent &value);
}

#endif /* KISSURFACECOLORIMETRY_H */