/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_CUSTOM_MODIFIERS_CATCHER_H
#define __KIS_CUSTOM_MODIFIERS_CATCHER_H

#include <QScopedPointer>
#include <QObject>

/**
 * @brief The KisCustomModifiersCatcher class is a special utility class that
 * tracks custom modifiers pressed. Its main purpose is to avoid manual tracking
 * of KeyPress/KeyRelease/FocusIn events in the class and reuse the common code in
 * multiple widgets.
 *
 * ~~~~~~~~~~~~~~~~~~~~{.cpp}
 * // in the c-tor of your widget create the catcher, it will automatically
 * // connect to the passed parent
 * KisCustomModifiersCatcher *catcher = new KisCustomModifiersCatcher(parent);
 *
 * // Register a tracked modifier
 * catcher->addModifier("pan-zoom", Qt::Key_Space);
 *
 * // in the pointer tracking event handlers just check
 * // if the modifier is pressed or not
 * bool isPressed = catcher->modifierPressed("pan-zoom");
 * ~~~~~~~~~~~~~~~~~~~~
 */

class KisCustomModifiersCatcher : public QObject
{
public:
    /**
     * Create the catcher and connect to the passed widget/object to
     * track its key events
     */
    KisCustomModifiersCatcher(QObject *parent);
    ~KisCustomModifiersCatcher() override;

    bool eventFilter(QObject* object, QEvent* event) override;

    /**
     * @brief addModifier registers a custom modifier
     * @param id a unique id string associated with the modifier. Later, you will use this string to fetch the modifier state.
     * @param modifier the key to track
     */
    void addModifier(const QString &id, Qt::Key modifier);

    /**
     * @brief modifierPressed returns the state of the tracked modifier
     */
    bool modifierPressed(const QString &id);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_CUSTOM_MODIFIERS_CATCHER_H */
