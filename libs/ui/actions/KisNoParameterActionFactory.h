/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_NOPARAMETERACTIONFACTORY_H
#define __KIS_NOPARAMETERACTIONFACTORY_H

#include "operations/kis_operation.h"

class KRITAUI_EXPORT KisNoParameterActionFactory : public KisOperation
{
public:
    KisNoParameterActionFactory(const QString &id) : KisOperation(id) {}
    void runFromXML(KisViewManager *view, const KisOperationConfiguration &config) override {
        Q_UNUSED(config);
        run(view);
    }
    virtual void run(KisViewManager *view) = 0;
};

#endif //__KIS_NOPARAMETERACTIONFACTORY_H
