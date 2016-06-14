/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QRGBA64_H
#define QRGBA64_H

#include <QtCore/qglobal.h>
#include <QtCore/qprocessordetection.h>

QT_BEGIN_NAMESPACE

class QRgba64 {
    quint64 rgba;

    // Make sure that the representation always has the order: red green blue alpha, independent
    // of byte order. This way, vector operations that assume 4 16-bit values see the correct ones.
    enum {
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
        RedShift = 48,
        GreenShift = 32,
        BlueShift = 16,
        AlphaShift = 0
#else // little endian:
        RedShift = 0,
        GreenShift = 16,
        BlueShift = 32,
        AlphaShift = 48
#endif
    };

    // No constructors are allowed in C++98, since this needs to be usable in a union.
    // We however require one for constexprs in C++11/C++14
#ifdef Q_COMPILER_CONSTEXPR
    explicit  Q_DECL_CONSTEXPR QRgba64(quint64 c) : rgba(c) { }
#endif
public:
#ifdef Q_COMPILER_CONSTEXPR
     Q_DECL_CONSTEXPR QRgba64() : rgba(0) { }
#endif

    Q_DECL_CONSTEXPR static
    QRgba64 fromRgba64(quint64 c)
    {
#ifdef Q_COMPILER_CONSTEXPR
        return QRgba64(c);
#else
        QRgba64 rgba64;
        rgba64.rgba = c;
        return rgba64;
#endif
    }
    Q_DECL_CONSTEXPR static
    QRgba64 fromRgba64(quint16 red, quint16 green, quint16 blue, quint16 alpha)
    {
        return fromRgba64(quint64(red)   << RedShift
                        | quint64(green) << GreenShift
                        | quint64(blue)  << BlueShift
                        | quint64(alpha) << AlphaShift);
    }
    Q_DECL_RELAXED_CONSTEXPR static QRgba64 fromRgba(quint8 red, quint8 green, quint8 blue, quint8 alpha)
    {
        QRgba64 rgb64 = fromRgba64(red, green, blue, alpha);
        // Expand the range so that 0x00 maps to 0x0000 and 0xff maps to 0xffff.
        rgb64.rgba |= rgb64.rgba << 8;
        return rgb64;
    }
    Q_DECL_RELAXED_CONSTEXPR static
    QRgba64 fromArgb32(uint rgb)
    {
        return fromRgba(rgb >> 16, rgb >> 8, rgb, rgb >> 24);
    }

    Q_DECL_CONSTEXPR bool isOpaque() const
    {
        return (rgba & alphaMask()) == alphaMask();
    }
    Q_DECL_CONSTEXPR bool isTransparent() const
    {
        return (rgba & alphaMask()) == 0;
    }

    Q_DECL_CONSTEXPR quint16 red()   const { return rgba >> RedShift;   }
    Q_DECL_CONSTEXPR quint16 green() const { return rgba >> GreenShift; }
    Q_DECL_CONSTEXPR quint16 blue()  const { return rgba >> BlueShift;  }
    Q_DECL_CONSTEXPR quint16 alpha() const { return rgba >> AlphaShift; }
    void setRed(quint16 _red)     { rgba = (rgba & ~(Q_UINT64_C(0xffff) << RedShift))   | (quint64(_red) << RedShift); }
    void setGreen(quint16 _green) { rgba = (rgba & ~(Q_UINT64_C(0xffff) << GreenShift)) | (quint64(_green) << GreenShift); }
    void setBlue(quint16 _blue)   { rgba = (rgba & ~(Q_UINT64_C(0xffff) << BlueShift))  | (quint64(_blue) << BlueShift); }
    void setAlpha(quint16 _alpha) { rgba = (rgba & ~(Q_UINT64_C(0xffff) << AlphaShift)) | (quint64(_alpha) << AlphaShift); }

    Q_DECL_CONSTEXPR quint8 red8()   const { return div_257(red()); }
    Q_DECL_CONSTEXPR quint8 green8() const { return div_257(green()); }
    Q_DECL_CONSTEXPR quint8 blue8()  const { return div_257(blue()); }
    Q_DECL_CONSTEXPR quint8 alpha8() const { return div_257(alpha()); }
    Q_DECL_CONSTEXPR uint toArgb32() const
    {
        return (alpha8() << 24) | (red8() << 16) | (green8() << 8) | blue8();
    }
    Q_DECL_CONSTEXPR ushort toRgb16() const
    {
        return (red() & 0xf800) | ((green() >> 10) << 5) | (blue() >> 11);
    }

    Q_DECL_RELAXED_CONSTEXPR QRgba64 premultiplied() const
    {
        const quint32 a = alpha();
        const quint16 r = div_65535(red()   * a);
        const quint16 g = div_65535(green() * a);
        const quint16 b = div_65535(blue()  * a);
        return fromRgba64(r, g, b, a);
    }

    Q_DECL_RELAXED_CONSTEXPR QRgba64 unpremultiplied() const
    {
#if Q_PROCESSOR_WORDSIZE < 8
        return unpremultiplied_32bit();
#else
        return unpremultiplied_64bit();
#endif
    }

    Q_DECL_CONSTEXPR operator quint64() const
    {
        return rgba;
    }

    QRgba64 operator=(quint64 _rgba)
    {
        rgba = _rgba;
        return *this;
    }

private:
    static Q_DECL_CONSTEXPR  quint64 alphaMask() { return Q_UINT64_C(0xffff) << AlphaShift; }

    static Q_DECL_CONSTEXPR  uint div_257_floor(uint x) { return  (x - (x >> 8)) >> 8; }
    static Q_DECL_CONSTEXPR  uint div_257(uint x) { return div_257_floor(x + 128); }
    static Q_DECL_CONSTEXPR  uint div_65535(uint x) { return (x + (x>>16) + 0x8000U) >> 16; }
    Q_DECL_RELAXED_CONSTEXPR  QRgba64 unpremultiplied_32bit() const
    {
        if (isOpaque() || isTransparent())
            return *this;
        const quint32 a = alpha();
        const quint16 r = (quint32(red())   * 0xffff + a/2) / a;
        const quint16 g = (quint32(green()) * 0xffff + a/2) / a;
        const quint16 b = (quint32(blue())  * 0xffff + a/2) / a;
        return fromRgba64(r, g, b, a);
    }
    Q_DECL_RELAXED_CONSTEXPR  QRgba64 unpremultiplied_64bit() const
    {
        if (isOpaque() || isTransparent())
            return *this;
        const quint64 a = alpha();
        const quint64 fa = (Q_UINT64_C(0xffff00008000) + a/2) / a;
        const quint16 r = (red()   * fa + 0x80000000) >> 32;
        const quint16 g = (green() * fa + 0x80000000) >> 32;
        const quint16 b = (blue()  * fa + 0x80000000) >> 32;
        return fromRgba64(r, g, b, a);
    }
};

Q_DECLARE_TYPEINFO(QRgba64, Q_PRIMITIVE_TYPE);

Q_DECL_CONSTEXPR inline QRgba64 qRgba64(quint16 r, quint16 g, quint16 b, quint16 a)
{
    return QRgba64::fromRgba64(r, g, b, a);
}

Q_DECL_CONSTEXPR inline QRgba64 qRgba64(quint64 c)
{
    return QRgba64::fromRgba64(c);
}

Q_DECL_RELAXED_CONSTEXPR inline QRgba64 qPremultiply(QRgba64 c)
{
    return c.premultiplied();
}

Q_DECL_RELAXED_CONSTEXPR inline QRgba64 qUnpremultiply(QRgba64 c)
{
    return c.unpremultiplied();
}

inline Q_DECL_CONSTEXPR uint qRed(QRgba64 rgb)
{ return rgb.red8(); }

inline Q_DECL_CONSTEXPR uint qGreen(QRgba64 rgb)
{ return rgb.green8(); }

inline Q_DECL_CONSTEXPR uint qBlue(QRgba64 rgb)
{ return rgb.blue8(); }

inline Q_DECL_CONSTEXPR uint qAlpha(QRgba64 rgb)
{ return rgb.alpha8(); }

QT_END_NAMESPACE

#endif // QRGBA64_H
