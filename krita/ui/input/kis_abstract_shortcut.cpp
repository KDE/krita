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

#include "kis_abstract_shortcut.h"

struct KisAbstractShortcut::Private
{
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

void KisAbstractShortcut::setShortcutIndex(int index)
{
    m_d->shortcutIndex = index;
}

bool KisAbstractShortcut::compareKeys(const QList<Qt::Key> &keys1,
                                      const QList<Qt::Key> &keys2)
{
    if (keys1.size() != keys2.size()) return false;

    foreach(Qt::Key key, keys1) {
        if (!keys2.contains(key)) return false;
    }
    return true;
}
