/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_delegated_tool_policies.h"

#include <QWidget>


#include <KoCanvasBase.h>
#include <KoShapeManager.h>
#include <KoSelection.h>


void DeselectShapesActivationPolicy::onActivate(KoCanvasBase *canvas)
{
    canvas->shapeManager()->selection()->deselectAll();
    canvas->canvasWidget()->update();
}

