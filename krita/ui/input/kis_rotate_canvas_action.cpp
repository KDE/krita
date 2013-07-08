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
#include <klocalizedstring.h>

#include "kis_cursor.h"
#include "kis_canvas_controller.h"
#include <kis_canvas2.h>
#include "kis_input_manager.h"

class KisRotateCanvasAction::Private
{
public:
    Private() : angleDrift(0) {}

    Shortcut mode;
    qreal angleDrift;
};


KisRotateCanvasAction::KisRotateCanvasAction()
    : d(new Private())
{
    setName(i18n("Rotate Canvas"));
    setDescription(i18n("The <i>Rotate Canvas</i> action rotates the canvas."));

    QHash<QString, int> shortcuts;
    shortcuts.insert(i18n("Toggle Rotate Mode"), RotateToggleShortcut);
    shortcuts.insert(i18n("Toggle Discrete Rotate Mode"), DiscreteRotateToggleShortcut);
    shortcuts.insert(i18n("Rotate Left"), RotateLeftShortcut);
    shortcuts.insert(i18n("Rotate Right"), RotateRightShortcut);
    shortcuts.insert(i18n("Reset Rotation"), RotateResetShortcut);
    setShortcutIndexes(shortcuts);
}

KisRotateCanvasAction::~KisRotateCanvasAction()
{
    delete d;
}

void KisRotateCanvasAction::activate()
{
    QApplication::setOverrideCursor(KisCursor::rotateCursor());
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
            d->mode = (Shortcut)shortcut;
            break;
        case DiscreteRotateToggleShortcut:
            d->mode = (Shortcut)shortcut;
            d->angleDrift = 0;
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

    qreal angle = (180 / M_PI) * (newAngle - oldAngle);

    if (d->mode == DiscreteRotateToggleShortcut) {
        const qreal angleStep = 15;
        qreal initialAngle = inputManager()->canvas()->rotationAngle();
        qreal roundedAngle = qRound((initialAngle + angle + d->angleDrift) / angleStep) * angleStep - initialAngle;
        d->angleDrift += angle - roundedAngle;
        angle = roundedAngle;
    }

    KisCanvasController *canvasController =
        dynamic_cast<KisCanvasController*>(inputManager()->canvas()->canvasController());
    canvasController->rotateCanvas(angle);
}
