
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

#include "kis_input_manager.h"

#include <QDebug>
#include <QQueue>

#include <KAction>
#include <KLocalizedString>
#include <KActionCollection>
#include <QApplication>

#include <KoToolProxy.h>

#include <kis_canvas2.h>
#include <kis_view2.h>
#include <kis_image.h>
#include <kis_canvas_resource_provider.h>
#include <ko_favorite_resource_manager.h>

#include "kis_shortcut.h"
#include "kis_abstract_input_action.h"
#include "kis_tool_invocation_action.h"
#include "kis_pan_action.h"
#include "kis_alternate_invocation_action.h"
#include "kis_rotate_canvas_action.h"
#include "kis_zoom_action.h"
#include "kis_show_palette_action.h"
#include "kis_change_primary_setting_action.h"

class KisInputManager::Private
{
public:
    Private(KisInputManager *qq)
        : q(qq)
        , toolProxy(0)
        , currentAction(0)
        , currentShortcut(0)
        , tabletPressEvent(0)
        , setMirrorMode(false)
        , fixedAction(false)
    { }

    void match(QEvent *event);
    void setupActions();
    KisShortcut *createShortcut(KisAbstractInputAction* action, int index);
    void clearState();

    KisInputManager *q;

    KisCanvas2 *canvas;
    KoToolProxy *toolProxy;

    KisAbstractInputAction* currentAction;
    KisShortcut* currentShortcut;

    QList<KisShortcut*> shortcuts;
    QList<KisAbstractInputAction*> actions;

    QList<KisShortcut*> potentialShortcuts;

    QPointF mousePosition;

    QTabletEvent *tabletPressEvent;

    bool setMirrorMode;
    bool fixedAction;
};

KisInputManager::KisInputManager(KisCanvas2 *canvas, KoToolProxy *proxy)
    : QObject(canvas), d(new Private(this))
{
    d->canvas = canvas;
    d->toolProxy = proxy;

    d->setupActions();

    d->potentialShortcuts = d->shortcuts;

    /*
     * Temporary solution so we can still set the mirror axis.
     *
     * TODO: Create a proper interface for this.
     * There really should be a better way to handle this, one that neither
     * relies on "hidden" mouse interaction or shortcuts.
     */
    KAction *setMirrorAxis = new KAction(i18n("Set Mirror Axis"), this);
    d->canvas->view()->actionCollection()->addAction("set_mirror_axis", setMirrorAxis);
    setMirrorAxis->setShortcut(QKeySequence("Shift+r"));
    connect(setMirrorAxis, SIGNAL(triggered(bool)), SLOT(setMirrorAxis()));

    connect(KoToolManager::instance(), SIGNAL(changedTool(KoCanvasController*,int)),
            SLOT(slotToolChanged()));
}

KisInputManager::~KisInputManager()
{
    qDeleteAll(d->shortcuts);
    qDeleteAll(d->actions);
    delete d;
}

bool KisInputManager::eventFilter(QObject* object, QEvent* event)
{
    Q_UNUSED(object)
    switch (event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick: {
        d->mousePosition = widgetToPixel(static_cast<QMouseEvent*>(event)->posF());

        //If the palette is visible, then hide it and eat the event.
        if (canvas()->favoriteResourceManager()->isPopupPaletteVisible()) {
            canvas()->favoriteResourceManager()->slotShowPopupPalette();
            return true;
        }

        if (d->setMirrorMode) {
            d->canvas->resourceManager()->setResource(KisCanvasResourceProvider::MirrorAxisCenter, d->canvas->image()->documentToPixel(d->mousePosition));
            QApplication::restoreOverrideCursor();
            d->setMirrorMode = false;
            return true;
        }
    } //Intentional fall through
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
        if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
            QKeyEvent *kevent = static_cast<QKeyEvent*>(event);
            if (kevent->isAutoRepeat()) {
                if (d->currentAction) {
                    if (d->currentAction->isBlockingAutoRepeat()) {
                        return true; //Ignore auto repeat key events if the action is asking for it.
                    }
                } else {
                    return true; //Always ignore auto repeat key events when we do not have a current action.
                }
            }
        }
        //Intentional fall through
    case QEvent::MouseButtonRelease:
        if (d->currentAction) { //If we are currently performing an action, we only update the state of that action and shortcut.
            d->currentShortcut->match(event);

            if (d->currentShortcut->matchLevel() == KisShortcut::NoMatch && !d->fixedAction) {
                d->clearState();
                break;
            }

            d->currentAction->inputEvent(event);
        } else { //Try to find a matching shortcut.
            d->match(event);
        }
        return true;
    case QEvent::MouseMove:
        if (!d->currentAction) {
            QMouseEvent *mevent = static_cast<QMouseEvent*>(event);
            //Update the current tool so things like the brush outline gets updated.
            d->toolProxy->mouseMoveEvent(mevent, widgetToPixel(mevent->posF()));
        } else {
            d->currentAction->inputEvent(event);
        }
        return true;
    case QEvent::Wheel:
        if (d->currentAction) {
            d->currentAction->inputEvent(event);
        } else {
            d->match(event);
            if (d->currentAction) {
                d->clearState();
            }
        }
        break;
    case QEvent::Enter:
        //Ensure we have focus so we get key events.
        d->canvas->canvasWidget()->setFocus();
        return true;
    case QEvent::Leave:
        //Clear all state so we don't have half-matched shortcuts dangling around.
        d->clearState();
        return true;
    case QEvent::TabletPress: {
        //We want both the tablet information and the mouse button state.
        //Since QTabletEvent only provides the tablet information, we save that
        //and then ignore the event so it will generate a mouse event.
        QTabletEvent* tevent = static_cast<QTabletEvent*>(event);

        //Since events get deleted once they are processed we need to clone the event
        //to save it.
        QTabletEvent* newEvent = new QTabletEvent(QEvent::TabletPress,
                                                  tevent->pos(),
                                                  tevent->globalPos(),
                                                  tevent->hiResGlobalPos(),
                                                  tevent->device(),
                                                  tevent->pointerType(),
                                                  tevent->pressure(),
                                                  tevent->xTilt(),
                                                  tevent->yTilt(),
                                                  tevent->tangentialPressure(),
                                                  tevent->rotation(),
                                                  tevent->z(),
                                                  tevent->modifiers(),
                                                  tevent->uniqueId()
                                                  );
        d->tabletPressEvent = newEvent;
        event->ignore();
        break;
    }
    case QEvent::TabletMove:
        //Only process tablet move events if the current action has special code for it.
        //In all other cases, we simply ignore it so it will generate a mouse event
        //instead.
        if (d->currentAction && d->currentAction->handleTablet()) {
            d->currentAction->inputEvent(event);
            return true;
        } else {
            event->ignore();
        }
        break;
    case QEvent::TabletRelease:
        //Always ignore tablet release events and have them generate mouse events instead.
        event->ignore();
    default:
        break;
    }

    return false;
}

KisCanvas2* KisInputManager::canvas() const
{
    return d->canvas;
}

KoToolProxy* KisInputManager::toolProxy() const
{
    return d->toolProxy;
}

QPointF KisInputManager::mousePosition() const
{
    return d->mousePosition;
}

QTabletEvent* KisInputManager::tabletPressEvent() const
{
    return d->tabletPressEvent;
}

void KisInputManager::setMirrorAxis()
{
    d->setMirrorMode = true;
    QApplication::setOverrideCursor(Qt::CrossCursor);
}

void KisInputManager::slotToolChanged()
{
    QString toolId = KoToolManager::instance()->activeToolId();
    if (toolId == "ArtisticTextToolFactoryID" || toolId == "TextToolFactory_ID") {
        d->fixedAction = true;
        if (!d->currentAction) {
            d->currentShortcut = d->shortcuts.at(0);
            d->currentAction = d->currentShortcut->action();
            d->currentAction->begin(d->currentShortcut->shortcutIndex());
        }
    } else {
        d->fixedAction = false;
    }
}

QPointF KisInputManager::widgetToPixel(const QPointF& position)
{
    QPointF pixel = QPointF(position.x() + 0.5f, position.y() + 0.5f);
    return d->canvas->coordinatesConverter()->widgetToDocument(pixel);
}

void KisInputManager::Private::match(QEvent* event)
{
    if (fixedAction) {
        return;
    }
    //Go through all possible shortcuts and update their state.
    foreach (KisShortcut* shortcut, potentialShortcuts) {
        shortcut->match(event);
        if(shortcut->matchLevel() == KisShortcut::NoMatch) {
            //There is no chance of this shortcut matching anything with the current input,
            //so remove it from the list of shortcuts to search through.
            potentialShortcuts.removeOne(shortcut);
        }
    }

    if (potentialShortcuts.count() == 0) {
        //With the current input, there is simply no shortcut that matches,
        //so restart the matching.
        potentialShortcuts = shortcuts;
        return;
    }

    if (potentialShortcuts.count() == 1 || event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick) {
        //Either we have only one possible match or we reached the queue threshold.
        KisShortcut* completedShortcut = 0;
        foreach (KisShortcut* shortcut, potentialShortcuts) {
            if (shortcut->matchLevel() == KisShortcut::CompleteMatch) {
                //Set the matched shortcut to the one with the highest priority.std::
                if (!completedShortcut || completedShortcut->priority() < shortcut->priority()) {
                    completedShortcut = shortcut;
                }
            } else {
                shortcut->clear();
            }
        }

        //We really do have a matched action, so lets activate it.
        if (completedShortcut) {
            currentShortcut = completedShortcut;
            currentAction = completedShortcut->action();
            currentAction->begin(completedShortcut->shortcutIndex());
        }
    }
}

void KisInputManager::Private::setupActions()
{
    //Create all the actions.
    KisAbstractInputAction* action = new KisToolInvocationAction(q);
    actions.append(action);

    KisShortcut* shortcut = createShortcut(action, KisToolInvocationAction::ActivateShortcut);
    shortcut->setButtons(QList<Qt::MouseButton>() << Qt::LeftButton);

    shortcut = createShortcut(action, KisToolInvocationAction::ConfirmShortcut);
    shortcut->setKeys(QList<Qt::Key>() << Qt::Key_Return);

    action = new KisAlternateInvocationAction(q);
    actions.append(action);

    shortcut = createShortcut(action, 0);
    shortcut->setButtons(QList<Qt::MouseButton>() << Qt::LeftButton);
    shortcut->setKeys(QList<Qt::Key>() << Qt::Key_Control);

    action = new KisChangePrimarySettingAction(q);
    actions.append(action);

    shortcut = createShortcut(action, 0);
    shortcut->setButtons(QList<Qt::MouseButton>() << Qt::LeftButton);
    shortcut->setKeys(QList<Qt::Key>() << Qt::Key_Shift);

    action = new KisPanAction(q);
    actions.append(action);

    shortcut = createShortcut(action, KisPanAction::PanToggleShortcut);
    shortcut->setKeys(QList<Qt::Key>() << Qt::Key_Space);

    shortcut = createShortcut(action, KisPanAction::PanToggleShortcut);
#if QT_VERSION >= 0x040700
    shortcut->setButtons(QList<Qt::MouseButton>() << Qt::MiddleButton);
#else
    shortcut->setButtons(QList<Qt::MouseButton>() << Qt::MidButton);
#endif

    shortcut = createShortcut(action, KisPanAction::PanLeftShortcut);
    shortcut->setKeys(QList<Qt::Key>() << Qt::Key_Left);
    shortcut = createShortcut(action, KisPanAction::PanRightShortcut);
    shortcut->setKeys(QList<Qt::Key>() << Qt::Key_Right);
    shortcut = createShortcut(action, KisPanAction::PanUpShortcut);
    shortcut->setKeys(QList<Qt::Key>() << Qt::Key_Up);
    shortcut = createShortcut(action, KisPanAction::PanDownShortcut);
    shortcut->setKeys(QList<Qt::Key>() << Qt::Key_Down);

    action = new KisRotateCanvasAction(q);
    actions.append(action);

    shortcut = createShortcut(action, KisRotateCanvasAction::RotateToggleShortcut);
    shortcut->setKeys(QList<Qt::Key>() << Qt::Key_Shift << Qt::Key_Space);

    shortcut = createShortcut(action, KisRotateCanvasAction::RotateToggleShortcut);
    shortcut->setKeys(QList<Qt::Key>() << Qt::Key_Shift);
#if QT_VERSION >= 0x040700
    shortcut->setButtons(QList<Qt::MouseButton>() << Qt::MiddleButton);
#else
    shortcut->setButtons(QList<Qt::MouseButton>() << Qt::MidButton);
#endif

    shortcut = createShortcut(action, KisRotateCanvasAction::RotateLeftShortcut);
    shortcut->setKeys(QList<Qt::Key>() << Qt::Key_4);

    shortcut = createShortcut(action, KisRotateCanvasAction::RotateRightShortcut);
    shortcut->setKeys(QList<Qt::Key>() << Qt::Key_6);

    shortcut = createShortcut(action, KisRotateCanvasAction::RotateResetShortcut);
    shortcut->setKeys(QList<Qt::Key>() << Qt::Key_5);

    action = new KisZoomAction(q);
    actions.append(action);

    shortcut = createShortcut(action, KisZoomAction::ZoomToggleShortcut);
    shortcut->setKeys(QList<Qt::Key>() << Qt::Key_Control << Qt::Key_Space);

    shortcut = createShortcut(action, KisZoomAction::ZoomToggleShortcut);
    shortcut->setKeys(QList<Qt::Key>() << Qt::Key_Control);
#if QT_VERSION >= 0x040700
    shortcut->setButtons(QList<Qt::MouseButton>() << Qt::MiddleButton);
#else
    shortcut->setButtons(QList<Qt::MouseButton>() << Qt::MidButton);
#endif

    shortcut = createShortcut(action, KisZoomAction::ZoomInShortcut);
    shortcut->setWheel(KisShortcut::WheelUp);

    shortcut = createShortcut(action, KisZoomAction::ZoomOutShortcut);
    shortcut->setWheel(KisShortcut::WheelDown);

    shortcut = createShortcut(action, KisZoomAction::ZoomInShortcut);
    shortcut->setKeys(QList<Qt::Key>() << Qt::Key_Plus);
    shortcut = createShortcut(action, KisZoomAction::ZoomOutShortcut);
    shortcut->setKeys(QList<Qt::Key>() << Qt::Key_Minus);

    shortcut = createShortcut(action, KisZoomAction::ZoomResetShortcut);
    shortcut->setKeys(QList<Qt::Key>() << Qt::Key_1);
    shortcut = createShortcut(action, KisZoomAction::ZoomToPageShortcut);
    shortcut->setKeys(QList<Qt::Key>() << Qt::Key_2);
    shortcut = createShortcut(action, KisZoomAction::ZoomToWidthShortcut);
    shortcut->setKeys(QList<Qt::Key>() << Qt::Key_3);

    action = new KisShowPaletteAction(q);
    actions.append(action);

    shortcut = createShortcut(action, 0);
    shortcut->setButtons(QList<Qt::MouseButton>() << Qt::RightButton);

    shortcut = createShortcut(action, 0);
    shortcut->setKeys(QList<Qt::Key>() << Qt::Key_F);
}

KisShortcut* KisInputManager::Private::createShortcut(KisAbstractInputAction* action, int index)
{
    KisShortcut* shortcut = new KisShortcut;
    shortcut->setAction(action);
    shortcut->setShortcutIndex(index);
    shortcuts.append(shortcut);

    return shortcut;
}

void KisInputManager::Private::clearState()
{
    if (fixedAction) {
        return;
    }

    if (currentShortcut) {
        currentAction->end();
        currentAction = 0;
        currentShortcut = 0;
        potentialShortcuts = shortcuts;

        delete tabletPressEvent;
        tabletPressEvent = 0;
    }

    foreach (KisShortcut* shortcut, shortcuts) {
        shortcut->clear();
    }
}
