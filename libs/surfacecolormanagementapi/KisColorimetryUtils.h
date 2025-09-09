/*
 *  SPDX-FileCopyrightText: 2023 Xaver Hugl <xaver.hugl@gmail.com>
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCOLORIMETRYUTILS_H
#define KISCOLORIMETRYUTILS_H

#include <QVector2D>
#include <QVector3D>
#include <QMatrix4x4>
#include <boost/operators.hpp>
#include <QDebug>

#include <kritasurfacecolormanagementapi_export.h>

namespace KisColorimetryUtils
{

// these functions were taken from KWin's colorimetry code
// https://invent.kde.org/plasma/kwin/-/blob/master/src/core/colorspace.h

QMatrix4x4 matrixFromColumns(const QVector3D &first, const QVector3D &second, const QVector3D &third);

struct XYZ;

/**
 * xyY, with Y unspecified
 */
struct KRITASURFACECOLORMANAGEMENTAPI_EXPORT xy {
    double x;
    double y;

    XYZ toXYZ() const;
    QVector2D asVector() const;
    bool operator==(const xy &other) const;
};

struct KRITASURFACECOLORMANAGEMENTAPI_EXPORT xyY {
    double x;
    double y;
    double Y;

    XYZ toXYZ() const;
    bool operator==(const xyY &other) const;
};

struct KRITASURFACECOLORMANAGEMENTAPI_EXPORT XYZ {
    double X;
    double Y;
    double Z;

    xyY toxyY() const;
    xy toxy() const;
    QVector3D asVector() const;
    XYZ operator*(double factor) const;
    XYZ operator/(double factor) const;
    XYZ operator+(const XYZ &other) const;
    bool operator==(const XYZ &other) const;

    static XYZ fromVector(const QVector3D &vector);
};

/**
 * Describes the definition of colors in a color space.
 * Red, green and blue define the chromaticities ("absolute colors") of the red, green and blue LEDs on a display in xy coordinates
 * White defines the the chromaticity of the reference white in xy coordinates
 */
class KRITASURFACECOLORMANAGEMENTAPI_EXPORT Colorimetry
{
public:
    static const Colorimetry BT709;
    static const Colorimetry PAL_M;
    static const Colorimetry PAL;
    static const Colorimetry NTSC;
    static const Colorimetry GenericFilm;
    static const Colorimetry BT2020;
    static const Colorimetry CIEXYZ;
    static const Colorimetry DCIP3;
    static const Colorimetry DisplayP3;
    static const Colorimetry AdobeRGB;

    /**
     * @returns a matrix adapting XYZ values from the source whitepoint to the destination whitepoint with the Bradford transform
     */
    static QMatrix4x4 chromaticAdaptationMatrix(XYZ sourceWhitepoint, XYZ destinationWhitepoint);

    static QMatrix4x4 calculateToXYZMatrix(XYZ red, XYZ green, XYZ blue, XYZ white);

    /**
     * checks if the colorimetry is sane and won't cause crashes or glitches
     */
    static bool isValid(xy red, xy green, xy blue, xy white);
    /**
     * checks if the colorimetry could be from a real display
     */
    static bool isReal(xy red, xy green, xy blue, xy white);

    explicit Colorimetry(XYZ red, XYZ green, XYZ blue, XYZ white);
    explicit Colorimetry(xyY red, xyY green, xyY blue, xyY white);
    explicit Colorimetry(xy red, xy green, xy blue, xy white);

    /**
     * @returns a matrix that transforms from the linear RGB representation of colors in this colorimetry to the XYZ representation
     */
    const QMatrix4x4 &toXYZ() const;
    /**
     * @returns a matrix that transforms from the XYZ representation to the linear RGB representation of colors in this colorimetry
     */
    const QMatrix4x4 &fromXYZ() const;
    QMatrix4x4 toLMS() const;
    QMatrix4x4 fromLMS() const;

    bool operator==(const Colorimetry &other) const;
    /**
     * @returns this colorimetry, adapted to the new whitepoint using the Bradford transform
     */
    Colorimetry adaptedTo(xyY newWhitepoint) const;
    /**
     * replaces the current whitepoint with the new one
     * this does not do whitepoint adaptation!
     */
    Colorimetry withWhitepoint(xyY newWhitePoint) const;
    /**
     * interpolates the primaries depending on the passed factor. The whitepoint stays unchanged
     */
    Colorimetry interpolateGamutTo(const Colorimetry &one, double factor) const;

    QMatrix4x4 relativeColorimetricTo(const Colorimetry &other) const;
    QMatrix4x4 absoluteColorimetricTo(const Colorimetry &other) const;

    const XYZ &red() const;
    const XYZ &green() const;
    const XYZ &blue() const;
    const XYZ &white() const;

private:
    XYZ m_red;
    XYZ m_green;
    XYZ m_blue;
    XYZ m_white;
    QMatrix4x4 m_toXYZ;
    QMatrix4x4 m_fromXYZ;
};

KRITASURFACECOLORMANAGEMENTAPI_EXPORT QDebug operator<<(QDebug debug, const xy &value);
KRITASURFACECOLORMANAGEMENTAPI_EXPORT QDebug operator<<(QDebug debug, const xyY &value);
KRITASURFACECOLORMANAGEMENTAPI_EXPORT QDebug operator<<(QDebug debug, const XYZ &value);
KRITASURFACECOLORMANAGEMENTAPI_EXPORT QDebug operator<<(QDebug debug, const Colorimetry &value);

} // namespace KisColorimetryUtils

#endif /* KISCOLORIMETRYUTILS_H */