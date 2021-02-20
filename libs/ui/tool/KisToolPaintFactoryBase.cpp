/*
 *  SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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
