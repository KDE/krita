/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_input_button.h"

#include <QTimer>
#include <QMouseEvent>
#include <QKeyEvent>
#include <KLocalizedString>
#include <QPushButton>

#include "kis_icon_utils.h"


class KisInputButton::Private
{
public:
    Private(KisInputButton *qq) : q(qq), type(KeyType), newInput(false), resetTimer(0) { }
    void updateLabel();

    KisInputButton *q;

    ButtonType type;

    QList<Qt::Key> keys;
    Qt::MouseButtons buttons;
    KisShortcutConfiguration::MouseWheelMovement wheel;
    bool newInput;

    QTimer *resetTimer;
};

KisInputButton::KisInputButton(QWidget *parent)
    : QPushButton(parent), d(new Private(this))
{
    setIcon(KisIconUtils::loadIcon("configure"));
    setText(i18nc("No input for this button", "None"));
    setCheckable(true);

    d->resetTimer = new QTimer(this);
    d->resetTimer->setInterval(5000);
    d->resetTimer->setSingleShot(true);
    connect(d->resetTimer, SIGNAL(timeout()), SLOT(reset()));
}

KisInputButton::~KisInputButton()
{
    delete d;
}

KisInputButton::ButtonType KisInputButton::type() const
{
    return d->type;
}

void KisInputButton::setType(KisInputButton::ButtonType newType)
{
    d->type = newType;
}

void KisInputButton::clear()
{
    d->keys.clear();
    d->buttons = QFlags<Qt::MouseButton>();
    d->wheel = KisShortcutConfiguration::NoMovement;
    d->updateLabel();
}

QList< Qt::Key > KisInputButton::keys() const
{
    return d->keys;
}

void KisInputButton::setKeys(const QList< Qt::Key > &newKeys)
{
    if (newKeys != d->keys) {
        d->keys = newKeys;
        d->updateLabel();
    }
}

Qt::MouseButtons KisInputButton::buttons() const
{
    return d->buttons;
}

void KisInputButton::setButtons(Qt::MouseButtons newButtons)
{
    if (newButtons != d->buttons) {
        d->buttons = newButtons;
        d->updateLabel();
    }
}

KisShortcutConfiguration::MouseWheelMovement KisInputButton::wheel() const
{
    return d->wheel;
}

void KisInputButton::setWheel(KisShortcutConfiguration::MouseWheelMovement newWheel)
{
    if (newWheel != d->wheel) {
        d->wheel = newWheel;
        d->updateLabel();
    }
}

void KisInputButton::mousePressEvent(QMouseEvent *event)
{
    if (isChecked()  && d->type == KisInputButton::MouseType) {
        d->buttons = event->buttons();
        d->updateLabel();
        d->resetTimer->start();
    }
}

void KisInputButton::mouseReleaseEvent(QMouseEvent *)
{
    if (isChecked()) {
        reset();
    }
    else {
        setChecked(true);
        setText(i18nc("Waiting for user input", "Input..."));
        d->resetTimer->start();
        d->newInput = true;
    }
}

void KisInputButton::wheelEvent(QWheelEvent *event)
{
    if (isChecked() && event->delta() != 0) {
        switch (event->orientation()) {
        case Qt::Horizontal:
            if (event->delta() < 0) {
                d->wheel = KisShortcutConfiguration::WheelRight;
            }
            else {
                d->wheel = KisShortcutConfiguration::WheelLeft;
            }

            break;

        case Qt::Vertical:
            if (event->delta() > 0) {
                d->wheel = KisShortcutConfiguration::WheelUp;
            }
            else {
                d->wheel = KisShortcutConfiguration::WheelDown;
            }

            break;
        }

        d->updateLabel();
    }
}

void KisInputButton::keyPressEvent(QKeyEvent *event)
{
    if (isChecked()) {
        if (d->newInput) {
            d->keys.clear();
            d->newInput = false;
        }

        Qt::Key key = static_cast<Qt::Key>(event->key());

        if (key == Qt::Key_Meta && event->modifiers().testFlag(Qt::ShiftModifier)) {
            key = Qt::Key_Alt;
        }

        d->keys.append(key);
        d->updateLabel();
        d->resetTimer->start();
    }
}

void KisInputButton::keyReleaseEvent(QKeyEvent *event)
{
    if (isChecked()) {
        reset();
    }
    else if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        setChecked(true);
        setText(i18nc("Waiting for user input", "Input..."));
        d->resetTimer->start();
        d->newInput = true;
    }
}

void KisInputButton::reset()
{
    setChecked(false);
    d->updateLabel();
    emit dataChanged();
}

void KisInputButton::Private::updateLabel()
{
    switch (type) {
    case MouseType:
        q->setText(KisShortcutConfiguration::buttonsToText(buttons));
        break;

    case KeyType:
        q->setText(KisShortcutConfiguration::keysToText(keys));
        break;

    case WheelType:
        q->setText(KisShortcutConfiguration::wheelToText(wheel));
        break;
    }
}
