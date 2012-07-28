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

#include "kis_zoom_action.h"

#include <QApplication>

#include <kis_canvas2.h>

#include "kis_input_manager.h"
#include <kis_view2.h>
#include <kis_zoom_manager.h>
#include <KoCanvasController.h>
#include <KoZoomController.h>

class KisZoomAction::Private
{
public:
    Private() : active(false) { }

    bool active;
};

KisZoomAction::KisZoomAction(KisInputManager* manager)
    : KisAbstractInputAction(manager), d(new Private)
{
    setName(i18n("Zoom Canvas"));

    QHash< QString, int > shortcuts;
    shortcuts.insert(i18n("Toggle Zoom Mode"), ZoomToggleShortcut);
    shortcuts.insert(i18n("Zoom In"), ZoomInShortcut);
    shortcuts.insert(i18n("Zoom Out"), ZoomOutShortcut);
    shortcuts.insert(i18n("Reset Zoom to 100%"), ZoomResetShortcut);
    shortcuts.insert(i18n("Fit to Page"), ZoomToPageShortcut);
    shortcuts.insert(i18n("Fit to Width"), ZoomToWidthShortcut);
    setShortcutIndexes(shortcuts);
}

KisZoomAction::~KisZoomAction()
{
    delete d;
}

void KisZoomAction::begin(int shortcut)
{
    switch(shortcut)
    {
        case ZoomToggleShortcut:
            setMousePosition(inputManager()->canvas()->coordinatesConverter()->documentToWidget(inputManager()->mousePosition()));
            QApplication::setOverrideCursor(Qt::OpenHandCursor);
            d->active = true;
            break;
        case ZoomInShortcut: {
            float zoom = inputManager()->canvas()->view()->zoomController()->zoomAction()->effectiveZoom();
            if( zoom >= 10 ) {
                zoom += 1.0;
            } else if (zoom >= 5) {
                zoom += 0.5;
            } else if (zoom >= 2) {
                zoom += 0.2;
            } else {
                zoom += 0.1;
            }
            inputManager()->canvas()->view()->zoomController()->setZoom(KoZoomMode::ZOOM_CONSTANT, zoom);
            break;
        }
        case ZoomOutShortcut: {
            float zoom = inputManager()->canvas()->view()->zoomController()->zoomAction()->effectiveZoom();
            if( zoom >= 10 ) {
                zoom -= 1.0;
            } else if (zoom >= 5) {
                zoom -= 0.5;
            } else if (zoom >= 2) {
                zoom -= 0.2;
            } else {
                zoom -= 0.1;
            }
            inputManager()->canvas()->view()->zoomController()->setZoom(KoZoomMode::ZOOM_CONSTANT, zoom);
            break;
        }
        case ZoomResetShortcut:
            inputManager()->canvas()->view()->zoomController()->setZoom(KoZoomMode::ZOOM_CONSTANT, 1.0);
            break;
        case ZoomToPageShortcut:
            inputManager()->canvas()->view()->zoomController()->setZoom(KoZoomMode::ZOOM_PAGE, 1.0);
            break;
        case ZoomToWidthShortcut:
            inputManager()->canvas()->view()->zoomController()->setZoom(KoZoomMode::ZOOM_WIDTH, 1.0);
            break;
    }
}

void KisZoomAction::end()
{
    d->active = false;
    QApplication::restoreOverrideCursor();
}

void KisZoomAction::inputEvent(QEvent* event)
{
    switch (event->type()) {
        case QEvent::MouseButtonPress:
            setMousePosition(static_cast<QMouseEvent*>(event)->posF());
            break;
        case QEvent::MouseMove: {
            QMouseEvent *mevent = static_cast<QMouseEvent*>(event);
            if(mevent->buttons()) {
                QPointF relMovement = -(mevent->posF() - mousePosition());

                float zoom = inputManager()->canvas()->view()->zoomController()->zoomAction()->effectiveZoom() + relMovement.y() / 100;
                inputManager()->canvas()->view()->zoomController()->setZoom(KoZoomMode::ZOOM_CONSTANT, zoom);

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

bool KisZoomAction::isBlockingAutoRepeat() const
{
    return d->active;
}
