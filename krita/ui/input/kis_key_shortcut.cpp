/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_key_shortcut.h"

struct KisKeyShortcut::Private
{
    QList<Qt::Key> modifiers;
    Qt::Key key;
    bool useWheel;
    WheelAction wheelAction;
};


KisKeyShortcut::KisKeyShortcut(KisAbstractInputAction *action, int index)
    : KisAbstractShortcut(action, index),
      m_d(new Private)
{
}

KisKeyShortcut::~KisKeyShortcut()
{
    delete m_d;
}

int KisKeyShortcut::priority() const
{
    return m_d->modifiers.size() * 2 + 1;
}

void KisKeyShortcut::setKey(const QList<Qt::Key> &modifiers, Qt::Key key)
{
    m_d->modifiers = modifiers;
    m_d->key = key;
    m_d->useWheel = false;
}

void KisKeyShortcut::setWheel(const QList<Qt::Key> &modifiers, WheelAction wheelAction)
{
    m_d->modifiers = modifiers;
    m_d->wheelAction = wheelAction;
    m_d->useWheel = true;
}

bool KisKeyShortcut::matchKey(const QList<Qt::Key> &modifiers, Qt::Key key)
{
    return !m_d->useWheel && key == m_d->key &&
        compareKeys(modifiers, m_d->modifiers);
}

bool KisKeyShortcut::matchKey(const QList<Qt::Key> &modifiers, WheelAction wheelAction)
{
    return m_d->useWheel && wheelAction == m_d->wheelAction &&
        compareKeys(modifiers, m_d->modifiers);
}
