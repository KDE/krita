/*
 *  SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KISSELECTIONTOOLFACTORYBASE_H
#define KISSELECTIONTOOLFACTORYBASE_H

#include "KisToolPaintFactoryBase.h"

#include "kritaui_export.h"

class KRITAUI_EXPORT KisSelectionToolFactoryBase : public KisToolPaintFactoryBase
{
public:
    explicit KisSelectionToolFactoryBase(const QString &id);
    ~KisSelectionToolFactoryBase() override;
protected:
    QList<QAction *> createActionsImpl() override;
};

class KRITAUI_EXPORT KisToolPolyLineFactoryBase : public KisToolPaintFactoryBase
{
public:
    explicit KisToolPolyLineFactoryBase(const QString &id);
    ~KisToolPolyLineFactoryBase() override;
protected:
    QList<QAction *> createActionsImpl() override;
};


#endif 
