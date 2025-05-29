/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISWAYLANDKEYBOARDWATCHER_H
#define KISWAYLANDKEYBOARDWATCHER_H

#include <memory>
#include <QList>

class KisWaylandKeyboardWatcher
{
public:
    KisWaylandKeyboardWatcher();
    ~KisWaylandKeyboardWatcher();

    bool hasKeyboardFocus() const;
    QList<Qt::Key> pressedKeys() const;
    Qt::KeyboardModifiers modifiers() const;

private:
    class Seat;
    class Keyboard;

    std::unique_ptr<Seat> m_seat;
};




#endif /* KISWAYLANDKEYBOARDWATCHER_H */