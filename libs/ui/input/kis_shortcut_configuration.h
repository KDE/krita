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

#ifndef KISSHORTCUTCONFIGURATION_H
#define KISSHORTCUTCONFIGURATION_H

#include <QList>
#include <QMetaType>

class QString;
class KisAbstractInputAction;

/**
 * \brief A class encapsulating all settings for a single shortcut.
 *
 * This class encapsulates mouse buttons, keyboard keys and other settings
 * related to a single shortcut for a single action.
 *
 * \note Each action can have several modes that activate it with usually
 * different behaviour for each mode. Different shortcuts can activate
 * different modes.
 */
class KisShortcutConfiguration
{
public:
    /**
     * The type of shortcut, i.e. what kind of input does it expect.
     */
    enum ShortcutType {
        UnknownType, ///< Unknown, empty shortcut.
        KeyCombinationType, ///< A list of keys that should be pressed.
        MouseButtonType, ///< A mouse button, possibly with key modifiers.
        MouseWheelType, ///< Mouse wheel movement, possibly with key modifiers.
        GestureType, ///< A touch gesture.
    };

    /**
     * The type of mouse wheel movement.
     */
    enum MouseWheelMovement {
        NoMovement, ///< No movement.
        WheelUp, ///< Upwards movement, away from the user.
        WheelDown, ///< Downwards movement, toward the user.
        WheelLeft, ///< Left movement.
        WheelRight, ///< Right movement.
        WheelTrackpad, ///< A pan movement on a trackpad.
    };

    /**
     * The type of gesture.
     */
    enum GestureAction {
        NoGesture, ///< No gesture.
        PinchGesture, ///< Pinch gesture, fingers moving towards or away from each other.
        PanGesture, ///< Pan gesture, fingers staying together but moving across the screen.
        RotateGesture, ///<Rotate gesture, two fingers rotating around a pivot point.
        SmartZoomGesture, ///< Smart zoom gesture, typically a double tap that is a boolean zoom/unzoom.
        ZoomAndRotateGesture, ///< Zoom and rotate gesture, canvas being both zoomed and rotated using two fingers.
    };

    /**
     * Constructor.
     */
    KisShortcutConfiguration();
    /**
     * Copy constructor.
     */
    KisShortcutConfiguration(const KisShortcutConfiguration &other);
    /**
     * Destructor.
     */
    virtual ~KisShortcutConfiguration();

    /**
     * Serialize the data of this shortcut into a string that can be saved into
     * a configuration file.
     *
     * The string will have the following format:
     *
     *     {mode;type;[key,key];buttons;wheel;gesture}
     *
     * with each property serialized into a base-16 integer.
     *
     * \return A serialized representation of this shortcut.
     */
    QString serialize();
    /**
     * Apply the data from a serialized shortcut to this shortcut.
     *
     * This method expects a string as described in serialize().
     *
     * \param serialized The serialized shortcut.
     *
     * \return true if successful, false if an error occurred.
     *
     * \sa serialize()
     */
    bool unserialize(const QString &serialized);

    /**
     * \return The action this shortcut is associated with.
     */
    KisAbstractInputAction *action() const;
    /**
     * Set the action this shortcut should be associated with.
     *
     * \param newAction The action to set.
     */
    void setAction(KisAbstractInputAction *newAction);

    /**
     * \return The type of shortcut.
     */
    ShortcutType type() const;
    /**
     * Set the type of shortcut.
     *
     * \param newType The type to set.
     */
    void setType(ShortcutType newType);

    /**
     * \return The mode of the action this shortcut will trigger.
     */
    uint mode() const;
    /**
     * Set the mode of the action this shortcut will trigger.
     *
     * \param newMode The mode to set.
     */
    void setMode(uint newMode);

    /**
     * \return The list of keys that will trigger this shortcut.
     *
     * \note Not applicable when type is GestureType.
     */
    QList<Qt::Key> keys() const;
    /**
     * Set the list of keys that will trigger this shortcut.
     *
     * \param newKeys The list of keys to use.
     *
     * \note Not applicable when type is GestureType.
     */
    void setKeys(const QList<Qt::Key> &newKeys);

    /**
     * \return The mouse buttons that will trigger this shortcut.
     *
     * \note Only applicable when type is MouseButtonType.
     */
    Qt::MouseButtons buttons() const;
    /**
     * Set the mouse buttons that will trigger this shortcut.
     *
     * \param newButtons The mouse buttons to use.
     *
     * \note Only applicable when type is MouseButtonType.
     */
    void setButtons(Qt::MouseButtons newButtons);

    /**
     * \return The mouse wheel movement that will trigger this shortcut.
     *
     * \note Only applicable when type is MouseWheelType.
     */
    MouseWheelMovement wheel() const;
    /**
     * Set the mouse wheel movement that will trigger this shortcut.
     *
     * \param type The wheel movement to use.
     *
     * \note Only applicable when type is MouseWheelType.
     */
    void setWheel(MouseWheelMovement type);

    /**
     * \return The gesture that will trigger this shortcut.
     *
     * \note Only applicable when type is GestureType.
     */
    GestureAction gesture() const;
    /**
     * Set the gesture that will trigger this shortcut.
     *
     * \param type The gesture to use.
     *
     * \note Only applicable when type is GestureType.
     */
    void setGesture(GestureAction type);
    /**
     * Convert a set of mouse buttons into a user-readable
     * string.
     *
     * This will convert the given set of buttons into a
     * string that can be shown to a user. For example, the
     * combination Qt::LeftButton + Qt::RightButton will produce
     * the string "Left + Right Button".
     *
     * \param buttons The buttons to convert.
     *
     * \return A string representing the buttons that can be shown
     * to a user.
     *
     * \note An empty set will produce the string "None".
     */
    static QString buttonsToText(Qt::MouseButtons buttons);
    /**
     * Convert a list of keys to a user-readable string.
     *
     * This will convert the given list of keys into a string
     * that can be shown to a user. For example, the list
     * [Qt::Key_Shift, Qt::Key_Space] will produce the string
     * "Shift + Space".
     *
     * \param keys The keys to convert.
     *
     * \return A string representing the keys that can be shown
     * to a user.
     *
     * \note An empty list will produce the string "None".
     */
    static QString keysToText(const QList<Qt::Key> &keys);
    /**
     * Convert the given mouse wheel movement to a string.
     *
     * This will convert the given mouse wheel movement into a
     * string that can be shown to a user. For example, WheelUp
     * will produce the string "Mouse Wheel Up".
     *
     * \param wheel The mouse wheel movement to convert.
     *
     * \return A string representing the mouse wheel movement
     * that can be shown to a user.
     *
     * \note NoMovement will produce the string "None".
     */
    static QString wheelToText(MouseWheelMovement wheel);
    /**
     * Convert a shortcut build of a set of keys and a set of mouse
     * buttons into a user-readable string.
     *
     * This will convert the given mouse buttons-based shortcut into a
     * string that can be shown to a user. For example, the combination
     * of Qt::Key_Control and Qt::LeftButton + Qt::RightButton will
     * produce the string "Ctrl + Left + Right Button".
     *
     * \param keys The keys to convert.
     * \param buttons The mouse buttons to convert.
     *
     * \return A string representing the shortcut that can be shown
     * to a user.
     *
     * \note An empty set of buttons will appear as the string "None".
     */
    static QString buttonsInputToText(const QList<Qt::Key> &keys, Qt::MouseButtons buttons);
    /**
     * Convert a shortcut build of a set of keys and a set of mouse
     * wheel buttons into a user-readable string.
     *
     * This will convert the given mouse wheel-based shortcut into a
     * string that can be shown to a user. For example, the combination
     * of Qt::Key_Control and WheelUp will produce the string
     * "Ctrl + Mouse Wheel Up".
     *
     * \param keys The keys to convert.
     * \param wheel The mouse wheel buttons to convert.
     *
     * \return A string representing the shortcut that can be shown
     * to a user.
     *
     * \note An empty set of wheel buttons will appear as
     * the string "None".
     */
    static QString wheelInputToText(const QList<Qt::Key> &keys, MouseWheelMovement wheel);

private:
    class Private;
    Private *const d;
};

Q_DECLARE_METATYPE(KisShortcutConfiguration *);

#endif // KISSHORTCUTCONFIGURATION_H
