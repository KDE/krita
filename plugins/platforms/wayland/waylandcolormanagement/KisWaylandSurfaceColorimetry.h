/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISWAYLANDSURFACECOLORIMETRY_H
#define KISWAYLANDSURFACECOLORIMETRY_H

#include <surfacecolormanagement/KisSurfaceColorimetry.h>
#include <qwayland-color-management-v1.h>

namespace KisSurfaceColorimetry
{

struct WaylandPrimaries : boost::equality_comparable<WaylandPrimaries> {
    using xy = KisColorimetryUtils::xy;
    using Colorimetry = KisColorimetryUtils::Colorimetry;

    xy red;
    xy green;
    xy blue;
    xy white;

    bool operator==(const WaylandPrimaries &other) const {
        return red == other.red &&
               green == other.green &&
               blue == other.blue &&
               white == other.white;
    }

    Colorimetry toColorimetry() const {
        return Colorimetry(red, green, blue, white);
    }

    static WaylandPrimaries fromColorimetry(const Colorimetry &colorimetry) {
        WaylandPrimaries result;

        result.red = colorimetry.red().toxy();
        result.green = colorimetry.green().toxy();
        result.blue = colorimetry.blue().toxy();
        result.white = colorimetry.white().toxy();

        return result;
    }
};


inline KisColorimetryUtils::xy xyFromWaylandXy(int32_t x, int32_t y) {
    return {x / 1'000'000.0, y / 1'000'000.0};
}

inline std::pair<int32_t, int32_t> waylandXyFromXy(KisColorimetryUtils::xy value) {
    return {std::rint(value.x * 1'000'000.0), std::rint(value.y * 1'000'000.0)};
}


struct WaylandSurfaceDescription : boost::equality_comparable<WaylandSurfaceDescription>
{
    using transfer_function = QtWayland::wp_color_manager_v1::transfer_function;
    using primaries = QtWayland::wp_color_manager_v1::primaries;

    std::optional<uint32_t> tfGamma;
    std::optional<transfer_function> tfNamed;

    std::optional<WaylandPrimaries> container;
    std::optional<primaries> namedContainer;

    std::optional<WaylandPrimaries> target;

    std::optional<KisSurfaceColorimetry::Luminance> luminances;
    std::optional<KisSurfaceColorimetry::MasteringLuminance> masteringLuminance;

    std::optional<uint32_t> targetMaxCLL;
    std::optional<uint32_t> targetMaxFALL;

    bool iccFileIsPresent = false;

    bool operator==(const WaylandSurfaceDescription &other) const {
        return tfGamma == other.tfGamma &&
               tfNamed == other.tfNamed &&
               container == other.container &&
               namedContainer == other.namedContainer &&
               target == other.target &&
               luminances == other.luminances &&
               masteringLuminance == other.masteringLuminance &&
               targetMaxCLL == other.targetMaxCLL &&
               targetMaxFALL == other.targetMaxFALL &&
               iccFileIsPresent == other.iccFileIsPresent;

    }

    SurfaceDescription toSurfaceDescription() const;
    static WaylandSurfaceDescription fromSurfaceDescription(const SurfaceDescription &desc);
};

QDebug operator<<(QDebug dbg, const WaylandPrimaries &points);
QDebug operator<<(QDebug dbg, const WaylandSurfaceDescription &data);

QtWayland::wp_color_manager_v1::primaries primariesKritaToWayland(KisSurfaceColorimetry::NamedPrimaries primaries);
QtWayland::wp_color_manager_v1::transfer_function transferFunctionKritaToWayland(KisSurfaceColorimetry::NamedTransferFunction transferFunction);
QtWayland::wp_color_manager_v1::render_intent renderIntentKritaToWayland(KisSurfaceColorimetry::RenderIntent intent);

}

QDebug operator<<(QDebug debug, QtWayland::wp_color_manager_v1::primaries p);
QDebug operator<<(QDebug debug, QtWayland::wp_color_manager_v1::feature f);
QDebug operator<<(QDebug debug, QtWayland::wp_color_manager_v1::render_intent ri);
QDebug operator<<(QDebug debug, QtWayland::wp_color_manager_v1::transfer_function tf);

#endif /* KISWAYLANDSURFACECOLORIMETRY_H */