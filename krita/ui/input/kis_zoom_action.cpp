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

#include <klocalizedstring.h>

#include <KoCanvasControllerWidget.h>
#include <KoZoomController.h>

#include <kis_canvas2.h>
#include <kis_canvas_controller.h>
#include "kis_cursor.h"
#include "KisViewManager.h"
#include "kis_input_manager.h"
#include "kis_config.h"


class KisZoomAction::Private
{
public:
    Private(KisZoomAction *qq) : q(qq), distance(0), lastDistance(0.f) {}

    QPointF centerPoint(QTouchEvent* event);

    KisZoomAction *q;
    int distance;
    Shortcuts mode;

    QPointF lastPosition;
    float lastDistance;

    void zoomTo(bool zoomIn, QEvent *event);
};

QPointF KisZoomAction::Private::centerPoint(QTouchEvent* event)
{
    QPointF result;
    int count = 0;

    Q_FOREACH (QTouchEvent::TouchPoint point, event->touchPoints()) {
        if (point.state() != Qt::TouchPointReleased) {
            result += point.screenPos();
            count++;
        }
    }

    if (count > 0) {
        return result / count;
    } else {
        return QPointF();
    }
}

void KisZoomAction::Private::zoomTo(bool zoomIn, QEvent *event)
{
    KoZoomAction *zoomAction = q->inputManager()->canvas()->viewManager()->zoomController()->zoomAction();

    if (event) {
        QPoint pos = q->eventPos(event);

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
    : KisAbstractInputAction("Zoom Canvas")
    , d(new Private(this))
{
    setName(i18n("Zoom Canvas"));
    setDescription(i18n("The <i>Zoom Canvas</i> action zooms the canvas."));

    QHash< QString, int > shortcuts;
    shortcuts.insert(i18n("Zoom Mode"), ZoomModeShortcut);
    shortcuts.insert(i18n("Discrete Zoom Mode"), DiscreteZoomModeShortcut);
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

void KisZoomAction::activate(int shortcut)
{
    if (shortcut == DiscreteZoomModeShortcut) {
        QApplication::setOverrideCursor(KisCursor::zoomDiscreteCursor());
    } else /* if (shortcut == SmoothZoomModeShortcut) */ {
        QApplication::setOverrideCursor(KisCursor::zoomSmoothCursor());
    }
}

void KisZoomAction::deactivate(int shortcut)
{
    Q_UNUSED(shortcut);
    QApplication::restoreOverrideCursor();
}

void KisZoomAction::begin(int shortcut, QEvent *event)
{
    KisAbstractInputAction::begin(shortcut, event);

    d->lastDistance = 0.f;

    switch(shortcut) {
        case ZoomModeShortcut: {
            d->mode = (Shortcuts)shortcut;
            QTouchEvent *tevent = dynamic_cast<QTouchEvent*>(event);
            if(tevent)
                d->lastPosition = d->centerPoint(tevent);
            break;
        }
        case DiscreteZoomModeShortcut:
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
            inputManager()->canvas()->viewManager()->zoomController()->setZoom(KoZoomMode::ZOOM_CONSTANT, 1.0);
            break;
        case ZoomToPageShortcut:
            inputManager()->canvas()->viewManager()->zoomController()->setZoom(KoZoomMode::ZOOM_PAGE, 1.0);
            break;
        case ZoomToWidthShortcut:
            inputManager()->canvas()->viewManager()->zoomController()->setZoom(KoZoomMode::ZOOM_WIDTH, 1.0);
            break;
    }
}

void KisZoomAction::inputEvent( QEvent* event )
{
    switch (event->type()) {
        case QEvent::TouchUpdate: {
            QTouchEvent *tevent = static_cast<QTouchEvent*>(event);
            QPointF center = d->centerPoint(tevent);

            int count = 0;
            float dist = 0.0f;
            Q_FOREACH (const QTouchEvent::TouchPoint &point, tevent->touchPoints()) {
                if (point.state() != Qt::TouchPointReleased) {
                    count++;

                    dist += (point.screenPos() - center).manhattanLength();
                }
            }

            dist /= count;
            float delta = qFuzzyCompare(1.0f, 1.0f + d->lastDistance) ? 1.f : dist / d->lastDistance;

            if(qAbs(delta) > 0.1f) {
                qreal zoom = inputManager()->canvas()->viewManager()->zoomController()->zoomAction()->effectiveZoom();
                Q_UNUSED(zoom);
                static_cast<KisCanvasController*>(inputManager()->canvas()->canvasController())->zoomRelativeToPoint(center.toPoint(), delta);
                d->lastDistance = dist;
                // Also do panning here, as doing it later requires a further check for validity
                QPointF moveDelta = center - d->lastPosition;
                inputManager()->canvas()->canvasController()->pan(-moveDelta.toPoint());
                d->lastPosition = center;
            }
            return;  // Don't try to update the cursor during a pinch-zoom
        }
        default:
            break;
    }
    KisAbstractInputAction::inputEvent(event);
}

void KisZoomAction::cursorMoved(const QPointF &lastPos, const QPointF &pos)
{
    QPointF diff = -(pos - lastPos);

    const int stepCont = 100;
    const int stepDisc = 20;

    if (d->mode == ZoomModeShortcut) {
        KisConfig cfg;
        float coeff;
        if (cfg.readEntry<bool>("InvertMiddleClickZoom", false)) {
            coeff = 1.0 - qreal(diff.y()) / stepCont;
        }
        else {
            coeff = 1.0 + qreal(diff.y()) / stepCont;
        }
        float zoom = coeff * inputManager()->canvas()->viewManager()->zoomController()->zoomAction()->effectiveZoom();
        inputManager()->canvas()->viewManager()->zoomController()->setZoom(KoZoomMode::ZOOM_CONSTANT, zoom);
    } else if (d->mode == DiscreteZoomModeShortcut) {
        d->distance += diff.y();
        bool zoomIn = d->distance > 0;
        while (qAbs(d->distance) > stepDisc) {
            d->zoomTo(zoomIn, 0);
            d->distance += zoomIn ? -stepDisc : stepDisc;
        }
    }
}

bool KisZoomAction::isShortcutRequired(int shortcut) const
{
    return shortcut == ZoomModeShortcut;
}

