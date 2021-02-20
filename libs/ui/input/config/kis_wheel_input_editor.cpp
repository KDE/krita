/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_wheel_input_editor.h"

#include "ui_kis_wheel_input_editor.h"
#include <QMenu>
#include <QWidgetAction>
#include <QTimer>

#include "kis_icon_utils.h"

class KisWheelInputEditor::Private
{
public:
    Private() { }

    Ui::KisWheelInputEditor *ui;
};

KisWheelInputEditor::KisWheelInputEditor(QWidget *parent)
    : QPushButton(parent), d(new Private)
{
    QWidget *popup = new QWidget();

    d->ui = new Ui::KisWheelInputEditor;
    d->ui->setupUi(popup);
    d->ui->wheelButton->setType(KisInputButton::WheelType);

    d->ui->clearModifiersButton->setIcon(KisIconUtils::loadIcon("edit-clear"));
    d->ui->clearWheelButton->setIcon(KisIconUtils::loadIcon("edit-clear"));

    QWidgetAction *action = new QWidgetAction(this);
    action->setDefaultWidget(popup);

    QMenu *menu = new QMenu(this);
    menu->addAction(action);
    setMenu(menu);

    QTimer::singleShot(0, this, SLOT(showMenu()));

    connect(d->ui->wheelButton, SIGNAL(dataChanged()), SLOT(updateLabel()));
    connect(d->ui->modifiersButton, SIGNAL(dataChanged()), SLOT(updateLabel()));
    connect(d->ui->clearWheelButton, SIGNAL(clicked(bool)), d->ui->wheelButton, SLOT(clear()));
    connect(d->ui->clearModifiersButton, SIGNAL(clicked(bool)), d->ui->modifiersButton, SLOT(clear()));
}

KisWheelInputEditor::~KisWheelInputEditor()
{
    delete d->ui;
    delete d;
}

QList< Qt::Key > KisWheelInputEditor::keys() const
{
    return d->ui->modifiersButton->keys();
}

void KisWheelInputEditor::setKeys(const QList< Qt::Key > &newKeys)
{
    d->ui->modifiersButton->setKeys(newKeys);
    updateLabel();
}

KisShortcutConfiguration::MouseWheelMovement KisWheelInputEditor::wheel() const
{
    return d->ui->wheelButton->wheel();
}

void KisWheelInputEditor::setWheel(KisShortcutConfiguration::MouseWheelMovement newWheel)
{
    d->ui->wheelButton->setWheel(newWheel);
    updateLabel();
}

void KisWheelInputEditor::updateLabel()
{
    setText(KisShortcutConfiguration::wheelInputToText(
        d->ui->modifiersButton->keys(),
        d->ui->wheelButton->wheel()));
}
