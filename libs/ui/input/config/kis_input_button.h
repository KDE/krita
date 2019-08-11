/*
 * This file is part of the KDE project
 * Copyright (C) 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KISINPUTBUTTON_H
#define KISINPUTBUTTON_H

#include <QPushButton>

#include "input/kis_shortcut_configuration.h"

/**
 * \brief A button that can detect input and will store its value.
 *
 * This button, when pressed, will detect input based on what type has been set.
 * It is mainly intended for shortcut configuration, that is, picking some input that is
 * later reused for shortcuts or similar.
 *
 */
class KisInputButton : public QPushButton
{
    Q_OBJECT
public:
    /**
     * The type of button.
     */
    enum ButtonType {
        MouseType, ///< Detect and store any combination of pressed mouse buttons.
        KeyType, ///< Detect and store any combination of key presses.
        WheelType, ///< Detect and store mouse wheel movement.
    };

    /**
     * Constructor.
     */
    explicit KisInputButton(QWidget *parent = 0);
    /**
     * Destructor.
     */
    ~KisInputButton() override;

    /**
     * \return The type of input this button detects.
     */
    ButtonType type() const;
    /**
     * Set the type of input this button should detect.
     *
     * \param newType The type of input to detect.
     */
    void setType(ButtonType newType);

    /**
     * \return The list of keys that was detected. Only applicable when type is `KeyType`.
     */
    QList<Qt::Key> keys() const;
    /**
     * Set the list of keys to display.
     *
     * This is mostly intended to make sure the button displays the right keys when viewed
     * in a dialog or similar UI.
     *
     * Only applicable when type is `KeyType`.
     *
     * \param newKeys The list of keys to display.
     */
    void setKeys(const QList<Qt::Key> &newKeys);

    /**
     * \return The mouse buttons that were detected. Only applicable when type is `MouseType`.
     */
    Qt::MouseButtons buttons() const;
    /**
     * Set the mouse buttons to display.
     *
     * This is mostly intended to make sure the button displays the right buttons when viewed
     * in a dialog or similar UI.
     *
     * Only applicable when type is `MouseType`.
     *
     * \param newButtons The mouse buttons to display.
     */
    void setButtons(Qt::MouseButtons newButtons);

    /**
     * \return The mouse wheel movement that was detected. Only applicable when type is `WheelType`.
     */
    KisShortcutConfiguration::MouseWheelMovement wheel() const;
    /**
     * Set the mouse wheel movement to display.
     *
     * This is mostly intended to make sure the button displays the right wheel movement when
     * viewed in a dialog or similar UI.
     *
     * Only applicable when type is `WheelType`.
     *
     * \param wheel The wheel movement to display.
     */
    void setWheel(KisShortcutConfiguration::MouseWheelMovement wheel);

public Q_SLOTS:
    /**
     * Clear all detected input and reset the button to an empty state.
     */
    void clear();

Q_SIGNALS:
    /**
     * Emitted whenever one of the values (keys, buttons, wheel) changes.
     */
    void dataChanged();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private Q_SLOTS:
    void reset();

private:
    class Private;
    Private *const d;
};

#endif // KISINPUTBUTTON_H
