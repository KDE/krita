/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef SYNC_BUTTON_AND_ACTION_H
#define SYNC_BUTTON_AND_ACTION_H

#include <QPointer>
#include <QAbstractButton>
#include "kis_action.h"

class SyncButtonAndAction : public QObject
{
    Q_OBJECT
public:
    SyncButtonAndAction(KisAction *action, QAbstractButton *button, QObject *parent)
        : QObject(parent),
          m_action(action),
          m_button(button)
    {
        connect(m_action, SIGNAL(changed()), SLOT(slotActionChanged()));
        connect(m_button, SIGNAL(clicked()), m_action, SLOT(trigger()));
        m_button->setIcon(m_action->icon());
        m_button->setText(m_action->text());
    }

private Q_SLOTS:
    void slotActionChanged() {
        if (m_action && m_button &&
            m_action->isEnabled() != m_button->isEnabled()) {

            m_button->setEnabled(m_action->isEnabled());
        }
    }

private:
    QPointer<KisAction> m_action;
    QPointer<QAbstractButton> m_button;
};

#endif /* __SYNC_BUTTON_AND_ACTION_H */
