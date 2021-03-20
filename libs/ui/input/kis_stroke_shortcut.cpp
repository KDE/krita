/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_stroke_shortcut.h"

#include "kis_abstract_input_action.h"

#include <QMouseEvent>

#include <cmath>

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
    const int maxScore = std::log2((int) Qt::MaxMouseButton);
    int buttonScore = 0;
    Q_FOREACH (Qt::MouseButton button, m_d->buttons) {
        buttonScore += maxScore - std::log2((int) button);
    }

    return m_d->modifiers.size() * 0xFFFF + buttonScore * 0xFF + action()->priority();
}

void KisStrokeShortcut::setButtons(const QSet<Qt::Key> &modifiers,
                                   const QSet<Qt::MouseButton> &buttons)
{
    if (buttons.empty()) return;

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
