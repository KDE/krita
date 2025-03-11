/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
 #ifndef KISSURFACECOLORSPACEWRAPPER_H
 #define KISSURFACECOLORSPACEWRAPPER_H

#include <QtGlobal>
#include <QMetaObject>
#include <boost/operators.hpp>

 #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
 #include <QSurfaceFormat>
 #else
 #include <QColorSpace>
 #endif

class KisSurfaceColorSpaceWrapper : public boost::equality_comparable<KisSurfaceColorSpaceWrapper>
{
public:

    enum ColorSpace {
        DefaultColorSpace,
        sRGBColorSpace,
        scRGBColorSpace,
        bt2020PQColorSpace
    };
    Q_ENUMS(ColorSpace)

    constexpr KisSurfaceColorSpaceWrapper()
        : KisSurfaceColorSpaceWrapper(DefaultColorSpace)
    {
    }

    constexpr KisSurfaceColorSpaceWrapper(ColorSpace colorSpace) 
        : m_colorSpace(colorSpace)
    {
    }

    static constexpr  KisSurfaceColorSpaceWrapper makeSRGBColorSpace() {
        return { sRGBColorSpace };
    }

    static constexpr KisSurfaceColorSpaceWrapper makeSCRGBColorSpace() {
        return { scRGBColorSpace };
    }

    static constexpr KisSurfaceColorSpaceWrapper makeBt2020PQColorSpace() {
        return { bt2020PQColorSpace };
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    static constexpr KisSurfaceColorSpaceWrapper fromQtColorSpace(QSurfaceFormat::ColorSpace colorSpace)
    {
        return { static_cast<ColorSpace>(colorSpace) };
    }
#else
    static KisSurfaceColorSpaceWrapper fromQtColorSpace(const QColorSpace &colorSpace)
    {
        if (colorSpace == QColorSpace()) {
            return { DefaultColorSpace };
        } else if (colorSpace == QColorSpace::SRgb) {
            return { sRGBColorSpace };
        } else if (colorSpace == QColorSpace::SRgbLinear) {
            return { scRGBColorSpace };
        } else if (colorSpace == QColorSpace::Bt2100Pq) {
            return { bt2020PQColorSpace };
        } else {
            qWarning() << "WARNING: KisSurfaceColorSpaceWrapper: unsupported surface color space" << colorSpace;
            return { DefaultColorSpace };
        }
    }
#endif

    KisSurfaceColorSpaceWrapper(const KisSurfaceColorSpaceWrapper &rhs) = default;
    KisSurfaceColorSpaceWrapper(KisSurfaceColorSpaceWrapper &&rhs) = default;
    KisSurfaceColorSpaceWrapper& operator=(const KisSurfaceColorSpaceWrapper &rhs) = default;
    KisSurfaceColorSpaceWrapper& operator=(KisSurfaceColorSpaceWrapper &&rhs) = default;

    constexpr bool operator==(const KisSurfaceColorSpaceWrapper &rhs) const {
        return m_colorSpace == rhs.m_colorSpace;
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    constexpr operator QSurfaceFormat::ColorSpace() const {
        return static_cast<QSurfaceFormat::ColorSpace>(m_colorSpace);
    }
#else
    operator QColorSpace() const {
        switch (m_colorSpace) {
        case DefaultColorSpace:
            return QColorSpace();
        case sRGBColorSpace:
            return QColorSpace::SRgb;
        case scRGBColorSpace:
            return QColorSpace::SRgbLinear;
        case bt2020PQColorSpace:
            return QColorSpace::Bt2100Pq;
        }
        
        Q_UNREACHABLE_RETURN(QColorSpace());
    }
#endif

private:
    ColorSpace m_colorSpace;
};

 #endif /* KISSURFACECOLORSPACEWRAPPER_H */