/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2017, 2020 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
