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

#include "kis_change_primary_setting_action.h"

#include <klocalizedstring.h>

#include "kis_input_manager.h"
#include "kis_canvas2.h"
#include "kis_tool_proxy.h"

#include <QApplication>
#include "kis_cursor.h"


KisChangePrimarySettingAction::KisChangePrimarySettingAction()
    : KisAbstractInputAction("Change Primary Setting")
{
    setName(i18n("Change Primary Setting"));
    setDescription(i18n("The <i>Change Primary Setting</i> action changes a tool's \"Primary Setting\", for example the brush size for the brush tool."));
}

KisChangePrimarySettingAction::~KisChangePrimarySettingAction()
{

}

void KisChangePrimarySettingAction::activate(int shortcut)
{
    Q_UNUSED(shortcut);
    inputManager()->toolProxy()->activateToolAction(KisTool::AlternateChangeSize);
}

void KisChangePrimarySettingAction::deactivate(int shortcut)
{
    Q_UNUSED(shortcut);
    inputManager()->toolProxy()->deactivateToolAction(KisTool::AlternateChangeSize);
}

int KisChangePrimarySettingAction::priority() const
{
    return 8;
}

void KisChangePrimarySettingAction::begin(int shortcut, QEvent *event)
{
    KisAbstractInputAction::begin(shortcut, event);

    if (event) {
        QMouseEvent targetEvent(QEvent::MouseButtonPress, eventPosF(event), Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier);

        inputManager()->toolProxy()->forwardEvent(KisToolProxy::BEGIN, KisTool::AlternateChangeSize, &targetEvent, event);
    }
}

void KisChangePrimarySettingAction::end(QEvent *event)
{
    if (event) {
        QMouseEvent targetEvent(QEvent::MouseButtonRelease, eventPosF(event), Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier);
        inputManager()->toolProxy()->forwardEvent(KisToolProxy::END, KisTool::AlternateChangeSize, &targetEvent, event);
    }

    KisAbstractInputAction::end(event);
}

void KisChangePrimarySettingAction::inputEvent(QEvent* event)
{
    if (event && (event->type() == QEvent::MouseMove || event->type() == QEvent::TabletMove)) {
        QMouseEvent targetEvent(QEvent::MouseMove, eventPos(event), Qt::NoButton, Qt::LeftButton, Qt::ShiftModifier);
        inputManager()->toolProxy()->forwardEvent(KisToolProxy::CONTINUE, KisTool::AlternateChangeSize, &targetEvent, event);
    }
}
