/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2017, 2020 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
