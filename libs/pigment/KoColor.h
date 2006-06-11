/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#ifndef KOCOLOR_H
#define KOCOLOR_H

#include <QColor>
#include <koffice_export.h>

#include "KoColorProfile.h"
#include "KoColorSpace.h"


/**
 * A KoColor describes a color in a certain colorspace.
 *
 */
class PIGMENT_EXPORT KoColor {

public:
    /// Create an empty KoColor. It will be valid, but also black and transparent
    KoColor();

    virtual ~KoColor();

    /// Create a KoColor from a QColor. The QColor is immediately converted to native. The QColor
    /// is assumed to have the current monitor profile.
    KoColor(const QColor & color, KoColorSpace * colorSpace);

    /// Create a KoColor from a QColor. The QColor is immediately converted to native. The QColor
    /// is assumed to have the current monitor profile.
    KoColor(const QColor & color, quint8 alpha, KoColorSpace * colorSpace);

    /// Create a KoColor using a native color strategy. The data is copied.
    KoColor(const quint8 * data, KoColorSpace * colorSpace);

    /// Create a KoColor by converting src into another colorspace
    KoColor(const KoColor &src, KoColorSpace * colorSpace);

    /// Copy constructor -- deep copies the colors.
    KoColor(const KoColor & rhs);

    /// Effective C++, item 11
    KoColor &operator=(const KoColor &);

    /// For easy memcpy'ing etc.
    quint8 * data() const { return m_data; }

    KoColorSpace * colorSpace() const { return m_colorSpace; }

    KoColorProfile *  profile() const { return m_colorSpace->getProfile(); }

    /// Convert this KoColor to the specified colorspace. If the specified colorspace is the
    /// same as the original colorspace, do nothing. Returns the converted KoColor.
    void convertTo(KoColorSpace * cs);

    /// Replace the existing color data, and colorspace with the specified data.
    void setColor(quint8 * data, KoColorSpace * colorSpace = 0);

    /// To save the user the trouble of doing color->colorSpace()->toQColor(color->data(), &c, &a
    void toQColor(QColor *c) const;
    void toQColor(QColor *c, quint8 *opacity) const;

    QColor toQColor() const;

    void dump() const;

private:

    quint8 * m_data;

    KoColorSpace * m_colorSpace;
};

#endif
