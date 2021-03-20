/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_OPERATION_REGISTRY_H
#define __KIS_OPERATION_REGISTRY_H

#include <kritaui_export.h>
#include <KoGenericRegistry.h>
#include "kis_operation.h"


class KRITAUI_EXPORT KisOperationRegistry : public KoGenericRegistry<KisOperation*>
{
public:
    KisOperationRegistry();
    ~KisOperationRegistry() override;
    static KisOperationRegistry* instance();
};

#endif /* __KIS_OPERATION_REGISTRY_H */
