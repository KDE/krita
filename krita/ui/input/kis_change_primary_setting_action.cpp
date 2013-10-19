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
#include <KoToolProxy.h>

KisChangePrimarySettingAction::KisChangePrimarySettingAction()
{
    setName(i18n("Change Primary Setting"));
    setDescription(i18n("The <i>Change Primary Setting</i> action changes a tool's \"Primary Setting\", for example the brush size for the brush tool."));
}

KisChangePrimarySettingAction::~KisChangePrimarySettingAction()
{

}

int KisChangePrimarySettingAction::priority() const
{
    return 8;
}

void KisChangePrimarySettingAction::begin(int shortcut, QEvent *event)
{
    KisAbstractInputAction::begin(shortcut, event);

    QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event);
    if (mouseEvent) {
        QMouseEvent targetEvent(QEvent::MouseButtonPress, mouseEvent->pos(), Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier);
        inputManager()->toolProxy()->mousePressEvent(&targetEvent, inputManager()->widgetToPixel(mouseEvent->posF()));
    }
}

void KisChangePrimarySettingAction::end(QEvent *event)
{
    QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event);
    if (mouseEvent) {
        QMouseEvent targetEvent(QEvent::MouseButtonRelease, mouseEvent->pos(), Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier);
        inputManager()->toolProxy()->mouseReleaseEvent(&targetEvent, inputManager()->widgetToPixel(mouseEvent->posF()));
    }

    KisAbstractInputAction::end(event);
}

void KisChangePrimarySettingAction::mouseMoved(const QPointF &lastPos, const QPointF &pos)
{
    Q_UNUSED(lastPos);

    QMouseEvent targetEvent(QEvent::MouseButtonRelease, pos.toPoint(), Qt::NoButton, Qt::LeftButton, Qt::ShiftModifier);
    inputManager()->toolProxy()->mouseMoveEvent(&targetEvent, inputManager()->widgetToPixel(pos));
}
