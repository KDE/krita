/*
 *  SPDX-FileCopyrightText: 2018 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisNewWindowLayoutDialog.h"

KisNewWindowLayoutDialog::KisNewWindowLayoutDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void KisNewWindowLayoutDialog::setName(const QString &name)
{
    nameInput->setText(name);
}

QString KisNewWindowLayoutDialog::name() const
{
    return nameInput->text();
}

bool KisNewWindowLayoutDialog::showImageInAllWindows() const
{
    return chkActiveInAllWindows->isChecked();
}

bool KisNewWindowLayoutDialog::primaryWorkspaceFollowsFocus() const
{
    return chkFollowFocus->isChecked();
}
