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

#include <KoCmykColorSpaceMaths.h>

#include <cfloat>

#include <QtGlobal>

#ifdef HAVE_OPENEXR
const half KoCmykColorSpaceMathsTraits<half>::zeroValueCMYK = 0.0;
const half KoCmykColorSpaceMathsTraits<half>::unitValueCMYK = 100.0;
const half KoCmykColorSpaceMathsTraits<half>::halfValueCMYK = 50.0;
#endif

const float KoCmykColorSpaceMathsTraits<float>::zeroValueCMYK = 0.0;
const float KoCmykColorSpaceMathsTraits<float>::unitValueCMYK = 100.0;
const float KoCmykColorSpaceMathsTraits<float>::halfValueCMYK = 50.0;

const double KoCmykColorSpaceMathsTraits<double>::zeroValueCMYK = 0.0;
const double KoCmykColorSpaceMathsTraits<double>::unitValueCMYK = 100.0;
const double KoCmykColorSpaceMathsTraits<double>::halfValueCMYK = 50.0;
