/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISTRANSACTIONWRAPPERFACTORY_H
#define KISTRANSACTIONWRAPPERFACTORY_H

#include "kritaimage_export.h"
#include "kis_types.h"

class KUndo2Command;

class KRITAIMAGE_EXPORT KisTransactionWrapperFactory
{
public:
    virtual ~KisTransactionWrapperFactory();

    virtual KUndo2Command* createBeginTransactionCommand(KisPaintDeviceSP device) = 0;
    virtual KUndo2Command* createEndTransactionCommand() = 0;
};

#endif // KISTRANSACTIONWRAPPERFACTORY_H
