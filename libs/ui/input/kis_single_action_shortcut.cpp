/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_single_action_shortcut.h"

#include <QKeySequence>
#include "kis_abstract_input_action.h"
#include <kis_assert.h>

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

bool KisSingleActionShortcut::conflictsWith(const QKeySequence &seq)
{
    if (seq.isEmpty()) return false;

    int seqMainKey = seq[0];
    QVector<int> sequenceKeys;

    if (seqMainKey & Qt::MetaModifier) {
        sequenceKeys.append(Qt::Key_Meta);
        seqMainKey &= ~Qt::MetaModifier;
    } else if (seqMainKey & Qt::ControlModifier) {
        sequenceKeys.append(Qt::Key_Control);
        seqMainKey &= ~Qt::ControlModifier;
    } else if (seqMainKey & Qt::ShiftModifier) {
        sequenceKeys.append(Qt::Key_Shift);
        seqMainKey &= ~Qt::ShiftModifier;
    } else if (seqMainKey & Qt::AltModifier) {
        sequenceKeys.append(Qt::Key_Alt);
        seqMainKey &= ~Qt::AltModifier;
    }

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(seqMainKey != 0, false);
    sequenceKeys.append(seqMainKey);
    std::sort(sequenceKeys.begin(), sequenceKeys.end());

    QVector<int> shortcutKeys;
    std::copy(m_d->modifiers.begin(), m_d->modifiers.end(), std::back_inserter(shortcutKeys));
    shortcutKeys.append(m_d->key);
    std::sort(shortcutKeys.begin(), shortcutKeys.end());

    return
        std::includes(sequenceKeys.begin(), sequenceKeys.end(),
                      shortcutKeys.begin(), shortcutKeys.end()) ||
        std::includes(shortcutKeys.begin(), shortcutKeys.end(),
                      sequenceKeys.begin(), sequenceKeys.end());
}
