/* This file is part of the KDE project
   Copyright (C) 2008 Fela Winkelmolen <fela.kde@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KarbonCalligraphyOptionWidget.h"

#include <KoIcon.h>

#include <klocalizedstring.h>
#include <kcombobox.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <kconfig.h>
#include <QDebug>
#include <kmessagebox.h>

#include <QInputDialog>
#include <QSpinBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QGridLayout>
#include <QToolButton>

#include "kis_double_parse_spin_box.h"
#include "kis_int_parse_spin_box.h"

/*
Profiles are saved in karboncalligraphyrc

In the group "General", profile is the name of profile used

Every profile is described in a group, the name of which is "ProfileN"
Starting to count from 0 onwards
(NOTE: the index in profiles is different from the N)

Default profiles are added by the function addDefaultProfiles(), once they
have been added, the entry defaultProfilesAdded in the "General" group is
set to true

TODO: add a reset defaults option?
*/

// name of the configuration file
const QString RCFILENAME = "karboncalligraphyrc";

KarbonCalligraphyOptionWidget::KarbonCalligraphyOptionWidget()
    : m_changingProfile(false)
{
    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_comboBox = new KComboBox(this);
    layout->addWidget(m_comboBox, 0, 0);

    m_saveButton = new QToolButton(this);
    m_saveButton->setToolTip(i18n("Save profile as..."));
    m_saveButton->setIcon(koIcon("document-save-as"));
    layout->addWidget(m_saveButton, 0, 1);

    m_removeButton = new QToolButton(this);
    m_removeButton->setToolTip(i18n("Remove profile"));
    m_removeButton->setIcon(koIcon("list-remove"));
    layout->addWidget(m_removeButton, 0, 2);

    QGridLayout *detailsLayout = new QGridLayout();
    detailsLayout->setContentsMargins(0, 0, 0, 0);
    detailsLayout->setVerticalSpacing(0);

    m_usePath = new QCheckBox(i18n("&Follow selected path"), this);
    detailsLayout->addWidget(m_usePath, 0, 0, 1, 4);

    QLabel *smoothDistance = new QLabel(i18n("Dist-I:"), this);
    smoothDistance->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_smoothDistance = new KisDoubleParseSpinBox(this);
    m_smoothDistance->setToolTip(i18n("The Distance interval samples based on the length of the stroke in the coordinates of the screen. So if you zoom in, you can make preciser strokes despite having the same distance interval."));
    m_smoothDistance->setRange(1, 1000);
    m_smoothDistance->setSingleStep(1);
    smoothDistance->setBuddy(m_smoothDistance);
    detailsLayout->addWidget(smoothDistance, 4, 0);
    detailsLayout->addWidget(m_smoothDistance, 4, 1);

    QLabel *smoothTime = new QLabel(i18n("Time-I:"), this);
    smoothTime->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_smoothTime = new KisDoubleParseSpinBox(this);
    m_smoothTime->setRange(1, 1000);
    m_smoothTime->setSingleStep(1);
    smoothTime->setBuddy(m_smoothTime);
    detailsLayout->addWidget(smoothTime, 4, 2);
    detailsLayout->addWidget(m_smoothTime, 4, 3);

    QLabel *capsLabel = new QLabel(i18n("Caps:"), this);
    capsLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_capsBox = new KisDoubleParseSpinBox(this);
    m_capsBox->setRange(0.0, 2.0);
    m_capsBox->setSingleStep(0.03);
    capsLabel->setBuddy(m_capsBox);
    detailsLayout->addWidget(capsLabel, 5, 2);
    detailsLayout->addWidget(m_capsBox, 5, 3);

    QLabel *massLabel = new QLabel(i18n("Mass:"), this);
    massLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_massBox = new KisDoubleParseSpinBox(this);
    m_massBox->setRange(0.0, 20.0);
    m_massBox->setDecimals(1);
    massLabel->setBuddy(m_massBox);
    detailsLayout->addWidget(massLabel, 6, 0);
    detailsLayout->addWidget(m_massBox, 6, 1);

    QLabel *dragLabel = new QLabel(i18n("Drag:"), this);
    dragLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_dragBox = new KisDoubleParseSpinBox(this);
    m_dragBox->setRange(0.0, 1.0);
    m_dragBox->setSingleStep(0.1);
    dragLabel->setBuddy(m_dragBox);
    detailsLayout->addWidget(dragLabel, 6, 2);
    detailsLayout->addWidget(m_dragBox, 6, 3);

    layout->addLayout(detailsLayout, 1, 0, 1, 3);
    layout->setRowStretch(2, 1);

    createConnections();
    addDefaultProfiles(); // if they are already added does nothing
    loadProfiles();
}

KarbonCalligraphyOptionWidget::~KarbonCalligraphyOptionWidget()
{
    qDeleteAll(m_profiles);
}

void KarbonCalligraphyOptionWidget::emitAll()
{
    emit usePathChanged(m_usePath->isChecked());
    emit capsChanged(m_capsBox->value());
    emit massChanged(m_massBox->value());
    emit dragChanged(m_dragBox->value());
    emit smoothTimeChanged(m_smoothTime->value());
    emit smoothDistanceChanged(m_smoothDistance->value());
}

void KarbonCalligraphyOptionWidget::loadProfile(const QString &name)
{
    if (m_changingProfile) {
        return;
    }
    // write the new profile in the config file
    KConfig config(RCFILENAME);
    KConfigGroup generalGroup(&config, "General");
    generalGroup.writeEntry("profile", name);
    config.sync();

    // and load it
    loadCurrentProfile();

    // don't show Current if it isn't selected
    if (name != i18n("Current")) {
        removeProfile(i18n("Current"));
    }
}

void KarbonCalligraphyOptionWidget::updateCurrentProfile()
{
    if (!m_changingProfile) {
        saveProfile("Current");
    }
}

void KarbonCalligraphyOptionWidget::saveProfileAs()
{
    QString name;

    // loop until a valid name is entered or the user cancelled
    while (1) {
        bool ok;
        name = QInputDialog::getText(this,
                                     i18n("Profile name"),
                                     i18n("Please insert the name by which "
                                          "you want to save this profile:"),
                                     QLineEdit::Normal, QString(), &ok);
        if (!ok) {
            return;
        }

        if (name.isEmpty() || name == i18n("Current")) {
            KMessageBox::sorry(this,
                               i18n("Sorry, the name you entered is invalid."),
                               i18nc("invalid profile name", "Invalid name."));
            // try again
            saveProfileAs();
            continue; // ask again
        }

        if (m_profiles.contains(name)) {
            int ret = KMessageBox::warningYesNo(this,
                                                i18n("A profile with that name already exists.\n"
                                                        "Do you want to overwrite it?"));

            if (ret == KMessageBox::Yes) {
                break;    // exit while loop (save profile)
            }
            // else ask again
        } else {
            // the name is valid
            break; // exit while loop (save profile)
        }
    }

    saveProfile(name);
}

void KarbonCalligraphyOptionWidget::removeProfile()
{
    removeProfile(m_comboBox->currentText());
}

/******************************************************************************
 ************************* Convenience Functions ******************************
 ******************************************************************************/

void KarbonCalligraphyOptionWidget::createConnections()
{
    connect(m_comboBox, SIGNAL(currentIndexChanged(QString)),
            SLOT(loadProfile(QString)));

    // propagate changes
    connect(m_usePath, SIGNAL(toggled(bool)),
            SIGNAL(usePathChanged(bool)));

    connect(m_capsBox, SIGNAL(valueChanged(double)),
            SIGNAL(capsChanged(double)));

    connect(m_massBox, SIGNAL(valueChanged(double)),
            SIGNAL(massChanged(double)));

    connect(m_dragBox, SIGNAL(valueChanged(double)),
            SIGNAL(dragChanged(double)));

    connect(m_smoothTime, SIGNAL(valueChanged(double)),
            SIGNAL(smoothTimeChanged(double)));
    connect(m_smoothDistance, SIGNAL(valueChanged(double)),
            SIGNAL(smoothDistanceChanged(double)));

    // update profile
    connect(m_usePath, SIGNAL(toggled(bool)),
            SLOT(updateCurrentProfile()));
    connect(m_capsBox, SIGNAL(valueChanged(double)),
            SLOT(updateCurrentProfile()));

    connect(m_massBox, SIGNAL(valueChanged(double)),
            SLOT(updateCurrentProfile()));

    connect(m_dragBox, SIGNAL(valueChanged(double)),
            SLOT(updateCurrentProfile()));

    connect(m_saveButton, SIGNAL(clicked()), SLOT(saveProfileAs()));
    connect(m_removeButton, SIGNAL(clicked()), SLOT(removeProfile()));

    // visualization
    //connect(m_useAngle, SIGNAL(toggled(bool)), SLOT(toggleUseAngle(bool)));
}

void KarbonCalligraphyOptionWidget::addDefaultProfiles()
{
    // check if the profiles where already added
    KConfig config(RCFILENAME);
    KConfigGroup generalGroup(&config, "General");

    if (generalGroup.readEntry("defaultProfilesAdded", false)) {
        return;
    }

    KConfigGroup profile0(&config, "Profile0");
    profile0.writeEntry("name", i18n("Mouse"));
    profile0.writeEntry("usePath", false);
    profile0.writeEntry("usePressure", false);
    profile0.writeEntry("useAngle", false);
    profile0.writeEntry("width", 30.0);
    profile0.writeEntry("thinning", 0.2);
    profile0.writeEntry("angle", 30);
    profile0.writeEntry("fixation", 1.0);
    profile0.writeEntry("caps", 0.0);
    profile0.writeEntry("mass", 3.0);
    profile0.writeEntry("drag", 0.7);

    KConfigGroup profile1(&config, "Profile1");
    profile1.writeEntry("name", i18n("Graphics Pen"));
    profile1.writeEntry("width", 50.0);
    profile1.writeEntry("usePath", false);
    profile1.writeEntry("usePressure", false);
    profile1.writeEntry("useAngle", false);
    profile1.writeEntry("thinning", 0.2);
    profile1.writeEntry("angle", 30);
    profile1.writeEntry("fixation", 1.0);
    profile1.writeEntry("caps", 0.0);
    profile1.writeEntry("mass", 1.0);
    profile1.writeEntry("drag", 0.9);

    generalGroup.writeEntry("profile", i18n("Mouse"));
    generalGroup.writeEntry("defaultProfilesAdded", true);

    config.sync();
}

void KarbonCalligraphyOptionWidget::loadProfiles()
{
    KConfig config(RCFILENAME);

    // load profiles as long as they are present
    int i = 0;
    while (1) { // forever
        KConfigGroup profileGroup(&config, "Profile" + QString::number(i));
        // invalid profile, assume we reached the last one
        if (!profileGroup.hasKey("name")) {
            break;
        }

        Profile *profile = new Profile;
        profile->index = i;
        profile->name =         profileGroup.readEntry("name", QString());
        profile->usePath =      profileGroup.readEntry("usePath", false);
        profile->caps =         profileGroup.readEntry("caps", 0.0);
        profile->mass =         profileGroup.readEntry("mass", 3.0);
        profile->drag =         profileGroup.readEntry("drag", 0.7);

        m_profiles.insert(profile->name, profile);
        ++i;
    }

    m_changingProfile = true;
    ProfileMap::const_iterator it = m_profiles.constBegin();
    ProfileMap::const_iterator lastIt = m_profiles.constEnd();
    for (; it != lastIt; ++it) {
        m_comboBox->addItem(it.key());
    }
    m_changingProfile = false;

    loadCurrentProfile();
}

void KarbonCalligraphyOptionWidget::loadCurrentProfile()
{
    KConfig config(RCFILENAME);
    KConfigGroup generalGroup(&config, "General");
    QString currentProfile = generalGroup.readEntry("profile", QString());
    // find the index needed by the comboBox
    int index = profilePosition(currentProfile);

    if (currentProfile.isEmpty() || index < 0) {
        return;
    }

    m_comboBox->setCurrentIndex(index);

    Profile *profile = m_profiles[currentProfile];

    m_changingProfile = true;
    m_usePath->setChecked(profile->usePath);
    m_capsBox->setValue(profile->caps);
    m_massBox->setValue(profile->mass);
    m_dragBox->setValue(profile->drag);
    m_changingProfile = false;
}

void KarbonCalligraphyOptionWidget::saveProfile(const QString &name)
{
    Profile *profile = new Profile;
    profile->name = name;
    profile->usePath = m_usePath->isChecked();
    profile->caps = m_capsBox->value();
    profile->mass = m_massBox->value();
    profile->drag = m_dragBox->value();

    if (m_profiles.contains(name)) {
        // there is already a profile with the same name, overwrite
        profile->index = m_profiles[name]->index;
        m_profiles.insert(name, profile);
    } else {
        // it is a new profile
        profile->index = m_profiles.count();
        m_profiles.insert(name, profile);
        // add the profile to the combobox
        QString dbg;
        for (int i = 0; i < m_comboBox->count(); ++i) {
            dbg += m_comboBox->itemText(i) + ' ';
        }
        int pos = profilePosition(name);
        m_changingProfile = true;
        m_comboBox->insertItem(pos, name);
        m_changingProfile = false;
        for (int i = 0; i < m_comboBox->count(); ++i) {
            dbg += m_comboBox->itemText(i) + ' ';
        }
    }

    KConfig config(RCFILENAME);
    QString str = "Profile" + QString::number(profile->index);
    KConfigGroup profileGroup(&config, str);

    profileGroup.writeEntry("name", name);
    profileGroup.writeEntry("usePath", profile->usePath);
    profileGroup.writeEntry("caps", profile->caps);
    profileGroup.writeEntry("mass", profile->mass);
    profileGroup.writeEntry("drag", profile->drag);

    KConfigGroup generalGroup(&config, "General");
    generalGroup.writeEntry("profile", name);

    config.sync();

    m_comboBox->setCurrentIndex(profilePosition(name));
}

void KarbonCalligraphyOptionWidget::removeProfile(const QString &name)
{
    int index = profilePosition(name);
    if (index < 0) {
        return;    // no such profile
    }

    // remove the file from the config file
    KConfig config(RCFILENAME);
    int deletedIndex = m_profiles[name]->index;
    QString deletedGroup = "Profile" + QString::number(deletedIndex);
    config.deleteGroup(deletedGroup);
    config.sync();

    // and from profiles
    m_profiles.remove(name);

    m_comboBox->removeItem(index);

    // now in the config file there is value ProfileN missing,
    // where N = configIndex, so put the last one there
    if (m_profiles.isEmpty()) {
        return;
    }

    int lastN = -1;
    Profile *profile = 0; // profile to be moved, will be the last one
    Q_FOREACH (Profile *p, m_profiles) {
        if (p->index > lastN) {
            lastN = p->index;
            profile = p;
        }
    }

    Q_ASSERT(profile != 0);

    // do nothing if the deleted group was the last one
    if (deletedIndex > lastN) {
        return;
    }

    QString lastGroup = "Profile" + QString::number(lastN);
    config.deleteGroup(lastGroup);

    KConfigGroup profileGroup(&config, deletedGroup);
    profileGroup.writeEntry("name", profile->name);
    profileGroup.writeEntry("usePath", profile->usePath);
    profileGroup.writeEntry("caps", profile->caps);
    profileGroup.writeEntry("mass", profile->mass);
    profileGroup.writeEntry("drag", profile->drag);
    config.sync();

    profile->index = deletedIndex;
}

int KarbonCalligraphyOptionWidget::profilePosition(const QString &profileName)
{
    int res = 0;
    ProfileMap::const_iterator it = m_profiles.constBegin();
    ProfileMap::const_iterator lastIt = m_profiles.constEnd();
    for (; it != lastIt; ++it) {
        if (it.key() == profileName) {
            return res;
        }
        ++res;
    }
    return -1;
}

void KarbonCalligraphyOptionWidget::setUsePathEnabled(bool enabled)
{
    m_usePath->setEnabled(enabled);
}
