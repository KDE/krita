/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_rotate_canvas_action.h"

#include <QApplication>
#include <KLocalizedString>

#include "kis_canvas_controller.h"
#include <kis_canvas2.h>
#include "kis_input_manager.h"


KisRotateCanvasAction::KisRotateCanvasAction(KisInputManager* manager)
    : KisAbstractInputAction(manager)
{
    setName(i18n("Rotate Canvas"));
    QHash<QString, int> shortcuts;
    shortcuts.insert(i18n("Toggle Rotate Mode"), RotateToggleShortcut);
    shortcuts.insert(i18n("Rotate Left"), RotateLeftShortcut);
    shortcuts.insert(i18n("Rotate Right"), RotateRightShortcut);
    shortcuts.insert(i18n("Reset Rotation"), RotateResetShortcut);
    setShortcutIndexes(shortcuts);
}

KisRotateCanvasAction::~KisRotateCanvasAction()
{
}

void KisRotateCanvasAction::activate()
{
    QApplication::setOverrideCursor(Qt::OpenHandCursor);
}

void KisRotateCanvasAction::deactivate()
{
    QApplication::restoreOverrideCursor();
}

void KisRotateCanvasAction::begin(int shortcut, QEvent *event)
{
    KisAbstractInputAction::begin(shortcut, event);

    KisCanvasController *canvasController =
        dynamic_cast<KisCanvasController*>(inputManager()->canvas()->canvasController());

    switch(shortcut) {
        case RotateToggleShortcut:
            break;
        case RotateLeftShortcut:
            canvasController->rotateCanvasLeft15();
            break;
        case RotateRightShortcut:
            canvasController->rotateCanvasRight15();
            break;
        case RotateResetShortcut:
            canvasController->resetCanvasTransformations();
            break;
    }
}

void KisRotateCanvasAction::mouseMoved(const QPointF &lastPos, const QPointF &pos)
{
    const KisCoordinatesConverter *converter = inputManager()->canvas()->coordinatesConverter();
    QPointF centerPoint = converter->flakeToWidget(converter->flakeCenterPoint());
    QPointF oldPoint = lastPos - centerPoint;
    QPointF newPoint = pos - centerPoint;

    qreal oldAngle = atan2(oldPoint.y(), oldPoint.x());
    qreal newAngle = atan2(newPoint.y(), newPoint.x());

    float angle = (180 / M_PI) * (newAngle - oldAngle);

    KisCanvasController *canvasController =
        dynamic_cast<KisCanvasController*>(inputManager()->canvas()->canvasController());
    canvasController->rotateCanvas(angle);
}
