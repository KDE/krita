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

#include <kis_debug.h>
#include <QMouseEvent>
#include <QApplication>
#include <QGesture>

#include <klocalizedstring.h>

#include <KoCanvasController.h>

#include <kis_canvas2.h>

#include "kis_input_manager.h"

class KisPanAction::Private
{
public:
    Private() : panDistance(10) { }

        QPointF averagePoint( QTouchEvent* event );

    const int panDistance;

    QPointF lastPosition;
    QPointF originalPreferredCenter;
};

KisPanAction::KisPanAction()
    : KisAbstractInputAction("Pan Canvas")
    , d(new Private)
{
    setName(i18n("Pan Canvas"));
    setDescription(i18n("The <i>Pan Canvas</i> action pans the canvas."));

    QHash<QString, int> shortcuts;
    shortcuts.insert(i18n("Pan Mode"), PanModeShortcut);
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

int KisPanAction::priority() const
{
    return 5;
}

void KisPanAction::activate(int shortcut)
{
    Q_UNUSED(shortcut);
    QApplication::setOverrideCursor(Qt::OpenHandCursor);
}

void KisPanAction::deactivate(int shortcut)
{
    Q_UNUSED(shortcut);
    QApplication::restoreOverrideCursor();
}

void KisPanAction::begin(int shortcut, QEvent *event)
{
    KisAbstractInputAction::begin(shortcut, event);

    bool overrideCursor = true;

    switch (shortcut) {
        case PanModeShortcut: {
            QTouchEvent *tevent = dynamic_cast<QTouchEvent*>(event);
            if (tevent) {
                d->lastPosition = d->averagePoint(tevent);
                break;
            }

            // Some QT wheel events are actually be touch pad pan events. From the QT docs:
            // "Wheel events are generated for both mouse wheels and trackpad scroll gestures."
            QWheelEvent *wheelEvent = dynamic_cast<QWheelEvent*>(event);
            if (wheelEvent) {
                inputManager()->canvas()->canvasController()->pan(-wheelEvent->pixelDelta());
                overrideCursor = false;
                break;
            }

            d->originalPreferredCenter = inputManager()->canvas()->canvasController()->preferredCenter();

            break;
        }
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

    if (overrideCursor) {
        QApplication::setOverrideCursor(Qt::ClosedHandCursor);
    }
}

void KisPanAction::end(QEvent *event)
{
    QApplication::restoreOverrideCursor();
    KisAbstractInputAction::end(event);
}

void KisPanAction::inputEvent(QEvent *event)
{
    switch (event->type()) {
        case QEvent::Gesture: {
            QGestureEvent *gevent = static_cast<QGestureEvent*>(event);
            if (gevent->activeGestures().at(0)->gestureType() == Qt::PanGesture) {
                QPanGesture *pan = static_cast<QPanGesture*>(gevent->activeGestures().at(0));
                inputManager()->canvas()->canvasController()->pan(-pan->delta().toPoint() * 0.2);
            }
            return;
        }
        case QEvent::TouchUpdate: {
            QTouchEvent *tevent = static_cast<QTouchEvent*>(event);
            QPointF newPos = d->averagePoint(tevent);
            QPointF delta = newPos - d->lastPosition;
            // If this is enormously large, then we are likely in the process of ending the gesture,
            // with fingers being lifted one by one from the perspective of our very speedy operations,
            // and as such, ignore those big jumps.
            if(delta.manhattanLength() < 50) {
                inputManager()->canvas()->canvasController()->pan(-delta.toPoint());
                d->lastPosition = newPos;
            }
            return;
        }
        default:
            break;
    }
    KisAbstractInputAction::inputEvent(event);
}

void KisPanAction::cursorMovedAbsolute(const QPointF &startPos, const QPointF &pos)
{
    inputManager()->canvas()->canvasController()->setPreferredCenter(-pos + startPos + d->originalPreferredCenter);
}

QPointF KisPanAction::Private::averagePoint( QTouchEvent* event )
{
    QPointF result;
    int count = 0;

    Q_FOREACH ( QTouchEvent::TouchPoint point, event->touchPoints() ) {
        if( point.state() != Qt::TouchPointReleased ) {
            result += point.screenPos();
            count++;
        }
    }

    if( count > 0 ) {
        return result / count;
    } else {
        return QPointF();
    }
}

bool KisPanAction::isShortcutRequired(int shortcut) const
{
    return shortcut == PanModeShortcut;
}

KisInputActionGroup KisPanAction::inputActionGroup(int shortcut) const
{
    Q_UNUSED(shortcut);
    return ViewTransformActionGroup;
}
