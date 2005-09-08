/* This file is part of the KDE project
 *  Copyright (C) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef _KIS_COLOR_IFACE_H
#define _KIS_COLOR_IFACE_H

#include <dcopref.h>
#include <dcopobj.h>

#include <qstring.h>

class KisColor;

class KisColorIface : virtual public DCOPObject
{
	K_DCOP
public:

	KisColorIface( KisColor * parent );

k_dcop:
 
    /**
     * Returns a byte array that represents one pixel of the current
     * color. This can be interpreted by the correct color strategy
     *
     * @returns the pixel data
     */
    QByteArray data();
    
    /**
     * Returns the native color model of this color
     *
     * @returns the colormodel
     */
    DCOPRef colorSpace();
    
    /**
     * @returns the icc profile of tthis color
     */
    DCOPref profile();
    
    /**
     * Convert this color to the equivalent color in the other colorspace
     *
     * @param cs the new colorspace
     * @param profile the new profile. This may be 0
     */
    void convertTo(DCOPRef cs, DCOPRef profile);

    /**
     * Replace the existing colordata with the specified data in the
     * specified colorspace; if the data and the colorspace don't fit...
     *
     * @param data the bytes that make up the color
     * @param cs the new colorspace
     * @param profile the new profile. This may be 0
     */
    void setColor(QByteArray data, DCOPRef cs, DCOPRef profile);

    /**
     * Create a QColor from the current color
     */
    QColor toQColor();

    /**
     * Retrieve the opacitiy level of the current color, normalized
     * to a byte.
     */
    Q_UINT8 opacity();
     

private:

	KisColor *m_parent;
};

#endif
