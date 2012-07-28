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

#include "kis_pan_action.h"

#include <QDebug>
#include <QMouseEvent>
#include <QApplication>

#include <KLocalizedString>

#include <KoCanvasController.h>

#include <kis_canvas2.h>

#include "kis_input_manager.h"

class KisPanAction::Private
{
public:
    Private() : active(false), panDistance(10) { }

    const int panDistance;
    bool active;
};

KisPanAction::KisPanAction(KisInputManager *manager)
    : KisAbstractInputAction(manager), d(new Private)
{
    setName(i18n("Pan Canvas"));

    QHash<QString, int> shortcuts;
    shortcuts.insert(i18n("Toggle Pan Mode"), PanToggleShortcut);
    shortcuts.insert(i18n("Pan Left"), PanLeftShortcut);
    shortcuts.insert(i18n("Pan Right"), PanRightShortcut);
    shortcuts.insert(i18n("Pan Up"), PanUpShortcut);
    shortcuts.insert(i18n("Pan Down"), PanDownShortcut);
    setShortcutIndexes(shortcuts);
}

KisPanAction::~KisPanAction()
{
    delete d;
}

void KisPanAction::begin(int shortcut)
{
    switch (shortcut) {
        case PanToggleShortcut:
            setMousePosition(inputManager()->canvas()->coordinatesConverter()->documentToWidget(inputManager()->mousePosition()));
            QApplication::setOverrideCursor(Qt::OpenHandCursor);
            d->active = true;
            break;
        case PanLeftShortcut:
            inputManager()->canvas()->canvasController()->pan(QPoint(d->panDistance, 0));
            break;
        case PanRightShortcut:
            inputManager()->canvas()->canvasController()->pan(QPoint(-d->panDistance, 0));
            break;
        case PanUpShortcut:
            inputManager()->canvas()->canvasController()->pan(QPoint(0, d->panDistance));
            break;
        case PanDownShortcut:
            inputManager()->canvas()->canvasController()->pan(QPoint(0, -d->panDistance));
            break;
    }
}

void KisPanAction::end()
{
    d->active = false;
    QApplication::restoreOverrideCursor();
}

void KisPanAction::inputEvent(QEvent *event)
{
    switch (event->type()) {
        case QEvent::MouseButtonPress: {
            setMousePosition(static_cast<QMouseEvent*>(event)->posF());
            break;
        }
        case QEvent::MouseMove: {
            QMouseEvent *mevent = static_cast<QMouseEvent*>(event);
            if (mevent->buttons()) {
                QPointF relMovement = -(mevent->posF() - mousePosition());
                inputManager()->canvas()->canvasController()->pan(relMovement.toPoint());
                setMousePosition(mevent->posF());
                QApplication::changeOverrideCursor(Qt::ClosedHandCursor);
            } else {
                QApplication::changeOverrideCursor(Qt::OpenHandCursor);
            }
            break;
        }
        default:
            break;
    }
}

bool KisPanAction::isBlockingAutoRepeat() const
{
    return d->active;
}
