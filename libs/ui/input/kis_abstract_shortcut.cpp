/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_abstract_shortcut.h"

#include "kis_abstract_input_action.h"


class Q_DECL_HIDDEN KisAbstractShortcut::Private
{
public:
    KisAbstractInputAction *action;
    int shortcutIndex;
};

KisAbstractShortcut::KisAbstractShortcut(KisAbstractInputAction *action, int index)
    : m_d(new Private)
{
    m_d->action = action;
    m_d->shortcutIndex = index;
}

KisAbstractShortcut::~KisAbstractShortcut()
{
    delete m_d;
}

KisAbstractInputAction* KisAbstractShortcut::action() const
{
    return m_d->action;
}

void KisAbstractShortcut::setAction(KisAbstractInputAction* action)
{
    m_d->action = action;
}

int KisAbstractShortcut::shortcutIndex() const
{
    return m_d->shortcutIndex;
}

bool KisAbstractShortcut::compareKeys(const QSet<Qt::Key> &keys1,
                                      const QSet<Qt::Key> &keys2)
{
    if (keys1.size() != keys2.size()) return false;

    Q_FOREACH (Qt::Key key, keys1) {
        if (!keys2.contains(key)) return false;
    }
    return true;
}

bool KisAbstractShortcut::isAvailable(KisInputActionGroupsMask groupMask) const
{
    return
        (action()->inputActionGroup(m_d->shortcutIndex) & groupMask) &&
        action()->isAvailable();
}
