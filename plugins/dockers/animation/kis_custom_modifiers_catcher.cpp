/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_custom_modifiers_catcher.h"

#include <QSet>

#include "input/kis_extended_modifiers_mapper.h"
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>


struct KisCustomModifiersCatcher::Private
{
    Private(QObject *_trackedObject) : trackedObject(_trackedObject) {}

    QObject *trackedObject;

    QSet<Qt::Key> trackedKeys;
    QHash<QString, Qt::Key> idToKeyMap;
    QSet<Qt::Key> pressedKeys;

    void reset() {
        // something went wrong!
        pressedKeys.clear();
    }
};


KisCustomModifiersCatcher::KisCustomModifiersCatcher(QObject *parent)
    : QObject(parent),
      m_d(new Private(parent))
{
    if (m_d->trackedObject) {
        parent->installEventFilter(this);
    }
}

KisCustomModifiersCatcher::~KisCustomModifiersCatcher()
{
}

void KisCustomModifiersCatcher::addModifier(const QString &id, Qt::Key modifier)
{
    m_d->idToKeyMap.insert(id, modifier);
    m_d->trackedKeys.insert(modifier);

    m_d->reset();
}

bool KisCustomModifiersCatcher::modifierPressed(const QString &id)
{
    if (!m_d->idToKeyMap.contains(id)) {
        qWarning() << "KisCustomModifiersCatcher::modifierPressed(): unexpected modifier id:" << id;
        return false;
    }
    return m_d->pressedKeys.contains(m_d->idToKeyMap[id]);
}

bool KisCustomModifiersCatcher::eventFilter(QObject* object, QEvent* event)
{
    if (object != m_d->trackedObject) return false;

    switch (event->type()) {
    case QEvent::ShortcutOverride: {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        if (keyEvent->isAutoRepeat()) break;

        Qt::Key key = KisExtendedModifiersMapper::workaroundShiftAltMetaHell(keyEvent);

        if (m_d->trackedKeys.contains(key)) {
            if (m_d->pressedKeys.contains(key)) {
                m_d->reset();
            } else {
                m_d->pressedKeys.insert(key);
            }
        }
        break;
    }
    case QEvent::KeyRelease: {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        if (keyEvent->isAutoRepeat()) break;

        Qt::Key key = KisExtendedModifiersMapper::workaroundShiftAltMetaHell(keyEvent);

        if (m_d->trackedKeys.contains(key)) {
            if (!m_d->pressedKeys.contains(key)) {
                m_d->reset();
            } else {
                m_d->pressedKeys.remove(key);
            }
        }

        break;
    }
    case QEvent::FocusIn: {
        m_d->reset();

        { // Emulate pressing of the key that are already pressed
            KisExtendedModifiersMapper mapper;

            Qt::KeyboardModifiers modifiers = mapper.queryStandardModifiers();
            Q_FOREACH (Qt::Key key, mapper.queryExtendedModifiers()) {
                QKeyEvent kevent(QEvent::ShortcutOverride, key, modifiers);
                eventFilter(object, &kevent);
            }
        }
    }
    default:
        break;
    }

    return false;
}
