/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoColorModelStandardIds.h"

#include <klocale.h>


extern const KoID AlphaColorModelID("A", i18n("Alpha"));
extern const KoID RGBAColorModelID("RGBA", i18n("Red Green Blue"));
extern const KoID XYZAColorModelID("XYZA", i18n("XYZ"));
extern const KoID LABAColorModelID("LABA", i18n("L a* b*"));
extern const KoID CMYKAColorModelID("CMYKA", i18n("Cyan Magenta Yellow Black"));
extern const KoID GrayAColorModelID("GRAYA", i18n("Grayscale"));
extern const KoID GrayColorModelID("GRAY", i18n("Grayscale (without transparency)"));
extern const KoID YCbCrAColorModelID("YCbCr", i18n("YCbCr"));

extern const KoID Integer8BitsColorDepthID("U8", i18n("8 Bits"));
extern const KoID Integer16BitsColorDepthID("U16", i18n("16 Bits"));
extern const KoID Float16BitsColorDepthID("F16", i18n("16 Bits Float (HDR)"));
extern const KoID Float32BitsColorDepthID("F32", i18n("32 Bits Float (HDR)"));
