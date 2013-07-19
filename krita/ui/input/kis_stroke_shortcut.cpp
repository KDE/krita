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

#include "kis_stroke_shortcut.h"

#include "kis_abstract_input_action.h"

class KisStrokeShortcut::Private
{
public:
    QList<Qt::Key> modifiers;
    QList<Qt::MouseButton> buttons;
};


KisStrokeShortcut::KisStrokeShortcut(KisAbstractInputAction *action, int index)
    : KisAbstractShortcut(action, index),
      m_d(new Private)
{
}

KisStrokeShortcut::~KisStrokeShortcut()
{
    delete m_d;
}

int KisStrokeShortcut::priority() const
{
    return m_d->modifiers.size() * 2 + m_d->buttons.size() + action()->priority();
}

void KisStrokeShortcut::setButtons(const QList<Qt::Key> &modifiers,
                                   const QList<Qt::MouseButton> &buttons)
{
    Q_ASSERT(buttons.size() > 0);

    m_d->modifiers = modifiers;
    m_d->buttons = buttons;
}

bool KisStrokeShortcut::matchReady(const QList<Qt::Key> &modifiers,
                                   const QList<Qt::MouseButton> &buttons)
{
    if (!compareKeys(m_d->modifiers, modifiers) ||
        buttons.size() < m_d->buttons.size() - 1) {

        return false;
    }

    foreach(Qt::MouseButton button, buttons) {
        if (!m_d->buttons.contains(button)) return false;
    }
    return true;
}

bool KisStrokeShortcut::matchBegin(Qt::MouseButton button)
{
    return m_d->buttons.contains(button);
}
