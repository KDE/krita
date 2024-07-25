/*
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_SUPPORTED_ARCHITECTURES_H
#define KIS_SUPPORTED_ARCHITECTURES_H

#include "kritamultiarch_export.h"

#include <QString>

class KRITAMULTIARCH_EXPORT KisSupportedArchitectures
{
public:
    static QString baseArchName();

    static QString bestArchName();

    static unsigned int bestArch();

    static QString supportedInstructionSets();
};

#endif // KIS_SUPPORTED_ARCHITECTURES_H
