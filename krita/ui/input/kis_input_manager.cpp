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

class KisInputManager::Private
{
public:
    Private(KisInputManager *qq)
        : q(qq)
        , toolProxy(0)
        , setMirrorMode(false)
        , forwardAllEventsToTool(false)
        , lastTabletEvent(0)
    { }

    bool tryHidePopupPalette();
    bool trySetMirrorMode(const QPointF &mousePosition);
    void saveTabletEvent(const QTabletEvent *event);
    void resetSavedTabletEvent(QEvent::Type type);
    void addStrokeShortcut(KisAbstractInputAction* action, int index,
                           const QList<Qt::Key> &modifiers,
                           const QList<Qt::MouseButton> &buttons);
    void addKeyShortcut(KisAbstractInputAction* action, int index,
                        const QList<Qt::Key> &modifiers,
                        Qt::Key key);
    void addWheelShortcut(KisAbstractInputAction* action, int index,
                          const QList<Qt::Key> &modifiers,
                          KisSingleActionShortcut::WheelAction wheelAction);
    bool processUnhandledEvent(QEvent *event);
    Qt::Key workaroundShiftAltMetaHell(const QKeyEvent *keyEvent);
    void setupActions();

    KisInputManager *q;

    KisCanvas2 *canvas;
    KoToolProxy *toolProxy;

    bool setMirrorMode;
    bool forwardAllEventsToTool;

    KisShortcutMatcher matcher;
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
                                                 const QList<Qt::MouseButton> &buttons)
{
    KisStrokeShortcut *strokeShortcut =
        new KisStrokeShortcut(action, index);
    strokeShortcut->setButtons(modifiers, buttons);
    matcher.addShortcut(strokeShortcut);
}

void KisInputManager::Private::addKeyShortcut(KisAbstractInputAction* action, int index,
                                              const QList<Qt::Key> &modifiers,
                                              Qt::Key key)
{
    KisSingleActionShortcut *keyShortcut =
        new KisSingleActionShortcut(action, index);
    keyShortcut->setKey(modifiers, key);
    matcher.addShortcut(keyShortcut);
}

void KisInputManager::Private::addWheelShortcut(KisAbstractInputAction* action, int index,
                                                const QList<Qt::Key> &modifiers,
                                                KisSingleActionShortcut::WheelAction wheelAction)
{
    KisSingleActionShortcut *keyShortcut =
        new KisSingleActionShortcut(action, index);
    keyShortcut->setWheel(modifiers, wheelAction);
    matcher.addShortcut(keyShortcut);
}

void KisInputManager::Private::setupActions()
{
#if QT_VERSION >= 0x040700
    Qt::MouseButton middleButton = Qt::MiddleButton;
#else
    Qt::MouseButton middleButton = Qt::MidButton;
#endif

    //Create all the actions.
    KisAbstractInputAction* action = new KisToolInvocationAction(q);
    matcher.addAction(action);
    addStrokeShortcut(action, KisToolInvocationAction::ActivateShortcut, KEYS(), BUTTONS(Qt::LeftButton));
    addKeyShortcut(action, KisToolInvocationAction::ConfirmShortcut, KEYS(), Qt::Key_Return);
    addKeyShortcut(action, KisToolInvocationAction::ConfirmShortcut, KEYS(), Qt::Key_Enter);
    addKeyShortcut(action, KisToolInvocationAction::CancelShortcut, KEYS(), Qt::Key_Escape);
    defaultInputAction = action;

    action = new KisAlternateInvocationAction(q);
    matcher.addAction(action);
    addStrokeShortcut(action, KisAlternateInvocationAction::PrimaryAlternateToggleShortcut, KEYS(Qt::Key_Control), BUTTONS(Qt::LeftButton));
    addStrokeShortcut(action, KisAlternateInvocationAction::SecondaryAlternateToggleShortcut, KEYS(Qt::Key_Control, Qt::Key_Alt), BUTTONS(Qt::LeftButton));

    action = new KisChangePrimarySettingAction(q);
    matcher.addAction(action);
    addStrokeShortcut(action, 0, KEYS(Qt::Key_Shift), BUTTONS(Qt::LeftButton));


    action = new KisPanAction(q);
    matcher.addAction(action);

    addStrokeShortcut(action, KisPanAction::PanToggleShortcut, KEYS(Qt::Key_Space), BUTTONS(Qt::LeftButton));
    addStrokeShortcut(action, KisPanAction::PanToggleShortcut, KEYS(), BUTTONS(middleButton));

    addKeyShortcut(action, KisPanAction::PanLeftShortcut, KEYS(), Qt::Key_Left);
    addKeyShortcut(action, KisPanAction::PanRightShortcut, KEYS(), Qt::Key_Right);
    addKeyShortcut(action, KisPanAction::PanUpShortcut, KEYS(), Qt::Key_Up);
    addKeyShortcut(action, KisPanAction::PanDownShortcut, KEYS(), Qt::Key_Down);


    action = new KisRotateCanvasAction(q);
    matcher.addAction(action);

    addStrokeShortcut(action, KisRotateCanvasAction::RotateToggleShortcut, KEYS(Qt::Key_Shift, Qt::Key_Space), BUTTONS(Qt::LeftButton));
    addStrokeShortcut(action, KisRotateCanvasAction::DiscreteRotateToggleShortcut, KEYS(Qt::Key_Shift, Qt::Key_Alt, Qt::Key_Space), BUTTONS(Qt::LeftButton));
    addStrokeShortcut(action, KisRotateCanvasAction::RotateToggleShortcut, KEYS(Qt::Key_Shift), BUTTONS(middleButton));

    addKeyShortcut(action, KisRotateCanvasAction::RotateLeftShortcut, KEYS(), Qt::Key_4);
    addKeyShortcut(action, KisRotateCanvasAction::RotateResetShortcut, KEYS(), Qt::Key_5);
    addKeyShortcut(action, KisRotateCanvasAction::RotateRightShortcut, KEYS(), Qt::Key_6);


    action = new KisZoomAction(q);
    matcher.addAction(action);

    addStrokeShortcut(action, KisZoomAction::ZoomToggleShortcut, KEYS(Qt::Key_Control), BUTTONS(middleButton));

    addStrokeShortcut(action, KisZoomAction::ZoomToggleShortcut, KEYS(Qt::Key_Control, Qt::Key_Space), BUTTONS(Qt::LeftButton));
    addStrokeShortcut(action, KisZoomAction::DiscreteZoomToggleShortcut, KEYS(Qt::Key_Control, Qt::Key_Alt, Qt::Key_Space), BUTTONS(Qt::LeftButton));

    addWheelShortcut(action, KisZoomAction::ZoomInShortcut, KEYS(), KisSingleActionShortcut::WheelUp);
    addWheelShortcut(action, KisZoomAction::ZoomOutShortcut, KEYS(), KisSingleActionShortcut::WheelDown);

    addKeyShortcut(action, KisZoomAction::ZoomInShortcut, KEYS(), Qt::Key_Plus);
    addKeyShortcut(action, KisZoomAction::ZoomOutShortcut, KEYS(), Qt::Key_Minus);

    addKeyShortcut(action, KisZoomAction::ZoomResetShortcut, KEYS(), Qt::Key_1);
    addKeyShortcut(action, KisZoomAction::ZoomToPageShortcut, KEYS(), Qt::Key_2);
    addKeyShortcut(action, KisZoomAction::ZoomToWidthShortcut, KEYS(), Qt::Key_3);

    action = new KisShowPaletteAction(q);
    matcher.addAction(action);

    addStrokeShortcut(action, 0, KEYS(), BUTTONS(Qt::RightButton));
    addKeyShortcut(action, 0, KEYS(), Qt::Key_F);
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

void KisInputManager::Private::saveTabletEvent(const QTabletEvent *event)
{
    delete lastTabletEvent;
    lastTabletEvent =
        new QTabletEvent(event->type(),
                         event->pos(),
                         event->globalPos(),
                         event->hiResGlobalPos(),
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
        KisSingleActionShortcut::WheelAction action =
            wheelEvent->delta() > 0 ?
            KisSingleActionShortcut::WheelUp : KisSingleActionShortcut::WheelDown;

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

