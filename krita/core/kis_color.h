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
#include "kis_types.h"
#include "kis_profile.h"
#include "kis_abstract_colorspace.h"


/**
 * A KisColor describes a color in a certain colorspace.
 *
 */
class KRITACORE_EXPORT KisColor {

public:
    // Create an empty KisColor. It will be valid, but also black and transparent
    KisColor(); 
    
    virtual ~KisColor();

    // Create a KisColor on the basis of a QColor, assuming an sRGB profile for the result
    // and the monitor profile for the source.
    KisColor(const QColor & color);

    // Create a KisColor on the basis of a QColor, assuming an sRGB profile for the result
    // and the monitor profile for the source.
    KisColor(const QColor & color, Q_UINT8 opacity);

    // Create a KisColor from a QColor. The QColor is immediately converted to native. The QColor
    // is assumed to have the current monitor profile.
    KisColor(const QColor & color, KisAbstractColorSpace * colorSpace, KisProfileSP profile = 0);
    
    // Create a KisColor from a QColor. The QColor is immediately converted to native. The QColor
    // is assumed to have the current monitor profile.    
    KisColor(const QColor & color, Q_UINT8 alpha, KisAbstractColorSpace * colorSpace, KisProfileSP profile = 0);
    
    // Create a KisColor using a native color strategy. The data is copied.
    KisColor(const Q_UINT8 * data, KisAbstractColorSpace * colorSpace, KisProfileSP profile = 0);

    // Create a KisColor by converting src into another colorspace
    KisColor(const KisColor &src, KisAbstractColorSpace * colorSpace, KisProfileSP profile = 0);

    // Copy constructor -- deep copies the colors.
    KisColor(const KisColor & rhs);

    // Effective C++, item 11
    KisColor &operator=(const KisColor &);

    // For easy memcpy'ing etc.
    Q_UINT8 * data() const { return m_data; }
    
    KisAbstractColorSpace * colorSpace() const { return m_colorSpace; }
    
    KisProfileSP profile() const { return m_profile; } 

    // Convert this KisColor to the specified colorspace. If the specified colorspace is the
    // same as the original colorspace, do nothing. Returns the converted KisColor.
    void convertTo(KisAbstractColorSpace * cs, KisProfileSP profile = 0);

    // Replace the existing color data, colorspace and color model with the specified data. 
    void setColor(Q_UINT8 * data, KisAbstractColorSpace * colorSpace = 0, KisProfileSP profile = 0);

    // To save the user the trouble of doing color->colorSpace()->toQColor(color->data(), &c, &a, profile
    void toQColor(QColor *c) const;
    void toQColor(QColor *c, QUANTUM *opacity) const;

    QColor toQColor() const;
    
    void dump() const;
#if 0
    // XXX (bsar): Do we need these?
    void toLab(float * l, float * a, float * b, float * alpha, KisProfileSP profile = 0);
    void fromLab(float l, float a, float b, float alpha, KisProfileSP profile = 0);
#endif    
private:

    Q_UINT8 * m_data;
    
    KisAbstractColorSpace * m_colorSpace;
    KisProfileSP m_profile;
};

#endif
