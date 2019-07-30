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
    Private()
        : active(false),
          lineToolActivated(false)
    {
    }

    bool active;
    bool lineToolActivated;
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

    if (shortcut == LineToolShortcut) {
        KoToolManager::instance()->switchToolTemporaryRequested("KritaShape/KisToolLine");
        d->lineToolActivated = true;
    }

    d->activatedToolProxy = inputManager()->toolProxy();
    d->activatedToolProxy->activateToolAction(KisTool::Primary);
}

void KisToolInvocationAction::deactivate(int shortcut)
{
    Q_UNUSED(shortcut);
    if (!inputManager()) return;

    KIS_SAFE_ASSERT_RECOVER_NOOP(d->activatedToolProxy);
    if (d->activatedToolProxy) {
        d->activatedToolProxy->deactivateToolAction(KisTool::Primary);
        d->activatedToolProxy.clear();
    }

    if (shortcut == LineToolShortcut && d->lineToolActivated) {
        d->lineToolActivated = false;
        KoToolManager::instance()->switchBackRequested();
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
    if (shortcut == ActivateShortcut || shortcut == LineToolShortcut) {
        d->runningToolProxy = inputManager()->toolProxy();
        d->active =
            d->runningToolProxy->forwardEvent(
                KisToolProxy::BEGIN, KisTool::Primary, event, event);
    } else if (shortcut == ConfirmShortcut) {
        QKeyEvent pressEvent(QEvent::KeyPress, Qt::Key_Return, 0);
        inputManager()->toolProxy()->keyPressEvent(&pressEvent);
        QKeyEvent releaseEvent(QEvent::KeyRelease, Qt::Key_Return, 0);
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
    d->active = true;
    inputEvent(event);
    d->active = savedState;
}

bool KisToolInvocationAction::supportsHiResInputEvents() const
{
    return inputManager()->toolProxy()->primaryActionSupportsHiResEvents();
}

bool KisToolInvocationAction::isShortcutRequired(int shortcut) const
{
    //These really all are pretty important for basic user interaction.
    Q_UNUSED(shortcut)
    return true;
}
