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

#include "kis_shortcut_configuration.h"

#include <QStringList>
#include <QKeySequence>
#include <KLocalizedString>

class KisShortcutConfiguration::Private
{
public:
    Private()
        : action(0),
          type(UnknownType),
          mode(0),
          wheel(NoMovement),
          gesture(NoGesture)
    { }

    KisAbstractInputAction *action;
    ShortcutType type;
    uint mode;

    QList<Qt::Key> keys;
    Qt::MouseButtons buttons;
    MouseWheelMovement wheel;
    GestureAction gesture;
};

KisShortcutConfiguration::KisShortcutConfiguration()
    : d(new Private)
{

}

KisShortcutConfiguration::KisShortcutConfiguration(const KisShortcutConfiguration &other)
    : d(new Private)
{
    d->action = other.action();
    d->type = other.type();
    d->mode = other.mode();
    d->keys = other.keys();
    d->buttons = other.buttons();
    d->wheel = other.wheel();
    d->gesture = other.gesture();
}

KisShortcutConfiguration::~KisShortcutConfiguration()
{
    delete d;
}

QString KisShortcutConfiguration::serialize()
{
    QString serialized("{");

    serialized.append(QString::number(d->mode, 16));
    serialized.append(';');
    serialized.append(QString::number(d->type, 16));
    serialized.append(";[");

    for (QList<Qt::Key>::iterator itr = d->keys.begin(); itr != d->keys.end(); ++itr) {
        serialized.append(QString::number(*itr, 16));

        if (itr + 1 != d->keys.end()) {
            serialized.append(',');
        }
    }

    serialized.append("];");

    serialized.append(QString::number(d->buttons, 16));
    serialized.append(';');
    serialized.append(QString::number(d->wheel, 16));
    serialized.append(';');
    serialized.append(QString::number(d->gesture, 16));
    serialized.append('}');

    return serialized;
}

bool KisShortcutConfiguration::unserialize(const QString &serialized)
{
    if (!serialized.startsWith('{'))
        return false;

    //Parse the serialized data and apply it to the current shortcut
    QString remainder = serialized;

    //Remove brackets
    remainder.remove('{').remove('}');

    //Split the remainder by ;
    QStringList parts = remainder.split(';');

    if (parts.size() < 6)
        return false; //Invalid input, abort

    //First entry in the list is the mode
    d->mode = parts.at(0).toUInt();

    //Second entry is the shortcut type
    d->type = static_cast<ShortcutType>(parts.at(1).toInt());

    if (d->type == UnknownType) {
        //Reject input that would set this shortcut to "Unknown"
        return false;
    }

    //Third entry is the list of keys
    QString serializedKeys = parts.at(2);
    //Remove brackets
    serializedKeys.remove('[').remove(']');
    //Split by , and add each entry as a key
    QStringList keylist = serializedKeys.split(',');
    Q_FOREACH(QString key, keylist) {
        if (!key.isEmpty()) {
            d->keys.append(static_cast<Qt::Key>(key.toUInt(0, 16)));
        }
    }

    //Fourth entry is the button mask
    d->buttons = static_cast<Qt::MouseButtons>(parts.at(3).toInt());
    d->wheel = static_cast<MouseWheelMovement>(parts.at(4).toUInt());
    d->gesture = static_cast<GestureAction>(parts.at(5).toUInt());

    return true;
}

KisAbstractInputAction *KisShortcutConfiguration::action() const
{
    return d->action;
}

void KisShortcutConfiguration::setAction(KisAbstractInputAction *newAction)
{
    if (d->action != newAction) {
        d->action = newAction;
    }
}

KisShortcutConfiguration::ShortcutType KisShortcutConfiguration::type() const
{
    return d->type;
}

void KisShortcutConfiguration::setType(KisShortcutConfiguration::ShortcutType newType)
{
    if (d->type != newType) {
        d->type = newType;
    }
}

uint KisShortcutConfiguration::mode() const
{
    return d->mode;
}

void KisShortcutConfiguration::setMode(uint newMode)
{
    if (d->mode != newMode) {
        d->mode = newMode;
    }
}

QList< Qt::Key > KisShortcutConfiguration::keys() const
{
    return d->keys;
}

void KisShortcutConfiguration::setKeys(const QList< Qt::Key > &newKeys)
{
    if (d->keys != newKeys) {
        d->keys = newKeys;
    }
}

Qt::MouseButtons KisShortcutConfiguration::buttons() const
{
    return d->buttons;
}

void KisShortcutConfiguration::setButtons(Qt::MouseButtons newButtons)
{
    if (d->buttons != newButtons) {
        d->buttons = newButtons;
    }
}

KisShortcutConfiguration::MouseWheelMovement KisShortcutConfiguration::wheel() const
{
    return d->wheel;
}

void KisShortcutConfiguration::setWheel(KisShortcutConfiguration::MouseWheelMovement type)
{
    if (d->wheel != type) {
        d->wheel = type;
    }
}

KisShortcutConfiguration::GestureAction KisShortcutConfiguration::gesture() const
{
    return d->gesture;
}

void KisShortcutConfiguration::setGesture(KisShortcutConfiguration::GestureAction type)
{
    if (d->gesture != type) {
        d->gesture = type;
    }
}

QString KisShortcutConfiguration::buttonsToText(Qt::MouseButtons buttons)
{
    QString text;

    int buttonCount = 0;

    if (buttons & Qt::LeftButton) {
        text.append(i18nc("Left Mouse Button", "Left"));
        buttonCount++;
    }

    if (buttons & Qt::RightButton) {
        if (buttonCount++ > 0) {
            text.append(" + ");
        }

        text.append(i18nc("Right Mouse Button", "Right"));
    }

    if (buttons & Qt::MidButton) {
        if (buttonCount++ > 0) {
            text.append(" + ");
        }

        text.append(i18nc("Middle Mouse Button", "Middle"));
    }

    if (buttons & Qt::XButton1) {
        if (buttonCount++ > 0) {
            text.append(" + ");
        }

        text.append(i18nc("Mouse Back Button", "Back"));
    }

    if (buttons & Qt::XButton1) {
        if (buttonCount++ > 0) {
            text.append(" + ");
        }

        text.append(i18nc("Mouse Forward Button", "Forward"));
    }

    if (buttonCount == 0) {
        text.append(i18nc("No input", "None"));
    }
    else {
        text.append(' ');
        text.append(i18ncp("Mouse Buttons", "Button", "Buttons", buttonCount));
    }

    return text;
}

QString KisShortcutConfiguration::keysToText(const QList<Qt::Key> &keys)
{
    QString output;

    foreach (Qt::Key key, keys) {
        if (output.size() > 0) {
            output.append(" + ");
        }

        switch (key) { //Because QKeySequence fails for Ctrl, Alt, Shift and Meta
        case Qt::Key_Control:
            output.append(i18nc("Ctrl key", "Ctrl"));
            break;

        case Qt::Key_Meta:
            output.append(i18nc("Meta key", "Meta"));
            break;

        case Qt::Key_Alt:
            output.append(i18nc("Alt key", "Alt"));
            break;

        case Qt::Key_Shift:
            output.append(i18nc("Shift key", "Shift"));
            break;

        default:
            QKeySequence s(key);
            output.append(s.toString(QKeySequence::NativeText));
            break;
        }

    }

    if (output.size() == 0) {
        output = i18nc("No input", "None");
    }

    return output;
}

QString KisShortcutConfiguration::wheelToText(KisShortcutConfiguration::MouseWheelMovement wheel)
{
    switch (wheel) {
    case KisShortcutConfiguration::WheelUp:
        return i18n("Mouse Wheel Up");
        break;

    case KisShortcutConfiguration::WheelDown:
        return i18n("Mouse Wheel Down");
        break;

    case KisShortcutConfiguration::WheelLeft:
        return i18n("Mouse Wheel Left");
        break;

    case KisShortcutConfiguration::WheelRight:
        return i18n("Mouse Wheel Right");
        break;

    default:
        return i18nc("No input", "None");
        break;
    }
}
