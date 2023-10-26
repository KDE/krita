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

    KisAction *increaseBrushSize = new KisAction(i18n("Increase Brush Size"), this);
    increaseBrushSize->setObjectName("increase_brush_size");
    increaseBrushSize->setShortcut(Qt::Key_BracketRight);
    actionRegistry->propertizeAction("increase_brush_size", increaseBrushSize);

    actions << increaseBrushSize;

    KisAction *decreaseBrushSize = new KisAction(i18n("Decrease Brush Size"), this);
    decreaseBrushSize->setShortcut(Qt::Key_BracketLeft);
    decreaseBrushSize->setObjectName("decrease_brush_size");
    actionRegistry->propertizeAction("decrease_brush_size", decreaseBrushSize);

    actions << decreaseBrushSize;

    KisAction *rotateBrushTipClockwise = new KisAction(i18n("Rotate brush tip clockwise"), this);
    rotateBrushTipClockwise->setObjectName("rotate_brush_tip_clockwise");
    actionRegistry->propertizeAction("rotate_brush_tip_clockwise", rotateBrushTipClockwise);

    actions << rotateBrushTipClockwise;

    KisAction *rotateBrushTipClockwisePrecise = new KisAction(i18n("Rotate brush tip clockwise (precise)"), this);
    rotateBrushTipClockwisePrecise->setObjectName("rotate_brush_tip_clockwise_precise");
    actionRegistry->propertizeAction("rotate_brush_tip_clockwise_precise", rotateBrushTipClockwisePrecise);

    actions << rotateBrushTipClockwisePrecise;

    KisAction *rotateBrushTipCounterClockwise = new KisAction(i18n("Rotate brush tip counter-clockwise"), this);
    rotateBrushTipCounterClockwise->setObjectName("rotate_brush_tip_counter_clockwise");
    actionRegistry->propertizeAction("rotate_brush_tip_counter_clockwise", rotateBrushTipCounterClockwise);

    actions << rotateBrushTipCounterClockwise;

    KisAction *rotateBrushTipCounterClockwisePrecise = new KisAction(i18n("Rotate brush tip counter-clockwise (precise)"), this);
    rotateBrushTipCounterClockwisePrecise->setObjectName("rotate_brush_tip_counter_clockwise_precise");
    actionRegistry->propertizeAction("rotate_brush_tip_counter_clockwise_precise", rotateBrushTipCounterClockwisePrecise);

    actions << rotateBrushTipCounterClockwisePrecise;

    return actions;
}
