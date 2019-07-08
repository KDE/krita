/*
 *  Copyright (C) 2007 Cyrille Berger <cberger@cberger.net>
 *  Copyright (C) 2011 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_color_space_selector.h"
#include "kis_advanced_color_space_selector.h"


#include <QUrl>

#include <KoFileDialog.h>
#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceEngine.h>
#include <KoID.h>

#include <KoConfig.h>
#include <kis_icon.h>

#include <QStandardPaths>

#include <KoResourcePaths.h>


#include <kis_debug.h>

#include "ui_wdgcolorspaceselector.h"

struct KisColorSpaceSelector::Private {
    Ui_WdgColorSpaceSelector* colorSpaceSelector;
    QString knsrcFile;
    bool profileValid;
    QString defaultsuffix;
    bool profileSetManually;
    KoID previousModel;
};

KisColorSpaceSelector::KisColorSpaceSelector(QWidget* parent) : QWidget(parent), m_advancedSelector(0), d(new Private)
{
    setObjectName("KisColorSpaceSelector");
    d->colorSpaceSelector = new Ui_WdgColorSpaceSelector;
    d->colorSpaceSelector->setupUi(this);
    d->colorSpaceSelector->cmbColorModels->setIDList(KoColorSpaceRegistry::instance()->colorModelsList(KoColorSpaceRegistry::OnlyUserVisible));
    fillCmbDepths(d->colorSpaceSelector->cmbColorModels->currentItem());

    d->colorSpaceSelector->bnInstallProfile->setIcon(KisIconUtils::loadIcon("document-open"));
    d->colorSpaceSelector->bnInstallProfile->setToolTip( i18n("Open Color Profile") );

    connect(d->colorSpaceSelector->cmbColorModels, SIGNAL(activated(KoID)),
            this, SLOT(slotModelsComboBoxActivated(KoID)));
    connect(d->colorSpaceSelector->cmbColorDepth, SIGNAL(activated(KoID)),
            this, SLOT(slotDepthsComboBoxActivated()));
    connect(d->colorSpaceSelector->cmbProfile, SIGNAL(activated(QString)),
            this, SLOT(slotProfilesComboBoxActivated()));
    connect(d->colorSpaceSelector->bnInstallProfile, SIGNAL(clicked()),
            this, SLOT(installProfile()));

    d->defaultsuffix = " "+i18nc("This is appended to the color profile which is the default for the given colorspace and bit-depth","(Default)");
    d->profileSetManually = false;
    d->previousModel = d->colorSpaceSelector->cmbColorModels->currentItem();

    connect(d->colorSpaceSelector->bnAdvanced, SIGNAL(clicked()), this,  SLOT(slotOpenAdvancedSelector()));

    fillCmbProfiles();
}

KisColorSpaceSelector::~KisColorSpaceSelector()
{
    delete d->colorSpaceSelector;
    delete d;
}


void KisColorSpaceSelector::fillCmbProfiles()
{
    const QString currentProfileName = d->colorSpaceSelector->cmbProfile->itemHighlighted();

    const QString colorSpaceId = KoColorSpaceRegistry::instance()->colorSpaceId(d->colorSpaceSelector->cmbColorModels->currentItem(), d->colorSpaceSelector->cmbColorDepth->currentItem());
    const QString defaultProfileName = KoColorSpaceRegistry::instance()->defaultProfileForColorSpace(colorSpaceId);

    d->colorSpaceSelector->cmbProfile->clear();

    QList<const KoColorProfile *>  profileList = KoColorSpaceRegistry::instance()->profilesFor(colorSpaceId);
    QStringList profileNames;
    Q_FOREACH (const KoColorProfile *profile, profileList) {
        profileNames.append(profile->name());
    }
    std::sort(profileNames.begin(), profileNames.end());
    Q_FOREACH (QString stringName, profileNames) {
        if (stringName == defaultProfileName) {
            d->colorSpaceSelector->cmbProfile->addSqueezedItem(stringName + d->defaultsuffix);
        } else {
            d->colorSpaceSelector->cmbProfile->addSqueezedItem(stringName);
        }
    }
    if (d->profileSetManually && profileNames.contains(currentProfileName)) {
        d->colorSpaceSelector->cmbProfile->setCurrent(currentProfileName);
    } else {
        d->colorSpaceSelector->cmbProfile->setCurrent(defaultProfileName + d->defaultsuffix);
    }
    colorSpaceChanged();
}

void KisColorSpaceSelector::fillCmbDepths(const KoID& id)
{
    KoID activeDepth = d->colorSpaceSelector->cmbColorDepth->currentItem();
    d->colorSpaceSelector->cmbColorDepth->clear();
    QList<KoID> depths = KoColorSpaceRegistry::instance()->colorDepthList(id, KoColorSpaceRegistry::OnlyUserVisible);
    d->colorSpaceSelector->cmbColorDepth->setIDList(depths, false);
    if (depths.contains(activeDepth)) {
        d->colorSpaceSelector->cmbColorDepth->setCurrent(activeDepth);
    }
}



const KoColorSpace* KisColorSpaceSelector::currentColorSpace()
{
    QString profilenamestring = d->colorSpaceSelector->cmbProfile->itemHighlighted();
    if (profilenamestring.contains(d->defaultsuffix)) {
        profilenamestring.remove(d->defaultsuffix);
        return KoColorSpaceRegistry::instance()->colorSpace(
                    d->colorSpaceSelector->cmbColorModels->currentItem().id(),
                    d->colorSpaceSelector->cmbColorDepth->currentItem().id(),
                    profilenamestring);
    } else {
        return KoColorSpaceRegistry::instance()->colorSpace(
                    d->colorSpaceSelector->cmbColorModels->currentItem().id(),
                    d->colorSpaceSelector->cmbColorDepth->currentItem().id(),
                    profilenamestring);
    }
}

void KisColorSpaceSelector::setCurrentColorModel(const KoID& id)
{
    d->colorSpaceSelector->cmbColorModels->setCurrent(id);
    fillCmbDepths(id);
}

void KisColorSpaceSelector::setCurrentColorDepth(const KoID& id)
{
    d->colorSpaceSelector->cmbColorDepth->setCurrent(id);
    if (!d->profileSetManually) {
        fillCmbProfiles();
    }
}

void KisColorSpaceSelector::setCurrentProfile(const QString& name)
{
    d->colorSpaceSelector->cmbProfile->setCurrent(name);
}

void KisColorSpaceSelector::setCurrentColorSpace(const KoColorSpace* colorSpace)
{
    if (!colorSpace) {
        return;
    }
    setCurrentColorModel(colorSpace->colorModelId());
    setCurrentColorDepth(colorSpace->colorDepthId());
    setCurrentProfile(colorSpace->profile()->name());
}

void KisColorSpaceSelector::showColorBrowserButton(bool showButton) {
    d->colorSpaceSelector->bnAdvanced->setVisible(showButton);
}

void KisColorSpaceSelector::colorSpaceChanged()
{
    bool valid = d->colorSpaceSelector->cmbProfile->count() != 0;
    d->profileValid = valid;
    emit(selectionChanged(valid));
    if(valid) {
        emit colorSpaceChanged(currentColorSpace());
        QString text = currentColorSpace()->profile()->name();
    }
}

void KisColorSpaceSelector::installProfile()
{
    QStringList mime;
    KoFileDialog dialog(this, KoFileDialog::OpenFiles, "OpenDocumentICC");
    dialog.setCaption(i18n("Install Color Profiles"));
    dialog.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    dialog.setMimeTypeFilters(QStringList() << "application/vnd.iccprofile", "application/vnd.iccprofile");

    QStringList profileNames = dialog.filenames();

    KoColorSpaceEngine *iccEngine = KoColorSpaceEngineRegistry::instance()->get("icc");
    Q_ASSERT(iccEngine);

    QString saveLocation = KoResourcePaths::saveLocation("icc_profiles");

    Q_FOREACH (const QString &profileName, profileNames) {
        QUrl file(profileName);
        if (!QFile::copy(profileName, saveLocation + file.fileName())) {
            dbgKrita << "Could not install profile!";
            return;
        }
        iccEngine->addProfile(saveLocation + file.fileName());

    }

    fillCmbProfiles();
}

void KisColorSpaceSelector::slotOpenAdvancedSelector()
{
    if (!m_advancedSelector) {
        m_advancedSelector = new KisAdvancedColorSpaceSelector(this, i18n("Select a Colorspace"));
        m_advancedSelector->setModal(true);
        if (currentColorSpace()) {
            m_advancedSelector->setCurrentColorSpace(currentColorSpace());
        }
        connect(m_advancedSelector, SIGNAL(selectionChanged(bool)), this, SLOT(slotProfileValid(bool)) );
    }

    QDialog::DialogCode result = (QDialog::DialogCode)m_advancedSelector->exec();

    if (result) {
        if (d->profileValid==true) {
            setCurrentColorSpace(m_advancedSelector->currentColorSpace());
        }
    }
}

void KisColorSpaceSelector::slotProfileValid(bool valid)
{
    d->profileValid = valid;
}


void KisColorSpaceSelector::slotModelsComboBoxActivated(const KoID& id)
{
    if (d->previousModel != id) {
        d->previousModel = id;
        d->profileSetManually = false;
        fillCmbDepths(id);
        fillCmbProfiles();
    }
}

void KisColorSpaceSelector::slotDepthsComboBoxActivated()
{
    fillCmbProfiles();
}


void KisColorSpaceSelector::slotProfilesComboBoxActivated()
{
    d->profileSetManually = true;
    colorSpaceChanged();
}

