/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.bet
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

#ifndef _KO_COLOR_SPACE_CONSTANTS_H_
#define _KO_COLOR_SPACE_CONSTANTS_H_

#include <climits>
#include <QtGlobal>

// TODO: find a better place or way to define those stuff
const quint8 OPACITY_TRANSPARENT_U8 = 0;
const quint8 OPACITY_OPAQUE_U8 = UCHAR_MAX;
const qreal OPACITY_TRANSPARENT_F = 0.0;
const qreal OPACITY_OPAQUE_F = 1.0;

#endif
