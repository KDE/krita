/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

#include <klocalizedstring.h>


const KoID AlphaColorModelID("A", ki18n("Alpha mask"));
const KoID RGBAColorModelID("RGBA", ki18n("RGB/Alpha"));
const KoID XYZAColorModelID("XYZA", ki18n("XYZ/Alpha"));
const KoID LABAColorModelID("LABA", ki18n("L*a*b*/Alpha"));
const KoID CMYKAColorModelID("CMYKA", ki18n("CMYK/Alpha"));
const KoID GrayAColorModelID("GRAYA", ki18n("Grayscale/Alpha"));
const KoID GrayColorModelID("GRAY", ki18n("Grayscale (without transparency)"));
const KoID YCbCrAColorModelID("YCbCrA", ki18n("YCbCr/Alpha"));

const KoID Integer8BitsColorDepthID("U8", ki18n("8-bit integer/channel"));
const KoID Integer16BitsColorDepthID("U16", ki18n("16-bit integer/channel"));
const KoID Float16BitsColorDepthID("F16", ki18n("16-bit float/channel"));
const KoID Float32BitsColorDepthID("F32", ki18n("32-bit float/channel"));
const KoID Float64BitsColorDepthID("F64", ki18n("64-bit float/channel"));
