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

#include <kaction.h>
#include <klocalizedstring.h>
#include <kactioncollection.h>
#include <QApplication>

#include <KoToolProxy.h>

#include <kis_canvas2.h>
#include <kis_view2.h>
#include <kis_image.h>
#include <kis_canvas_resource_provider.h>
#include <ko_favorite_resource_manager.h>

#include "kis_abstract_input_action.h"
#include "kis_tool_invocation_action.h"
#include "kis_pan_action.h"
#include "kis_alternate_invocation_action.h"
#include "kis_rotate_canvas_action.h"
#include "kis_zoom_action.h"
#include "kis_show_palette_action.h"
#include "kis_change_primary_setting_action.h"

#include "kis_shortcut_matcher.h"
#include "kis_stroke_shortcut.h"
#include "kis_single_action_shortcut.h"

#include "kis_input_profile.h"
#include "kis_input_profile_manager.h"
#include "kis_shortcut_configuration.h"

class KisInputManager::Private
{
public:
    Private(KisInputManager *qq)
        : q(qq)
        , toolProxy(0)
        , setMirrorMode(false)
        , forwardAllEventsToTool(false)
#ifdef Q_WS_X11
        , hiResEventsWorkaroundCoeff(1.0, 1.0)
#endif
        , lastTabletEvent(0)
    { }

    bool tryHidePopupPalette();
    bool trySetMirrorMode(const QPointF &mousePosition);
    void saveTabletEvent(const QTabletEvent *event);
    void resetSavedTabletEvent(QEvent::Type type);
    void addStrokeShortcut(KisAbstractInputAction* action, int index, const QList< Qt::Key >& modifiers, Qt::MouseButtons buttons);
    void addKeyShortcut(KisAbstractInputAction* action, int index,
                        const QList<Qt::Key> &modifiers);
    void addWheelShortcut(KisAbstractInputAction* action, int index, const QList< Qt::Key >& modifiers, KisShortcutConfiguration::MouseWheelMovement wheelAction);
    bool processUnhandledEvent(QEvent *event);
    Qt::Key workaroundShiftAltMetaHell(const QKeyEvent *keyEvent);
    void setupActions();

    KisInputManager *q;

    KisCanvas2 *canvas;
    KoToolProxy *toolProxy;

    bool setMirrorMode;
    bool forwardAllEventsToTool;

    KisShortcutMatcher matcher;
#ifdef Q_WS_X11
    QPointF hiResEventsWorkaroundCoeff;
#endif
    QTabletEvent *lastTabletEvent;

    KisAbstractInputAction *defaultInputAction;
};

static inline QList<Qt::Key> KEYS() {
    return QList<Qt::Key>();
}
static inline QList<Qt::Key> KEYS(Qt::Key key) {
    return QList<Qt::Key>() << key;
}
static inline QList<Qt::Key> KEYS(Qt::Key key1, Qt::Key key2) {
    return QList<Qt::Key>() << key1 << key2;
}
static inline QList<Qt::Key> KEYS(Qt::Key key1, Qt::Key key2, Qt::Key key3) {
    return QList<Qt::Key>() << key1 << key2 << key3;
}
static inline QList<Qt::MouseButton> BUTTONS(Qt::MouseButton button) {
    return QList<Qt::MouseButton>() << button;
}
static inline QList<Qt::MouseButton> BUTTONS(Qt::MouseButton button1, Qt::MouseButton button2) {
    return QList<Qt::MouseButton>() << button1 << button2;
}


void KisInputManager::Private::addStrokeShortcut(KisAbstractInputAction* action, int index,
                                                 const QList<Qt::Key> &modifiers,
                                                 Qt::MouseButtons buttons)
{
    KisStrokeShortcut *strokeShortcut =
        new KisStrokeShortcut(action, index);

    QList<Qt::MouseButton> buttonList;
    if(buttons & Qt::LeftButton) {
        buttonList << Qt::LeftButton;
    }
    if(buttons & Qt::RightButton) {
        buttonList << Qt::RightButton;
    }
    if(buttons & Qt::MidButton) {
        buttonList << Qt::MidButton;
    }
    if(buttons & Qt::XButton1) {
        buttonList << Qt::XButton1;
    }
    if(buttons & Qt::XButton2) {
        buttonList << Qt::XButton2;
    }

    strokeShortcut->setButtons(modifiers, buttonList);
    matcher.addShortcut(strokeShortcut);
}

void KisInputManager::Private::addKeyShortcut(KisAbstractInputAction* action, int index,
                                              const QList<Qt::Key> &keys)
{
    KisSingleActionShortcut *keyShortcut =
        new KisSingleActionShortcut(action, index);

    QList<Qt::Key> modifiers = keys.mid(1);
    keyShortcut->setKey(modifiers, keys.at(0));
    matcher.addShortcut(keyShortcut);
}

void KisInputManager::Private::addWheelShortcut(KisAbstractInputAction* action, int index,
                                                const QList<Qt::Key> &modifiers,
                                                KisShortcutConfiguration::MouseWheelMovement wheelAction)
{
    KisSingleActionShortcut *keyShortcut =
        new KisSingleActionShortcut(action, index);

    KisSingleActionShortcut::WheelAction a;
    switch(wheelAction) {
        case KisShortcutConfiguration::WheelUp:
            a = KisSingleActionShortcut::WheelUp;
            break;
        case KisShortcutConfiguration::WheelDown:
            a = KisSingleActionShortcut::WheelDown;
            break;
        case KisShortcutConfiguration::WheelLeft:
            a = KisSingleActionShortcut::WheelLeft;
            break;
        case KisShortcutConfiguration::WheelRight:
            a = KisSingleActionShortcut::WheelRight;
            break;
        default:
            return;
    }

    keyShortcut->setWheel(modifiers, a);
    matcher.addShortcut(keyShortcut);
}

void KisInputManager::Private::setupActions()
{
    QList<KisAbstractInputAction*> actions = KisInputProfileManager::instance()->actions();
    foreach(KisAbstractInputAction *action, actions) {
        if(dynamic_cast<KisToolInvocationAction*>(action)) {
            defaultInputAction = action;
        }
    }

    connect(KisInputProfileManager::instance(), SIGNAL(currentProfileChanged()), q, SLOT(profileChanged()));
    if(KisInputProfileManager::instance()->currentProfile()) {
        q->profileChanged();
    }
}

bool KisInputManager::Private::processUnhandledEvent(QEvent *event)
{
    bool retval = false;

    if (forwardAllEventsToTool ||
        event->type() == QEvent::KeyPress ||
        event->type() == QEvent::KeyRelease) {

        defaultInputAction->inputEvent(event);
        retval = true;
    }

    return retval && !forwardAllEventsToTool;
}

Qt::Key KisInputManager::Private::workaroundShiftAltMetaHell(const QKeyEvent *keyEvent)
{
    Qt::Key key = (Qt::Key)keyEvent->key();

    if (keyEvent->key() == Qt::Key_Meta &&
        keyEvent->modifiers().testFlag(Qt::ShiftModifier)) {

        key = Qt::Key_Alt;
    }

    return key;
}

bool KisInputManager::Private::tryHidePopupPalette()
{
    if (canvas->favoriteResourceManager()->isPopupPaletteVisible()) {
        canvas->favoriteResourceManager()->slotShowPopupPalette();
        return true;
    }
    return false;
}

bool KisInputManager::Private::trySetMirrorMode(const QPointF &mousePosition)
{
    if (setMirrorMode) {
        canvas->resourceManager()->setResource(KisCanvasResourceProvider::MirrorAxisCenter, canvas->image()->documentToPixel(mousePosition));
        QApplication::restoreOverrideCursor();
        setMirrorMode = false;
        return true;
    }
    return false;
}

#ifdef Q_WS_X11
inline QPointF dividePoints(const QPointF &pt1, const QPointF &pt2) {
    return QPointF(pt1.x() / pt2.x(), pt1.y() / pt2.y());
}

inline QPointF multiplyPoints(const QPointF &pt1, const QPointF &pt2) {
    return QPointF(pt1.x() * pt2.x(), pt1.y() * pt2.y());
}
#endif

void KisInputManager::Private::saveTabletEvent(const QTabletEvent *event)
{
    delete lastTabletEvent;

#ifdef Q_WS_X11
    /**
     * There is a bug in Qt-x11 when working in 2 tablets + 2 monitors
     * setup. The hiResGlobalPos() value gets scaled wrongly somehow.
     * Happily, the error is linear (without the offset) so we can simply
     * scale it a bit.
     */

    if (event->type() == QEvent::TabletPress) {
        if ((event->globalPos() - event->hiResGlobalPos()).manhattanLength() > 4) {
            hiResEventsWorkaroundCoeff = dividePoints(event->globalPos(), event->hiResGlobalPos());
        } else {
            hiResEventsWorkaroundCoeff = QPointF(1.0, 1.0);
        }
    }
#endif

    lastTabletEvent =
        new QTabletEvent(event->type(),
                         event->pos(),
                         event->globalPos(),
#ifdef Q_WS_X11
                         multiplyPoints(event->hiResGlobalPos(), hiResEventsWorkaroundCoeff),
#else
                         event->hiResGlobalPos(),
#endif
                         event->device(),
                         event->pointerType(),
                         event->pressure(),
                         event->xTilt(),
                         event->yTilt(),
                         event->tangentialPressure(),
                         event->rotation(),
                         event->z(),
                         event->modifiers(),
                         event->uniqueId());
}

void KisInputManager::Private::resetSavedTabletEvent(QEvent::Type type)
{
    bool needResetSavedEvent = true;

#ifdef Q_OS_WIN
    /**
     * For linux platform each mouse event corresponds to a single
     * tablet event so the saved tablet event is deleted after any
     * mouse event.
     *
     * For windows platform the mouse events get compressed so one
     * mouse event may correspond to a few tablet events, so we keep a
     * saved tablet event till the end of the stroke, that is till
     * mouseRelese event
     */
    needResetSavedEvent = type == QEvent::MouseButtonRelease;
#else
    Q_UNUSED(type);
#endif

    if (needResetSavedEvent) {
        delete lastTabletEvent;
        lastTabletEvent = 0;
    }
}

QTabletEvent* KisInputManager::lastTabletEvent() const
{
    return d->lastTabletEvent;
}

KisInputManager::KisInputManager(KisCanvas2 *canvas, KoToolProxy *proxy)
    : QObject(canvas), d(new Private(this))
{
    d->canvas = canvas;
    d->toolProxy = proxy;

    d->setupActions();

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
    delete d;
}

bool KisInputManager::eventFilter(QObject* object, QEvent* event)
{
    Q_UNUSED(object);
    bool retval = false;

    // KoToolProxy needs to pre-process some events to ensure the
    // global shortcuts (not the input manager's ones) are not
    // executed, in particular, this line will accept events when the
    // tool is in text editing, preventing shortcut triggering
    d->toolProxy->processEvent(event);

    switch (event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick: {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        if (d->tryHidePopupPalette() || d->trySetMirrorMode(widgetToPixel(mouseEvent->posF()))) {
            retval = true;
        } else {
            retval = d->matcher.buttonPressed(mouseEvent->button(), mouseEvent);
        }
        d->resetSavedTabletEvent(event->type());
        break;
    }
    case QEvent::MouseButtonRelease: {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        retval = d->matcher.buttonReleased(mouseEvent->button(), mouseEvent);
        d->resetSavedTabletEvent(event->type());
        break;
    }
    case QEvent::KeyPress: {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        Qt::Key key = d->workaroundShiftAltMetaHell(keyEvent);

        if (!keyEvent->isAutoRepeat()) {
            retval = d->matcher.keyPressed(key);
        } else {
            retval = d->matcher.autoRepeatedKeyPressed(key);
        }

        /**
         * Workaround for temporary switching of tools by
         * KoCanvasControllerWidget. We don't need this switch because
         * we handle it ourselves.
         */
        retval |= !d->forwardAllEventsToTool &&
            (keyEvent->key() == Qt::Key_Space ||
             keyEvent->key() == Qt::Key_Escape);

        break;
    }
    case QEvent::KeyRelease: {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (!keyEvent->isAutoRepeat()) {
            Qt::Key key = d->workaroundShiftAltMetaHell(keyEvent);
            retval = d->matcher.keyReleased(key);
        }
        break;
    }
    case QEvent::MouseMove: {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (!d->matcher.mouseMoved(mouseEvent)) {
            //Update the current tool so things like the brush outline gets updated.
            d->toolProxy->mouseMoveEvent(mouseEvent, widgetToPixel(mouseEvent->posF()));
        }
        retval = true;
        d->resetSavedTabletEvent(event->type());
        break;
    }
    case QEvent::Wheel: {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        KisSingleActionShortcut::WheelAction action;

        if(wheelEvent->orientation() == Qt::Horizontal) {
            if(wheelEvent->delta() < 0) {
                action = KisSingleActionShortcut::WheelRight;
            }
            else {
                action = KisSingleActionShortcut::WheelLeft;
            }
        }
        else {
            if(wheelEvent->delta() > 0) {
                action = KisSingleActionShortcut::WheelUp;
            }
            else {
                action = KisSingleActionShortcut::WheelDown;
            }
        }

        retval = d->matcher.wheelEvent(action, wheelEvent);
        break;
    }
    case QEvent::Enter:
        //Ensure we have focus so we get key events.
        d->canvas->canvasWidget()->setFocus();
        break;
    case QEvent::FocusIn:
        //Clear all state so we don't have half-matched shortcuts dangling around.
        d->matcher.reset();
        //Make sure the input actions know we are active.
        KisAbstractInputAction::setInputManager(this);
        break;
    case QEvent::TabletPress:
    case QEvent::TabletMove:
    case QEvent::TabletRelease: {
        //We want both the tablet information and the mouse button state.
        //Since QTabletEvent only provides the tablet information, we
        //save that and then ignore the event so it will generate a mouse
        //event.
        QTabletEvent* tabletEvent = static_cast<QTabletEvent*>(event);
        d->saveTabletEvent(tabletEvent);

#ifdef Q_OS_WIN
        if (event->type() == QEvent::TabletMove) {
            retval = d->matcher.tabletMoved(static_cast<QTabletEvent*>(event));
        }

        if (retval) {
            event->accept();
        } else {
            event->ignore();
        }
#else
        event->ignore();
#endif


        break;
    }
    default:
        break;
    }

    return !retval ? d->processUnhandledEvent(event) : true;
}

KisCanvas2* KisInputManager::canvas() const
{
    return d->canvas;
}

KoToolProxy* KisInputManager::toolProxy() const
{
    return d->toolProxy;
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
        d->forwardAllEventsToTool = true;
        d->matcher.suppressAllActions(true);
    } else {
        d->forwardAllEventsToTool = false;
        d->matcher.suppressAllActions(false);
    }
}

QPointF KisInputManager::widgetToPixel(const QPointF& position)
{
    QPointF pixel = QPointF(position.x() + 0.5f, position.y() + 0.5f);
    return d->canvas->coordinatesConverter()->widgetToDocument(pixel);
}

void KisInputManager::profileChanged()
{
    d->matcher.reset();
    d->matcher.clearShortcuts();

    QList<KisShortcutConfiguration*> shortcuts = KisInputProfileManager::instance()->currentProfile()->allShortcuts();
    foreach(KisShortcutConfiguration *shortcut, shortcuts) {
        switch(shortcut->type()) {
            case KisShortcutConfiguration::KeyCombinationType:
                d->addKeyShortcut(shortcut->action(), shortcut->mode(), shortcut->keys());
                break;
            case KisShortcutConfiguration::MouseButtonType:
                d->addStrokeShortcut(shortcut->action(), shortcut->mode(), shortcut->keys(), shortcut->buttons());
                break;
            case KisShortcutConfiguration::MouseWheelType:
                d->addWheelShortcut(shortcut->action(), shortcut->mode(), shortcut->keys(), shortcut->wheel());
                break;
            default:
                break;
        }
    }
}
