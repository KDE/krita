/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef _KIS_COLOR_H_
#define _KIS_COLOR_H_

#include <qcolor.h>
#include <koffice_export.h>
#include "ksharedptr.h"

#include "kis_global.h"
#include "kis_profile.h"
#include "kis_colorspace.h"


/**
 * A KisColor describes a color in a certain colorspace.
 *
 */
class KRITACOLOR_EXPORT KisColor {

public:
    /// Create an empty KisColor. It will be valid, but also black and transparent
    KisColor();

    virtual ~KisColor();

    /// Create a KisColor from a QColor. The QColor is immediately converted to native. The QColor
    /// is assumed to have the current monitor profile.
    KisColor(const QColor & color, KisColorSpace * colorSpace);

    /// Create a KisColor from a QColor. The QColor is immediately converted to native. The QColor
    /// is assumed to have the current monitor profile.
    KisColor(const QColor & color, quint8 alpha, KisColorSpace * colorSpace);

    /// Create a KisColor using a native color strategy. The data is copied.
    KisColor(const quint8 * data, KisColorSpace * colorSpace);

    /// Create a KisColor by converting src into another colorspace
    KisColor(const KisColor &src, KisColorSpace * colorSpace);

    /// Copy constructor -- deep copies the colors.
    KisColor(const KisColor & rhs);

    /// Effective C++, item 11
    KisColor &operator=(const KisColor &);

    /// For easy memcpy'ing etc.
    quint8 * data() const { return m_data; }

    KisColorSpace * colorSpace() const { return m_colorSpace; }

    KisProfile *  profile() const { return m_colorSpace->getProfile(); }

    /// Convert this KisColor to the specified colorspace. If the specified colorspace is the
    /// same as the original colorspace, do nothing. Returns the converted KisColor.
    void convertTo(KisColorSpace * cs);

    /// Replace the existing color data, and colorspace with the specified data.
    void setColor(quint8 * data, KisColorSpace * colorSpace = 0);

    /// To save the user the trouble of doing color->colorSpace()->toQColor(color->data(), &c, &a
    void toQColor(QColor *c) const;
    void toQColor(QColor *c, quint8 *opacity) const;

    QColor toQColor() const;

    void dump() const;

private:

    quint8 * m_data;

    KisColorSpace * m_colorSpace;
};

#endif
