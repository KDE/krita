/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISINTERSTROKEDATAFACTORY_H
#define KISINTERSTROKEDATAFACTORY_H

#include "kritaimage_export.h"

class KisInterstrokeData;

class KRITAIMAGE_EXPORT KisInterstrokeDataFactory
{
public:
    virtual ~KisInterstrokeDataFactory();

    virtual bool isCompatible(KisInterstrokeData *data) = 0;
    virtual KisInterstrokeData* create() = 0;
};

#endif // KISINTERSTROKEDATAFACTORY_H
