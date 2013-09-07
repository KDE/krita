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

#include "kis_cursor.h"
#include <KoCanvasControllerWidget.h>
#include <KoZoomController.h>
#include <kis_view2.h>
#include "kis_input_manager.h"


class KisZoomAction::Private
{
public:
    Private(KisZoomAction *qq) : q(qq), distance(0) {}
    KisZoomAction *q;
    int distance;
    Shortcuts mode;

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

KisZoomAction::KisZoomAction()
    : d(new Private(this))
{
    setName(i18n("Zoom Canvas"));
    setDescription(i18n("The <i>Zoom Canvas</i> action zooms the canvas."));

    QHash< QString, int > shortcuts;
    shortcuts.insert(i18n("Toggle Zoom Mode"), ZoomToggleShortcut);
    shortcuts.insert(i18n("Toggle Discrete Zoom Mode"), DiscreteZoomToggleShortcut);
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

int KisZoomAction::priority() const
{
    return 4;
}

void KisZoomAction::activate()
{
    QApplication::setOverrideCursor(KisCursor::zoomCursor());
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
            d->mode = (Shortcuts)shortcut;
            break;
        case DiscreteZoomToggleShortcut:
            d->mode = (Shortcuts)shortcut;
            d->distance = 0;
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
    QPointF diff = -(pos - lastPos);

    const int stepCont = 100;
    const int stepDisc = 20;

    if (d->mode == ZoomToggleShortcut) {
        float coeff = 1.0 + qreal(diff.y()) / stepCont;
        float zoom = coeff * inputManager()->canvas()->view()->zoomController()->zoomAction()->effectiveZoom();
        inputManager()->canvas()->view()->zoomController()->setZoom(KoZoomMode::ZOOM_CONSTANT, zoom);
    } else if (d->mode == DiscreteZoomToggleShortcut) {
        d->distance += diff.y();
        bool zoomIn = d->distance > 0;
        while (qAbs(d->distance) > stepDisc) {
            d->zoomTo(zoomIn, 0);
            d->distance += zoomIn ? -stepDisc : stepDisc;
        }
    }
}
