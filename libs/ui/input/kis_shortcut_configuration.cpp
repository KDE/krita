/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_shortcut_configuration.h"

#include <QStringList>
#include <QKeySequence>
#include <KLocalizedString>

#include <boost/preprocessor/repeat_from_to.hpp>

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
    d->mode = parts.at(0).toUInt(nullptr, 16);

    //Second entry is the shortcut type
    d->type = static_cast<ShortcutType>(parts.at(1).toInt(nullptr, 16));

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
            d->keys.append(static_cast<Qt::Key>(key.toUInt(nullptr, 16)));
        }
    }

    //Fourth entry is the button mask
    d->buttons = static_cast<Qt::MouseButtons>(parts.at(3).toInt(nullptr, 16));
    d->wheel = static_cast<MouseWheelMovement>(parts.at(4).toUInt(nullptr, 16));
    d->gesture = static_cast<GestureAction>(parts.at(5).toUInt(nullptr, 16));

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
    QString sep = i18nc("Separator in the list of mouse buttons for shortcut", " + ");

    int buttonCount = 0;

    if (buttons & Qt::LeftButton) {
        text.append(i18nc("Left Mouse Button", "Left"));
        buttonCount++;
    }

    if (buttons & Qt::RightButton) {
        if (buttonCount++ > 0) {
            text.append(sep);
        }

        text.append(i18nc("Right Mouse Button", "Right"));
    }

    if (buttons & Qt::MiddleButton) {
        if (buttonCount++ > 0) {
            text.append(sep);
        }

        text.append(i18nc("Middle Mouse Button", "Middle"));
    }

    if (buttons & Qt::BackButton) {
        if (buttonCount++ > 0) {
            text.append(sep);
        }

        text.append(i18nc("Mouse Back Button", "Back"));
    }

    if (buttons & Qt::ForwardButton) {
        if (buttonCount++ > 0) {
            text.append(sep);
        }

        text.append(i18nc("Mouse Forward Button", "Forward"));
    }

    if (buttons & Qt::TaskButton) {
        if (buttonCount++ > 0) {
            text.append(sep);
        }

        text.append(i18nc("Mouse Task Button", "Task"));
    }

// Qt supports up to ExtraButton24 so include those
#define EXTRA_BUTTON(z, n, _)\
    if (buttons & Qt::ExtraButton##n) { \
        if (buttonCount++ > 0) { text.append(sep); } \
        text.append(i18nc("Mouse Button", "Mouse %1", n + 3)); \
    }
BOOST_PP_REPEAT_FROM_TO(4, 25, EXTRA_BUTTON, _)
#undef EXTRA_BUTTON

    if (buttonCount == 0) {
        text.append(i18nc("No mouse buttons for shortcut", "None"));
    }
    else {
        text = i18ncp(
            "%1 = List of mouse buttons for shortcut. "
            "Plural form is chosen upon the number of buttons in that list.",
            "%1 Button", "%1 Buttons", text, buttonCount);
    }

    return text;
}

QString KisShortcutConfiguration::keysToText(const QList<Qt::Key> &keys)
{
    QString output;

    Q_FOREACH (Qt::Key key, keys) {
        if (output.size() > 0) {
            output.append(i18nc("Separator in the list of keys for shortcut", " + "));
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
        output = i18nc("No keys for shortcut", "None");
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

    case KisShortcutConfiguration::WheelTrackpad:
        return i18n("Trackpad Pan");
        break;

    default:
        return i18nc("No mouse wheel buttons for shortcut", "None");
        break;
    }
}

QString KisShortcutConfiguration::buttonsInputToText(const QList<Qt::Key> &keys, Qt::MouseButtons buttons)
{
    QString buttonsText = KisShortcutConfiguration::buttonsToText(buttons);

    if (keys.size() > 0) {
        return i18nc(
            "%1 = modifier keys in shortcut; %2 = mouse buttons in shortcut",
            "%1 + %2",
            KisShortcutConfiguration::keysToText(keys),
            buttonsText);
    }
    else {
        return buttonsText;
    }
}

QString KisShortcutConfiguration::wheelInputToText(const QList<Qt::Key> &keys, KisShortcutConfiguration::MouseWheelMovement wheel)
{
    QString wheelText = KisShortcutConfiguration::wheelToText(wheel);

    if (keys.size() > 0) {
        return i18nc(
            "%1 = modifier keys in shortcut; %2 = mouse wheel buttons in shortcut",
            "%1 + %2",
            KisShortcutConfiguration::keysToText(keys),
            wheelText);
    }
    else {
        return wheelText;
    }
}
