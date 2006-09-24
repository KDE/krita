/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.bet
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

#include <KoColorSpaceMaths.h>

// TODO: use more functions from KoIntegersMaths to do the computation

/// This specialization is needed because the default implementation won't work when scaling up

template<>
quint16 KoColorSpaceMaths<quint8,quint16>::scaleToA(quint8 a)
{
    return UINT8_TO_UINT16(a);
}

template<>
quint8 KoColorSpaceMaths<quint8,quint8>::scaleToA(quint8 a)
{
    return a;
}

template<>
quint8 KoColorSpaceMaths<quint16,quint8>::scaleToA(quint16 a)
{
    return UINT16_TO_UINT8(a);
}

template<>
quint16 KoColorSpaceMaths<quint16,quint16>::scaleToA(quint16 a)
{
    return a;
}
