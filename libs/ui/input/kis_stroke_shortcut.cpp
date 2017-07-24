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

#include <QMouseEvent>


class Q_DECL_HIDDEN KisStrokeShortcut::Private
{
public:
    QSet<Qt::Key> modifiers;
    QSet<Qt::MouseButton> buttons;
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
    int buttonScore = 0;
    Q_FOREACH (Qt::MouseButton button, m_d->buttons) {
        buttonScore += Qt::XButton2 - button;
    }

    return m_d->modifiers.size() * 0xFFFF + buttonScore * 0xFF + action()->priority();
}

void KisStrokeShortcut::setButtons(const QSet<Qt::Key> &modifiers,
                                   const QSet<Qt::MouseButton> &buttons)
{
    if (buttons.size() == 0) return;

    m_d->modifiers = modifiers;
    m_d->buttons = buttons;
}

bool KisStrokeShortcut::matchReady(const QSet<Qt::Key> &modifiers,
                                   const QSet<Qt::MouseButton> &buttons)
{
    bool modifiersOk =
        (m_d->modifiers.isEmpty() && action()->canIgnoreModifiers()) ||
        compareKeys(m_d->modifiers, modifiers);

    if (!modifiersOk || buttons.size() < m_d->buttons.size() - 1) {

        return false;
    }

    Q_FOREACH (Qt::MouseButton button, buttons) {
        if (!m_d->buttons.contains(button)) return false;
    }
    return true;
}

bool KisStrokeShortcut::matchBegin(Qt::MouseButton button)
{
    return m_d->buttons.contains(button);
}

QMouseEvent KisStrokeShortcut::fakeEndEvent(const QPointF &localPos) const
{
    Qt::MouseButton button = !m_d->buttons.isEmpty() ? *m_d->buttons.begin() : Qt::NoButton;
    return QMouseEvent(QEvent::MouseButtonRelease, localPos, button, Qt::NoButton, Qt::NoModifier);
}
