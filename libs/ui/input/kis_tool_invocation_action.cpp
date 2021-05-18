/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tool_invocation_action.h"

#include <kis_debug.h>

#include <klocalizedstring.h>

#include <KoToolManager.h>

#include <kis_tool_proxy.h>
#include <kis_canvas2.h>
#include <kis_coordinates_converter.h>

#include "kis_tool.h"
#include "kis_input_manager.h"
#include "kis_image.h"

class KisToolInvocationAction::Private
{
public:
    bool active {false};
    bool deactivating {false};

    QPointer<KisToolProxy> activatedToolProxy;
    QPointer<KisToolProxy> runningToolProxy;
};

KisToolInvocationAction::KisToolInvocationAction()
    : KisAbstractInputAction("Tool Invocation")
    , d(new Private)
{
    setName(i18n("Tool Invocation"));
    setDescription(i18n("The <i>Tool Invocation</i> action invokes the current tool, for example, using the brush tool, it will start painting."));

    QHash<QString, int> indexes;
    indexes.insert(i18n("Activate"), ActivateShortcut);
    indexes.insert(i18n("Confirm"), ConfirmShortcut);
    indexes.insert(i18n("Cancel"), CancelShortcut);
    indexes.insert(i18n("Activate Line Tool"), LineToolShortcut);
    indexes.insert(i18n("Activate Ellipse Tool"), EllipseToolShortcut);
    indexes.insert(i18n("Activate Rectangle Tool"), RectToolShortcut);
    indexes.insert(i18n("Activate Move Tool"), MoveToolShortcut);
    indexes.insert(i18n("Activate Fill Tool"), FillToolShortcut);
    indexes.insert(i18n("Activate Gradient Tool"), GradientToolShortcut);
    indexes.insert(i18n("Activate Measure Tool"), MeasureToolShortcut);
    indexes.insert(i18n("Activate Elliptical Selection Tool"), EllipseSelToolShortcut);
    indexes.insert(i18n("Activate Rectangular Selection Tool"), RectSelToolShortcut);
    indexes.insert(i18n("Activate Contiguous Selection Tool"), ContigSelToolShortcut);
    indexes.insert(i18n("Activate Freehand Selection Tool"), FreehandSelToolShortcut);

    setShortcutIndexes(indexes);
}

KisToolInvocationAction::~KisToolInvocationAction()
{
    delete d;
}

void KisToolInvocationAction::activate(int shortcut)
{
    Q_UNUSED(shortcut);
    if (!inputManager()) return;

    QString temporaryToolId;
    switch(shortcut) {
        case LineToolShortcut:
            temporaryToolId = "KritaShape/KisToolLine";
            break;
        case EllipseToolShortcut:
            temporaryToolId = "KritaShape/KisToolEllipse";
            break;
        case RectToolShortcut:
            temporaryToolId = "KritaShape/KisToolRectangle";
            break;
        case MoveToolShortcut:
            temporaryToolId = "KritaTransform/KisToolMove";
            break;
        case FillToolShortcut:
            temporaryToolId = "KritaFill/KisToolFill";
            break;
        case GradientToolShortcut:
            temporaryToolId = "KritaFill/KisToolGradient";
            break;
        case MeasureToolShortcut:
            temporaryToolId = "KritaShape/KisToolMeasure";
            break;
        case EllipseSelToolShortcut:
            temporaryToolId = "KisToolSelectElliptical";
            break;
        case RectSelToolShortcut:
            temporaryToolId = "KisToolSelectRectangular";
            break;
        case ContigSelToolShortcut:
            temporaryToolId = "KisToolSelectContiguous";
            break;
        case FreehandSelToolShortcut:
            temporaryToolId = "KisToolSelectOutline";
            break;
    }

    if (!temporaryToolId.isEmpty()) {
        KoToolManager::instance()->switchToolRequested(temporaryToolId);
    }

    d->activatedToolProxy = inputManager()->toolProxy();
    d->activatedToolProxy->activateToolAction(KisTool::Primary);
}

void KisToolInvocationAction::deactivate(int shortcut)
{
    Q_UNUSED(shortcut);
    if (!inputManager()) return;

    /**
     * Activate call might have come before actual input manager or tool proxy
     * was attached. So we may end up with null activatedToolProxy.
     */
    if (d->activatedToolProxy) {
        d->activatedToolProxy->deactivateToolAction(KisTool::Primary);
        d->activatedToolProxy.clear();
    }

    if (shortcut != ActivateShortcut && shortcut != ConfirmShortcut && shortcut != CancelShortcut && !d->deactivating) {
        // Switching tool will force deactivation and will re-enter
        d->deactivating = true;
        KoToolManager::instance()->switchBackRequested();
        d->deactivating = false;
    }
}

int KisToolInvocationAction::priority() const
{
    return 0;
}

bool KisToolInvocationAction::canIgnoreModifiers() const
{
    return true;
}

void KisToolInvocationAction::begin(int shortcut, QEvent *event)
{
    if (shortcut != ConfirmShortcut && shortcut != CancelShortcut ) {
        d->runningToolProxy = inputManager()->toolProxy();
        d->active =
            d->runningToolProxy->forwardEvent(
                KisToolProxy::BEGIN, KisTool::Primary, event, event);
    }
    else if (shortcut == ConfirmShortcut) {
        QKeyEvent pressEvent(QEvent::KeyPress, Qt::Key_Return, QFlags<Qt::KeyboardModifier>());
        inputManager()->toolProxy()->keyPressEvent(&pressEvent);
        QKeyEvent releaseEvent(QEvent::KeyRelease, Qt::Key_Return, QFlags<Qt::KeyboardModifier>());
        inputManager()->toolProxy()->keyReleaseEvent(&releaseEvent);

        /**
         * All the tools now have a KisTool::requestStrokeEnd() method,
         * so they should use this instead of direct filtering Enter key
         * press. Until all the tools support it, we just duplicate the
         * key event and the method call
         */
        inputManager()->canvas()->image()->requestStrokeEnd();

        /**
         * Some tools would like to distinguish automated requestStrokeEnd()
         * calls from explicit user actions. Just let them do it!
         *
         * Please note that this call should happen **after**
         * requestStrokeEnd(). Some of the tools will switch to another
         * tool on this request, and this (next) tool does not expect to
         * get requestStrokeEnd() right after switching in.
         */
        inputManager()->toolProxy()->explicitUserStrokeEndRequest();

    } else if (shortcut == CancelShortcut) {
        /**
         * The tools now have a KisTool::requestStrokeCancellation() method,
         * so just request it.
         */

        inputManager()->canvas()->image()->requestStrokeCancellation();
    }
}

void KisToolInvocationAction::end(QEvent *event)
{
    if (d->active) {
        // It might happen that the action is still running, while the
        // canvas has been removed, which kills the toolProxy.
        KIS_SAFE_ASSERT_RECOVER_NOOP(d->runningToolProxy);
        if (d->runningToolProxy) {
            d->runningToolProxy->
                forwardEvent(KisToolProxy::END, KisTool::Primary, event, event);
            d->runningToolProxy.clear();
        }
        d->active = false;
    }

    KisAbstractInputAction::end(event);
}

void KisToolInvocationAction::inputEvent(QEvent* event)
{
    if (!d->active) return;
    if (!d->runningToolProxy) return;

    d->runningToolProxy->
        forwardEvent(KisToolProxy::CONTINUE, KisTool::Primary, event, event);
}

void KisToolInvocationAction::processUnhandledEvent(QEvent* event)
{
    bool savedState = d->active;
    KisToolProxy *savedToolProxy = d->runningToolProxy;
    if (!d->runningToolProxy) {
        d->runningToolProxy = inputManager()->toolProxy();
    }
    d->active = true;
    inputEvent(event);
    d->active = savedState;
    d->runningToolProxy = savedToolProxy;
}

bool KisToolInvocationAction::supportsHiResInputEvents(int shortcut) const
{
    Q_UNUSED(shortcut);
    return inputManager()->toolProxy()->primaryActionSupportsHiResEvents();
}

bool KisToolInvocationAction::isShortcutRequired(int shortcut) const
{
    // These three shortcuts are pretty important for basic user interaction,
    // so they should always exist.
    return (shortcut == ActivateShortcut || shortcut == ConfirmShortcut || shortcut == CancelShortcut);
}
