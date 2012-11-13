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

#include <KoCanvasControllerWidget.h>
#include <KoZoomController.h>
#include <kis_view2.h>
#include "kis_input_manager.h"


class KisZoomAction::Private
{
public:
    Private(KisZoomAction *qq) : q(qq) {}
    KisZoomAction *q;

    void zoomTo(bool zoomIn, QEvent *event);
};

void KisZoomAction::Private::zoomTo(bool zoomIn, QEvent *event)
{
    KoZoomAction *zoomAction = q->inputManager()->canvas()->view()->zoomController()->zoomAction();

    if (event) {
        QPoint pos;
        QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event);
        if (mouseEvent) {
            pos = mouseEvent->pos();
        } else {
            QWheelEvent *wheelEvent = dynamic_cast<QWheelEvent*>(event);
            if (wheelEvent) {
                pos = wheelEvent->pos();
            } else {
                qWarning() << "Unhandled type of event";
            }
        }

        float oldZoom = zoomAction->effectiveZoom();
        float newZoom = zoomIn ?
            zoomAction->nextZoomLevel() : zoomAction->prevZoomLevel();

        KoCanvasControllerWidget *controller =
            dynamic_cast<KoCanvasControllerWidget*>(
                q->inputManager()->canvas()->canvasController());

        controller->zoomRelativeToPoint(pos, newZoom / oldZoom);
    } else {
        if (zoomIn) {
            zoomAction->zoomIn();
        } else {
            zoomAction->zoomOut();
        }
    }
}

KisZoomAction::KisZoomAction(KisInputManager* manager)
    : KisAbstractInputAction(manager), d(new Private(this))
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

void KisZoomAction::activate()
{
    QApplication::setOverrideCursor(Qt::OpenHandCursor);
}

void KisZoomAction::deactivate()
{
    QApplication::restoreOverrideCursor();
}

void KisZoomAction::begin(int shortcut, QEvent *event)
{
    KisAbstractInputAction::begin(shortcut, event);

    switch(shortcut) {
        case ZoomToggleShortcut:
            break;
        case ZoomInShortcut:
            d->zoomTo(true, event);
            break;
        case ZoomOutShortcut:
            d->zoomTo(false, event);
            break;
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

void KisZoomAction::mouseMoved(const QPointF &lastPos, const QPointF &pos)
{
    QPointF relMovement = -(pos - lastPos);

    float zoom = inputManager()->canvas()->view()->zoomController()->zoomAction()->effectiveZoom() + relMovement.y() / 100;
    inputManager()->canvas()->view()->zoomController()->setZoom(KoZoomMode::ZOOM_CONSTANT, zoom);
}
