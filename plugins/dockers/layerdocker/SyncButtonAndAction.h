/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
