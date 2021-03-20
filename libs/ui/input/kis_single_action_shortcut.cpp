/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_single_action_shortcut.h"

#include "kis_abstract_input_action.h"

class Q_DECL_HIDDEN KisSingleActionShortcut::Private
{
public:
    QSet<Qt::Key> modifiers;
    Qt::Key key;
    bool useWheel;
    WheelAction wheelAction;
};


KisSingleActionShortcut::KisSingleActionShortcut(KisAbstractInputAction *action, int index)
    : KisAbstractShortcut(action, index),
      m_d(new Private)
{
}

KisSingleActionShortcut::~KisSingleActionShortcut()
{
    delete m_d;
}

int KisSingleActionShortcut::priority() const
{
    return m_d->modifiers.size() * 2 + 1 + action()->priority();
}

void KisSingleActionShortcut::setKey(const QSet<Qt::Key> &modifiers, Qt::Key key)
{
    m_d->modifiers = modifiers;
    m_d->key = key;
    m_d->useWheel = false;
}

void KisSingleActionShortcut::setWheel(const QSet<Qt::Key> &modifiers, WheelAction wheelAction)
{
    m_d->modifiers = modifiers;
    m_d->wheelAction = wheelAction;
    m_d->useWheel = true;
}

bool KisSingleActionShortcut::match(const QSet<Qt::Key> &modifiers, Qt::Key key)
{
    return !m_d->useWheel && key == m_d->key &&
        compareKeys(modifiers, m_d->modifiers);
}

bool KisSingleActionShortcut::match(const QSet<Qt::Key> &modifiers, WheelAction wheelAction)
{
    return m_d->useWheel && wheelAction == m_d->wheelAction &&
        compareKeys(modifiers, m_d->modifiers);
}
