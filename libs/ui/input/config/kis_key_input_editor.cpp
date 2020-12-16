/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_key_input_editor.h"

#include <QWidgetAction>
#include <QMenu>
#include <QTimer>

#include "kis_icon_utils.h"

#include "ui_kis_key_input_editor.h"

class KisKeyInputEditor::Private
{
public:
    Private() { }

    Ui::KisKeyInputEditor *ui;
};

KisKeyInputEditor::KisKeyInputEditor(QWidget *parent)
    : QPushButton(parent), d(new Private)
{
    QWidget *popup = new QWidget();

    d->ui = new Ui::KisKeyInputEditor;
    d->ui->setupUi(popup);

    d->ui->clearKeysButton->setIcon(KisIconUtils::loadIcon("edit-clear"));

    QWidgetAction *action = new QWidgetAction(this);
    action->setDefaultWidget(popup);

    QMenu *menu = new QMenu(this);
    menu->addAction(action);
    setMenu(menu);

    QTimer::singleShot(0, this, SLOT(showMenu()));

    connect(d->ui->keysButton, SIGNAL(dataChanged()), SLOT(updateLabel()));
    connect(d->ui->clearKeysButton, SIGNAL(clicked(bool)), d->ui->keysButton, SLOT(clear()));
}

KisKeyInputEditor::~KisKeyInputEditor()
{
    delete d->ui;
    delete d;
}

QList< Qt::Key > KisKeyInputEditor::keys() const
{
    return d->ui->keysButton->keys();
}

void KisKeyInputEditor::setKeys(const QList< Qt::Key > &newKeys)
{
    d->ui->keysButton->setKeys(newKeys);
    updateLabel();
}

void KisKeyInputEditor::updateLabel()
{
    setText(KisShortcutConfiguration::keysToText(d->ui->keysButton->keys()));
}
