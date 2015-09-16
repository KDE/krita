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
