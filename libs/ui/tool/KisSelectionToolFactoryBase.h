/*
 *  Copyright (c) 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

class KRITAUI_EXPORT KisToolPolyLineFactoryBase : public KisSelectionToolFactoryBase
{
public:
    explicit KisToolPolyLineFactoryBase(const QString &id);
    ~KisToolPolyLineFactoryBase() override;
protected:
    QList<QAction *> createActionsImpl() override;
};


#endif 
