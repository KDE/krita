/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

#include "KisCanvasNavigationActionStrategyTouch.h"
#include "KisCanvasNavigationActionStrategyNativeGesture.h"


class KisPanAction::Private
{
public:
    Private() : panDistance(10) { }

    const int panDistance;

    QPointF originalPreferredCenter;

    std::unique_ptr<KisCanvasNavigationActionStrategy> actionStrategy;
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

    /**
     * Firstly, try to handle native gestures and touch events
     */
    if (event
        && (event->type() == QEvent::NativeGesture || event->type() == QEvent::TouchBegin
            || event->type() == QEvent::TouchUpdate)
        && shortcut == PanModeShortcut) {

        using Flag = KisCanvasNavigationActionStrategyNativeGesture::Flag;
        using Flags = KisCanvasNavigationActionStrategyNativeGesture::Flags;

        Flags flags;
        flags.setFlag(Flag::PanEnabled);

        if (event->type() == QEvent::NativeGesture) {
            d->actionStrategy.reset(new KisCanvasNavigationActionStrategyNativeGesture(flags, eventPosF(event), inputManager()->canvas()));
        } else {
            const QTouchEvent *tevent = static_cast<const QTouchEvent*>(event);
            d->actionStrategy.reset(new KisCanvasNavigationActionStrategyTouch(flags, tevent, inputManager()->canvas()));
        }

        // native gestures don't have cursor tracking by the OS, so they shouldn't show any cursor
        QApplication::restoreOverrideCursor();
        return;
    }

    bool overrideCursor = true;

    switch (shortcut) {
        case PanModeShortcut: {
            if (event->type() == QEvent::Wheel) {
                // Some QT wheel events are actually be touch pad pan events. From the QT docs:
                // "Wheel events are generated for both mouse wheels and trackpad scroll gestures."
                QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
                inputManager()->canvas()->canvasController()->pan(-wheelEvent->pixelDelta());

                // native gestures don't have cursor tracking by the OS, so they shouldn't show any cursor
                QApplication::restoreOverrideCursor();

                return;
            } else {
                d->originalPreferredCenter = inputManager()->canvas()->canvasController()->preferredCenter();
            }

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
    d->actionStrategy.reset();
    QApplication::restoreOverrideCursor();
    KisAbstractInputAction::end(event);
}

void KisPanAction::inputEvent(QEvent *event)
{
    if(!event) {
        return;
    }

    if (d->actionStrategy && d->actionStrategy->supportsEvent(event)) {
        d->actionStrategy->inputEvent(event);
    } else if (event->type() == QEvent::Wheel) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        inputManager()->canvas()->canvasController()->pan(-wheelEvent->pixelDelta());
    } else {
        KisAbstractInputAction::inputEvent(event);
    }
}

void KisPanAction::cursorMovedAbsolute(const QPointF &startPos, const QPointF &pos)
{
    inputManager()->canvas()->canvasController()->setPreferredCenter(-pos + startPos + d->originalPreferredCenter);
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

bool KisPanAction::supportsHiResInputEvents(int shortcut) const
{
    Q_UNUSED(shortcut);
    return true;
}
