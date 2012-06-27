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

#include <kis_canvas2.h>
#include <kis_image.h>

#include "kis_input_manager.h"

class KisRotateCanvasAction::Private
{
public:
    Private() : active(false) { }

    bool active;
};

KisRotateCanvasAction::KisRotateCanvasAction(KisInputManager* manager)
    : KisAbstractInputAction(manager), d(new Private)
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
    delete d;
}

void KisRotateCanvasAction::begin(int shortcut)
{
    switch(shortcut) {
        case RotateToggleShortcut:
            setMousePosition(inputManager()->canvas()->coordinatesConverter()->documentToWidget(inputManager()->mousePosition()));
            QApplication::setOverrideCursor(Qt::OpenHandCursor);
            d->active = true;
            break;
        case RotateLeftShortcut:
            inputManager()->canvas()->rotateCanvasLeft15();
            break;
        case RotateRightShortcut:
            inputManager()->canvas()->rotateCanvasRight15();
            break;
        case RotateResetShortcut:
            inputManager()->canvas()->resetCanvasTransformations();
            break;
    }
}

void KisRotateCanvasAction::end()
{
    d->active = false;
    QApplication::restoreOverrideCursor();
}

void KisRotateCanvasAction::inputEvent(QEvent* event)
{
    switch (event->type()) {
        case QEvent::MouseButtonPress: {
            setMousePosition(static_cast<QMouseEvent*>(event)->posF());
            break;
        }
        case QEvent::MouseMove: {
            QMouseEvent *mevent = static_cast<QMouseEvent*>(event);
            if (mevent->buttons()) {
                const KisCoordinatesConverter *converter = inputManager()->canvas()->coordinatesConverter();
                QPointF centerPoint = converter->flakeToWidget(converter->flakeCenterPoint());
                QPointF oldPoint = mousePosition() - centerPoint;
                QPointF newPoint = mevent->posF() - centerPoint;

                qreal oldAngle = atan2(oldPoint.y(), oldPoint.x());
                qreal newAngle = atan2(newPoint.y(), newPoint.x());

                float angle = (180 / M_PI) * (newAngle - oldAngle);

                inputManager()->canvas()->rotateCanvas(angle);

                setMousePosition(mevent->posF());
                QApplication::changeOverrideCursor(Qt::ClosedHandCursor);
            } else {
                QApplication::changeOverrideCursor(Qt::OpenHandCursor);
            }
        }
        default:
            break;
    }
}

bool KisRotateCanvasAction::isBlockingAutoRepeat() const
{
    return d->active;
}
