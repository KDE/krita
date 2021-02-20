/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_input_configuration_page.h"
#include "ui_kis_input_configuration_page.h"

#include "input/kis_input_profile_manager.h"
#include "input/kis_input_profile.h"
#include "kis_edit_profiles_dialog.h"
#include "kis_input_profile_model.h"
#include "kis_input_configuration_page_item.h"
#include <QDir>
#include <KoResourcePaths.h>


#include "kis_icon_utils.h"

KisInputConfigurationPage::KisInputConfigurationPage(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    ui = new Ui::KisInputConfigurationPage;
    this->setContentsMargins(0,0,0,0);
    ui->setupUi(this);

    ui->profileComboBox->setModel(new KisInputProfileModel(ui->profileComboBox));
    updateSelectedProfile();
    connect(ui->profileComboBox, SIGNAL(currentIndexChanged(QString)), SLOT(changeCurrentProfile(QString)));

    ui->editProfilesButton->setIcon(KisIconUtils::loadIcon("document-edit"));

    connect(ui->editProfilesButton, SIGNAL(clicked(bool)), SLOT(editProfilesButtonClicked()));
    connect(KisInputProfileManager::instance(), SIGNAL(profilesChanged()), SLOT(updateSelectedProfile()));

    QList<KisAbstractInputAction *> actions = KisInputProfileManager::instance()->actions();
    Q_FOREACH(KisAbstractInputAction * action, actions) {
        KisInputConfigurationPageItem *item = new KisInputConfigurationPageItem(this);
        item->setAction(action);
        ui->configurationItemsArea->setSpacing(0);
        ui->configurationItemsArea->addWidget(item);
    }
    ui->configurationItemsArea->addStretch(20); // ensures listed input are on top

    QScroller *scroller = KisKineticScroller::createPreconfiguredScroller(ui->scrollArea);
    if (scroller) {
        connect(scroller, SIGNAL(stateChanged(QScroller::State)),
                this, SLOT(slotScrollerStateChanged(QScroller::State)));
    }
}

void KisInputConfigurationPage::saveChanges()
{
    KisInputProfileManager::instance()->saveProfiles();
}

void KisInputConfigurationPage::revertChanges()
{
    KisInputProfileManager::instance()->loadProfiles();
}

void KisInputConfigurationPage::setDefaults()
{
    QDir profileDir(KoResourcePaths::saveLocation("data", "input/", false));

    if (profileDir.exists()) {
        QStringList entries = profileDir.entryList(QStringList() << "*.profile", QDir::NoDot | QDir::NoDotDot);
        Q_FOREACH(const QString & file, entries) {
            profileDir.remove(file);
        }

        KisInputProfileManager::instance()->loadProfiles();
    }
}

void KisInputConfigurationPage::editProfilesButtonClicked()
{
    KisEditProfilesDialog dialog;
    dialog.exec();
}

void KisInputConfigurationPage::updateSelectedProfile()
{
    if (KisInputProfileManager::instance()->currentProfile()) {
        ui->profileComboBox->setCurrentItem(KisInputProfileManager::instance()->currentProfile()->name());
    }
}

void KisInputConfigurationPage::changeCurrentProfile(const QString &newProfile)
{
    KisInputProfileManager::instance()->setCurrentProfile(KisInputProfileManager::instance()->profile(newProfile));
}
