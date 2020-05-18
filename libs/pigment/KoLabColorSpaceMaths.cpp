/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2017,2020 L. E. Segovia <amy@amyspark.me>
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

#include <KoLabColorSpaceMaths.h>

#include <cfloat>

#include <QtGlobal>

#ifdef HAVE_OPENEXR
const half KoLabColorSpaceMathsTraits<half>::zeroValueL = 0.0;
const half KoLabColorSpaceMathsTraits<half>::unitValueL = 100.0;
const half KoLabColorSpaceMathsTraits<half>::halfValueL = 50.0;
const half KoLabColorSpaceMathsTraits<half>::zeroValueAB = -128.0;
const half KoLabColorSpaceMathsTraits<half>::unitValueAB = +127.0;
const half KoLabColorSpaceMathsTraits<half>::halfValueAB = 0.0;
#endif

const float KoLabColorSpaceMathsTraits<float>::zeroValueL = 0.0;
const float KoLabColorSpaceMathsTraits<float>::unitValueL = 100.0;
const float KoLabColorSpaceMathsTraits<float>::halfValueL = 50.0;
const float KoLabColorSpaceMathsTraits<float>::zeroValueAB = -128.0;
const float KoLabColorSpaceMathsTraits<float>::unitValueAB = +127.0;
const float KoLabColorSpaceMathsTraits<float>::halfValueAB = 0.0;

const double KoLabColorSpaceMathsTraits<double>::zeroValueL = 0.0;
const double KoLabColorSpaceMathsTraits<double>::unitValueL = 100.0;
const double KoLabColorSpaceMathsTraits<double>::halfValueL = 50.0;
const double KoLabColorSpaceMathsTraits<double>::zeroValueAB = -128.0;
const double KoLabColorSpaceMathsTraits<double>::unitValueAB = +127.0;
const double KoLabColorSpaceMathsTraits<double>::halfValueAB = 0.0;
