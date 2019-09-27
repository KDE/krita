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
#include "KisToolPaintFactoryBase.h"

#include <kis_action_registry.h>
#include <kis_action.h>

#include <klocalizedstring.h>

KisToolPaintFactoryBase::KisToolPaintFactoryBase(const QString &id)
    : KoToolFactoryBase(id)
{
}

KisToolPaintFactoryBase::~KisToolPaintFactoryBase()
{
}

QList<QAction *> KisToolPaintFactoryBase::createActionsImpl()
{
    KisActionRegistry *actionRegistry = KisActionRegistry::instance();
    QList<QAction *> actions;

    KisAction *increaseBrushSize = new KisAction(i18n("Increase Brush Size"));
    increaseBrushSize->setObjectName("increase_brush_size");
    increaseBrushSize->setShortcut(Qt::Key_BracketRight);
    actionRegistry->propertizeAction("increase_brush_size", increaseBrushSize);

    actions << increaseBrushSize;

    KisAction *decreaseBrushSize = new KisAction(i18n("Decrease Brush Size"));
    decreaseBrushSize->setShortcut(Qt::Key_BracketLeft);
    decreaseBrushSize->setObjectName("decrease_brush_size");
    actionRegistry->propertizeAction("decrease_brush_size", decreaseBrushSize);

    actions << decreaseBrushSize;

    return actions;
}
