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

#include "kis_edit_profiles_dialog.h"

#include <QPushButton>
#include <QStringListModel>
#include <KLocalizedString>

#include "KoIcon.h"
#include "input/kis_input_profile_manager.h"
#include "kis_input_profile_model.h"

#include "ui_kis_edit_profiles_dialog.h"

class KisEditProfilesDialog::Private
{
public:
    Private() { }

    Ui::KisEditProfilesDialog *ui;
    KisInputProfileModel *profileModel;
};

KisEditProfilesDialog::KisEditProfilesDialog(QWidget *parent, Qt::WindowFlags flags)
    : KDialog(parent, flags), d(new Private())
{
    QWidget *mainWidget = new QWidget(this);
    d->ui = new Ui::KisEditProfilesDialog();
    d->ui->setupUi(mainWidget);
    setMainWidget(mainWidget);

    d->profileModel = new KisInputProfileModel(this);
    d->ui->profileList->setModel(d->profileModel);

    d->ui->addButton->setIcon(koIcon("list-add"));
    d->ui->removeButton->setIcon(koIcon("list-remove"));
    d->ui->duplicateButton->setIcon(koIcon("edit-copy"));
    d->ui->renameButton->setIcon(koIcon("edit-rename"));

    connect(d->ui->addButton, SIGNAL(clicked(bool)), SLOT(addButtonClicked()));
    connect(d->ui->removeButton, SIGNAL(clicked(bool)), SLOT(removeButtonClicked()));
    connect(d->ui->duplicateButton, SIGNAL(clicked(bool)), SLOT(duplicateButtonClicked()));
    connect(d->ui->renameButton, SIGNAL(clicked(bool)), SLOT(renameButtonClicked()));

    setButtons(Close | Default);
    setWindowTitle(i18n("Edit Profiles"));
}

KisEditProfilesDialog::~KisEditProfilesDialog()
{
    delete d;
}

void KisEditProfilesDialog::addButtonClicked()
{
    QString newProfileName = i18n("New Profile");
    KisInputProfileManager::instance()->addProfile(newProfileName);
    d->ui->profileList->edit(d->profileModel->find(newProfileName));
}

void KisEditProfilesDialog::removeButtonClicked()
{
    KisInputProfileManager::instance()->removeProfile(d->profileModel->profileName(d->ui->profileList->currentIndex()));
}

void KisEditProfilesDialog::duplicateButtonClicked()
{
    QString currentName = d->profileModel->profileName(d->ui->profileList->currentIndex());
    QString newName = i18n("Copy of %1").arg(currentName);
    KisInputProfileManager::instance()->duplicateProfile(currentName, newName);
}

void KisEditProfilesDialog::renameButtonClicked()
{
    d->ui->profileList->edit(d->ui->profileList->currentIndex());
}
