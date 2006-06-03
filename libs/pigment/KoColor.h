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
#ifndef KOCOLOR_H_
#define KOCOLOR_H_

#include <QColor>
#include <koffice_export.h>

/**
 * A KoColor describes a color in a certain colorspace.
 *
 */
class PIGMENT_EXPORT KoColor {

public:
    /// Create an empty KoColor. It will be valid, but also black and transparent
    KoColor();

    virtual ~KoColor();

    /// Copy constructor -- deep copies the colors.
    KoColor(const KoColor & rhs);

    /// Effective C++, item 11
    KoColor &operator=(const KoColor &);

    /// To save the user the trouble of doing color->colorSpace()->toQColor(color->data(), &c, &a
    void toQColor(QColor *c) const;
    void toQColor(QColor *c, quint8 *opacity) const;

    QColor toQColor() const;

private:

    quint8 * m_data;
};

#endif
