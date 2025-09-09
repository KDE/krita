/*
 *  SPDX-FileCopyrightText: 2023 Xaver Hugl <xaver.hugl@gmail.com>
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisColorimetryUtils.h"

#include <QDebug>

namespace KisColorimetryUtils
{

QMatrix4x4 matrixFromColumns(const QVector3D &first, const QVector3D &second, const QVector3D &third)
{
    QMatrix4x4 ret;
    ret(0, 0) = first.x();
    ret(1, 0) = first.y();
    ret(2, 0) = first.z();
    ret(0, 1) = second.x();
    ret(1, 1) = second.y();
    ret(2, 1) = second.z();
    ret(0, 2) = third.x();
    ret(1, 2) = third.y();
    ret(2, 2) = third.z();
    return ret;
}

XYZ xy::toXYZ() const
{
    if (qFuzzyIsNull(y)) {
        return XYZ{0, 0, 0};
    }
    return XYZ{
        .X = x / y,
        .Y = 1.0,
        .Z = (1 - x - y) / y,
    };
}

QVector2D xy::asVector() const
{
    return QVector2D(x, y);
}

bool xy::operator==(const xy &other) const
{
    return qFuzzyCompare(x, other.x) && qFuzzyCompare(y, other.y);
}

XYZ xyY::toXYZ() const
{
    if (qFuzzyIsNull(y)) {
        return XYZ{0, 0, 0};
    }
    return XYZ{
        .X = Y * x / y,
        .Y = Y,
        .Z = Y * (1 - x - y) / y,
    };
}

bool xyY::operator==(const xyY &other) const
{
    return qFuzzyCompare(x, other.x) && qFuzzyCompare(y, other.y) && qFuzzyCompare(Y, other.Y);
}

xyY XYZ::toxyY() const
{
    const double sum = X + Y + Z;
    if (qFuzzyIsNull(sum)) {
        // this is nonsense, but at least won't crash
        return xyY{
            .x = 0,
            .y = 0,
            .Y = 1,
        };
    }
    return xyY{
        .x = X / sum,
        .y = Y / sum,
        .Y = Y,
    };
}

xy XYZ::toxy() const
{
    const double sum = X + Y + Z;
    if (qFuzzyIsNull(sum)) {
        // this is nonsense, but at least won't crash
        return xy{
            .x = 0,
            .y = 0,
        };
    }
    return xy{
        .x = X / sum,
        .y = Y / sum,
    };
}

XYZ XYZ::operator*(double factor) const
{
    return XYZ{
        .X = X * factor,
        .Y = Y * factor,
        .Z = Z * factor,
    };
}

XYZ XYZ::operator/(double divisor) const
{
    return XYZ{
        .X = X / divisor,
        .Y = Y / divisor,
        .Z = Z / divisor,
    };
}

XYZ XYZ::operator+(const XYZ &other) const
{
    return XYZ{
        .X = X + other.X,
        .Y = Y + other.Y,
        .Z = Z + other.Z,
    };
}

QVector3D XYZ::asVector() const
{
    return QVector3D(X, Y, Z);
}

XYZ XYZ::fromVector(const QVector3D &vector)
{
    return XYZ{
        .X = vector.x(),
        .Y = vector.y(),
        .Z = vector.z(),
    };
}

bool XYZ::operator==(const XYZ &other) const
{
    return qFuzzyCompare(X, other.X) && qFuzzyCompare(Y, other.Y) && qFuzzyCompare(Z, other.Z);
}

QMatrix4x4 Colorimetry::chromaticAdaptationMatrix(XYZ sourceWhitepoint, XYZ destinationWhitepoint)
{
    static const QMatrix4x4 bradford = []() {
        QMatrix4x4 ret;
        ret(0, 0) = 0.8951;
        ret(0, 1) = 0.2664;
        ret(0, 2) = -0.1614;
        ret(1, 0) = -0.7502;
        ret(1, 1) = 1.7135;
        ret(1, 2) = 0.0367;
        ret(2, 0) = 0.0389;
        ret(2, 1) = -0.0685;
        ret(2, 2) = 1.0296;
        return ret;
    }();
    static const QMatrix4x4 inverseBradford = []() {
        QMatrix4x4 ret;
        ret(0, 0) = 0.9869929;
        ret(0, 1) = -0.1470543;
        ret(0, 2) = 0.1599627;
        ret(1, 0) = 0.4323053;
        ret(1, 1) = 0.5183603;
        ret(1, 2) = 0.0492912;
        ret(2, 0) = -0.0085287;
        ret(2, 1) = 0.0400428;
        ret(2, 2) = 0.9684867;
        return ret;
    }();
    if (sourceWhitepoint == destinationWhitepoint) {
        return QMatrix4x4{};
    }
    const QVector3D factors = (bradford.map(destinationWhitepoint.asVector())) / (bradford.map(sourceWhitepoint.asVector()));
    QMatrix4x4 adaptation{};
    adaptation(0, 0) = factors.x();
    adaptation(1, 1) = factors.y();
    adaptation(2, 2) = factors.z();
    return inverseBradford * adaptation * bradford;
}

QMatrix4x4 Colorimetry::calculateToXYZMatrix(XYZ red, XYZ green, XYZ blue, XYZ white)
{
    const QVector3D r = red.asVector();
    const QVector3D g = green.asVector();
    const QVector3D b = blue.asVector();
    const auto component_scale = (matrixFromColumns(r, g, b)).inverted().map(white.asVector());
    return matrixFromColumns(r * component_scale.x(), g * component_scale.y(), b * component_scale.z());
}

Colorimetry Colorimetry::interpolateGamutTo(const Colorimetry &one, double factor) const
{
    return Colorimetry{
        m_red * (1 - factor) + one.red() * factor,
        m_green * (1 - factor) + one.green() * factor,
        m_blue * (1 - factor) + one.blue() * factor,
        m_white, // whitepoint should stay the same
    };
}

static double triangleArea(QVector2D p1, QVector2D p2, QVector2D p3)
{
    return std::abs(0.5 * (p1.x() * (p2.y() - p3.y()) + p2.x() * (p3.y() - p1.y()) + p3.x() * (p1.y() - p2.y())));
}

bool Colorimetry::isValid(xy red, xy green, xy blue, xy white)
{
    // this is more of a heuristic than a hard rule
    // but if the gamut is too small, it's not really usable
    const double gamutArea = triangleArea(red.asVector(), green.asVector(), blue.asVector());
    if (gamutArea < 0.02) {
        return false;
    }
    // if the white point is inside the gamut triangle,
    // the three triangles made up between the primaries and the whitepoint
    // must have the same area as the gamut triangle
    const double area1 = triangleArea(white.asVector(), green.asVector(), blue.asVector());
    const double area2 = triangleArea(red.asVector(), white.asVector(), blue.asVector());
    const double area3 = triangleArea(red.asVector(), green.asVector(), white.asVector());
    if (std::abs(area1 + area2 + area3 - gamutArea) > 0.001) {
        // this would cause terrible glitches
        return false;
    }
    return true;
}

bool Colorimetry::isReal(xy red, xy green, xy blue, xy white)
{
    if (!isValid(red, green, blue, white)) {
        return false;
    }
    // outside of XYZ definitely can't be shown on a display
    // TODO maybe calculate if all values are within the human-visible gamut too?
    if (red.x < 0 || red.x > 1 || red.y < 0 || red.y > 1 || green.x < 0 || green.x > 1 || green.y < 0 || green.y > 1 || blue.x < 0 || blue.x > 1 || blue.y < 0
        || blue.y > 1 || white.x < 0 || white.x > 1 || white.y < 0 || white.y > 1) {
        return false;
    }
    return true;
}

Colorimetry::Colorimetry(XYZ red, XYZ green, XYZ blue, XYZ white)
    : m_red(red)
    , m_green(green)
    , m_blue(blue)
    , m_white(white)
    , m_toXYZ(calculateToXYZMatrix(red, green, blue, white))
    , m_fromXYZ(m_toXYZ.inverted())
{
}

Colorimetry::Colorimetry(xyY red, xyY green, xyY blue, xyY white)
    : Colorimetry(red.toXYZ(), green.toXYZ(), blue.toXYZ(), white.toXYZ())
{
}

Colorimetry::Colorimetry(xy red, xy green, xy blue, xy white)
    : m_white(xyY{white.x, white.y, 1.0}.toXYZ())
{
    const auto brightness = (matrixFromColumns(xyY{red.x, red.y, 1.0}.toXYZ().asVector(),
                                               xyY{green.x, green.y, 1.0}.toXYZ().asVector(),
                                               xyY{blue.x, blue.y, 1.0}.toXYZ().asVector()))
                                .inverted().map(
        xyY{white.x, white.y, 1.0}.toXYZ().asVector());
    m_red = xyY{red.x, red.y, brightness.x()}.toXYZ();
    m_green = xyY{green.x, green.y, brightness.y()}.toXYZ();
    m_blue = xyY{blue.x, blue.y, brightness.z()}.toXYZ();
    m_toXYZ = calculateToXYZMatrix(m_red, m_green, m_blue, m_white);
    m_fromXYZ = m_toXYZ.inverted();
}

const QMatrix4x4 &Colorimetry::toXYZ() const
{
    return m_toXYZ;
}

const QMatrix4x4 &Colorimetry::fromXYZ() const
{
    return m_fromXYZ;
}

// converts from XYZ to LMS suitable for ICtCp
static const QMatrix4x4 s_xyzToDolbyLMS = []() {
    QMatrix4x4 ret;
    ret(0, 0) = 0.3593;
    ret(0, 1) = 0.6976;
    ret(0, 2) = -0.0359;
    ret(1, 0) = -0.1921;
    ret(1, 1) = 1.1005;
    ret(1, 2) = 0.0754;
    ret(2, 0) = 0.0071;
    ret(2, 1) = 0.0748;
    ret(2, 2) = 0.8433;
    return ret;
}();
static const QMatrix4x4 s_inverseDolbyLMS = s_xyzToDolbyLMS.inverted();

QMatrix4x4 Colorimetry::toLMS() const
{
    return s_xyzToDolbyLMS * m_toXYZ;
}

QMatrix4x4 Colorimetry::fromLMS() const
{
    return m_fromXYZ * s_inverseDolbyLMS;
}

Colorimetry Colorimetry::adaptedTo(xyY newWhitepoint) const
{
    const auto mat = chromaticAdaptationMatrix(this->white(), newWhitepoint.toXYZ());
    return Colorimetry{
        XYZ::fromVector(mat.map(red().asVector())),
        XYZ::fromVector(mat.map(green().asVector())),
        XYZ::fromVector(mat.map(blue().asVector())),
        newWhitepoint.toXYZ(),
    };
}

Colorimetry Colorimetry::withWhitepoint(xyY newWhitePoint) const
{
    newWhitePoint.Y = 1;
    return Colorimetry{
        m_red,
        m_green,
        m_blue,
        newWhitePoint.toXYZ(),
    };
}

QMatrix4x4 Colorimetry::relativeColorimetricTo(const Colorimetry &other) const
{
    return other.fromXYZ() * chromaticAdaptationMatrix(white(), other.white()) * toXYZ();
}

QMatrix4x4 Colorimetry::absoluteColorimetricTo(const Colorimetry &other) const
{
    return other.fromXYZ() * toXYZ();
}

bool Colorimetry::operator==(const Colorimetry &other) const
{
    return red() == other.red() && green() == other.green() && blue() == other.blue() && white() == other.white();
}

const XYZ &Colorimetry::red() const
{
    return m_red;
}

const XYZ &Colorimetry::green() const
{
    return m_green;
}

const XYZ &Colorimetry::blue() const
{
    return m_blue;
}

const XYZ &Colorimetry::white() const
{
    return m_white;
}

const Colorimetry Colorimetry::BT709 = Colorimetry{
    xy{0.64, 0.33},
    xy{0.30, 0.60},
    xy{0.15, 0.06},
    xy{0.3127, 0.3290},
};
const Colorimetry Colorimetry::PAL_M = Colorimetry{
    xy{0.67, 0.33},
    xy{0.21, 0.71},
    xy{0.14, 0.08},
    xy{0.310, 0.316},
};
const Colorimetry Colorimetry::PAL = Colorimetry{
    xy{0.640, 0.330},
    xy{0.290, 0.600},
    xy{0.150, 0.060},
    xy{0.3127, 0.3290},
};
const Colorimetry Colorimetry::NTSC = Colorimetry{
    xy{0.630, 0.340},
    xy{0.310, 0.595},
    xy{0.155, 0.070},
    xy{0.3127, 0.3290},
};
const Colorimetry Colorimetry::GenericFilm = Colorimetry{
    xy{0.681, 0.319},
    xy{0.243, 0.692},
    xy{0.145, 0.049},
    xy{0.310, 0.316},
};
const Colorimetry Colorimetry::BT2020 = Colorimetry{
    xy{0.708, 0.292},
    xy{0.170, 0.797},
    xy{0.131, 0.046},
    xy{0.3127, 0.3290},
};
const Colorimetry Colorimetry::CIEXYZ = Colorimetry{
    XYZ{1.0, 0.0, 0.0},
    XYZ{0.0, 1.0, 0.0},
    XYZ{0.0, 0.0, 1.0},
    xy{1.0 / 3.0, 1.0 / 3.0}.toXYZ(),
};
const Colorimetry Colorimetry::DCIP3 = Colorimetry{
    xy{0.680, 0.320},
    xy{0.265, 0.690},
    xy{0.150, 0.060},
    xy{0.314, 0.351},
};
const Colorimetry Colorimetry::DisplayP3 = Colorimetry{
    xy{0.680, 0.320},
    xy{0.265, 0.690},
    xy{0.150, 0.060},
    xy{0.3127, 0.3290},
};
const Colorimetry Colorimetry::AdobeRGB = Colorimetry{
    xy{0.6400, 0.3300},
    xy{0.2100, 0.7100},
    xy{0.1500, 0.0600},
    xy{0.3127, 0.3290},
};

QDebug operator<<(QDebug debug, const xy &value) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "xy(x: " << value.x << ", y: " << value.y << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const xyY &value) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "xyY(x: " << value.x << ", y: " << value.y << ", Y: " << value.Y << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const XYZ &value) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "XYZ(X: " << value.X << ", Y: " << value.Y << ", Z: " << value.Z << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const Colorimetry &value) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "Colorimetry(Red: " << value.red().toxy() << ", Green: " << value.green().toxy() << ", Blue: " << value.blue().toxy() << ", White: " << value.white().toxy() << ")";
    return debug;
}

} // namespace KisColorimetryUtils