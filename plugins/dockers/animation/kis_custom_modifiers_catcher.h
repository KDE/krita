/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
