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

#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceEngine.h>
#include <KoID.h>

#include <KoConfig.h>
#include <KoIcon.h>

#ifdef GHNS
#include <knewstuff3/downloaddialog.h>
#include <knewstuff3/uploaddialog.h>
#endif

#include <QDesktopServices>

#include <kcomponentdata.h>
#include <kfiledialog.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kio/copyjob.h>

#include "kis_factory2.h"

#include "ui_wdgcolorspaceselector.h"

struct KisColorSpaceSelector::Private {
    Ui_WdgColorSpaceSelector* colorSpaceSelector;
    QString knsrcFile;
};

KisColorSpaceSelector::KisColorSpaceSelector(QWidget* parent) : QWidget(parent), d(new Private)
{
    setObjectName("KisColorSpaceSelector");
    d->colorSpaceSelector = new Ui_WdgColorSpaceSelector;
    d->colorSpaceSelector->setupUi(this);
    d->colorSpaceSelector->cmbColorModels->setIDList(KoColorSpaceRegistry::instance()->colorModelsList(KoColorSpaceRegistry::OnlyUserVisible));
    fillCmbDepths(d->colorSpaceSelector->cmbColorModels->currentItem());

    d->colorSpaceSelector->bnInstallProfile->setIcon(koIcon("document-open"));
    d->colorSpaceSelector->bnInstallProfile->setToolTip( i18n("Open Color Profile") );

    d->colorSpaceSelector->bnDownloadProfile->setIcon(koIcon("download"));
    d->colorSpaceSelector->bnDownloadProfile->setToolTip( i18n("Download Color Profile") );
    d->colorSpaceSelector->bnDownloadProfile->setEnabled( true );
    d->colorSpaceSelector->bnDownloadProfile->hide();

    d->colorSpaceSelector->bnUploadProfile->setIcon(koIcon("go-up"));
    d->colorSpaceSelector->bnUploadProfile->setToolTip( i18n("Share Color Profile") );
    d->colorSpaceSelector->bnUploadProfile->setEnabled( false );
    d->colorSpaceSelector->bnUploadProfile->hide();

#ifdef GHNS
    d->colorSpaceSelector->bnUploadProfile->show();
    d->colorSpaceSelector->bnDownloadProfile->show();
#endif

    connect(d->colorSpaceSelector->cmbColorModels, SIGNAL(activated(const KoID &)),
            this, SLOT(fillCmbDepths(const KoID &)));
    connect(d->colorSpaceSelector->cmbColorDepth, SIGNAL(activated(const KoID &)),
            this, SLOT(fillCmbProfiles()));
    connect(d->colorSpaceSelector->cmbColorModels, SIGNAL(activated(const KoID &)),
            this, SLOT(fillCmbProfiles()));
    connect(d->colorSpaceSelector->cmbProfile, SIGNAL(activated(const QString &)),
            this, SLOT(colorSpaceChanged()));
    connect(d->colorSpaceSelector->cmbProfile, SIGNAL(currentIndexChanged(int)),
            this, SLOT(buttonUpdate()));


    connect(d->colorSpaceSelector->bnInstallProfile, SIGNAL(clicked()), this, SLOT(installProfile()));
    connect(d->colorSpaceSelector->bnDownloadProfile, SIGNAL(clicked()), this, SLOT(downloadProfile()));
    connect(d->colorSpaceSelector->bnUploadProfile, SIGNAL(clicked()), this, SLOT(uploadProfile()));

    d->knsrcFile = "kritaiccprofiles.knsrc";

    fillCmbProfiles();
}

KisColorSpaceSelector::~KisColorSpaceSelector()
{
    delete d->colorSpaceSelector;
    delete d;
}


void KisColorSpaceSelector::fillCmbProfiles()
{
    QString s = KoColorSpaceRegistry::instance()->colorSpaceId(d->colorSpaceSelector->cmbColorModels->currentItem(), d->colorSpaceSelector->cmbColorDepth->currentItem());
    d->colorSpaceSelector->cmbProfile->clear();

    const KoColorSpaceFactory * csf = KoColorSpaceRegistry::instance()->colorSpaceFactory(s);
    if (csf == 0) return;

    QList<const KoColorProfile *>  profileList = KoColorSpaceRegistry::instance()->profilesFor(csf);

    foreach(const KoColorProfile *profile, profileList) {
        d->colorSpaceSelector->cmbProfile->addSqueezedItem(profile->name());
    }
    d->colorSpaceSelector->cmbProfile->setCurrent(csf->defaultProfile());
    colorSpaceChanged();
}

void KisColorSpaceSelector::fillCmbDepths(const KoID& id)
{
    KoID activeDepth = d->colorSpaceSelector->cmbColorDepth->currentItem();
    d->colorSpaceSelector->cmbColorDepth->clear();
    QList<KoID> depths = KoColorSpaceRegistry::instance()->colorDepthList(id, KoColorSpaceRegistry::OnlyUserVisible);
    d->colorSpaceSelector->cmbColorDepth->setIDList(depths);
    if (depths.contains(activeDepth)) {
        d->colorSpaceSelector->cmbColorDepth->setCurrent(activeDepth);
    }
}

const KoColorSpace* KisColorSpaceSelector::currentColorSpace()
{
    return KoColorSpaceRegistry::instance()->colorSpace(
               d->colorSpaceSelector->cmbColorModels->currentItem().id(), d->colorSpaceSelector->cmbColorDepth->currentItem().id()
               , d->colorSpaceSelector->cmbProfile->itemHighlighted());
}

void KisColorSpaceSelector::setCurrentColorModel(const KoID& id)
{
    d->colorSpaceSelector->cmbColorModels->setCurrent(id);
    fillCmbDepths(id);
}

void KisColorSpaceSelector::setCurrentColorDepth(const KoID& id)
{
    d->colorSpaceSelector->cmbColorDepth->setCurrent(id);
    fillCmbProfiles();
}

void KisColorSpaceSelector::setCurrentProfile(const QString& name)
{
    d->colorSpaceSelector->cmbProfile->setCurrent(name);
}

void KisColorSpaceSelector::setCurrentColorSpace(const KoColorSpace* colorSpace)
{
  setCurrentColorModel(colorSpace->colorModelId());
  setCurrentColorDepth(colorSpace->colorDepthId());
  setCurrentProfile(colorSpace->profile()->name());
}

void KisColorSpaceSelector::colorSpaceChanged()
{
    bool valid = d->colorSpaceSelector->cmbProfile->count() != 0;
    emit(selectionChanged(valid));
    if(valid) {
        emit colorSpaceChanged(currentColorSpace());
    }
}

void KisColorSpaceSelector::installProfile()
{
    KUrl homedir(QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
    QStringList profileNames = KFileDialog::getOpenFileNames(homedir, "*.icm *.icc", this, "Install Color Profiles");

    KoColorSpaceEngine *iccEngine = KoColorSpaceEngineRegistry::instance()->get("icc");
    Q_ASSERT(iccEngine);

    QString saveLocation = KGlobal::mainComponent().dirs()->saveLocation("icc_profiles");

    foreach (QString profileName, profileNames) {
        KUrl file(profileName);
        if (!QFile::copy(profileName, saveLocation + file.fileName())) {
            kWarning() << "Could not install profile!";
            return;
        }
        iccEngine->addProfile(saveLocation + file.fileName());

    }

    fillCmbProfiles();
}

void KisColorSpaceSelector::downloadProfile()
{
#ifdef GHNS
    KNS3::DownloadDialog dialog( "kritaiccprofiles.knsrc", this);
    dialog.exec();
    KoColorSpaceEngine *iccEngine = KoColorSpaceEngineRegistry::instance()->get("icc");
    Q_ASSERT(iccEngine);
    foreach (const KNS3::Entry& e, dialog.changedEntries()) {
        foreach(const QString &file, e.installedFiles()) {
            QFileInfo fi(file);
            iccEngine->addProfile( fi.absolutePath()+'/'+fi.fileName());
        }
        foreach(const QString &file, e.uninstalledFiles()) {
            QFileInfo fi(file);
            iccEngine->removeProfile( fi.absolutePath()+'/'+fi.fileName());
        }
    }
    fillCmbProfiles();
#endif
}

void KisColorSpaceSelector::uploadProfile()
{
#ifdef GHNS
    KNS3::UploadDialog dialog("kritaiccprofiles.knsrc", this);
    const KoColorProfile *  profile = KoColorSpaceRegistry::instance()->profileByName(d->colorSpaceSelector->cmbProfile->currentText());
    if(!profile)  return;
    dialog.setUploadFile(KUrl::fromLocalFile(profile->fileName()));
    dialog.setUploadName(profile->name());
    dialog.exec();
#endif
}

void KisColorSpaceSelector::buttonUpdate()
{
   const KoColorProfile *  profile = KoColorSpaceRegistry::instance()->profileByName(d->colorSpaceSelector->cmbProfile->currentText());
   if(!profile)  return;

   QFileInfo fileInfo(profile->fileName());
   if(fileInfo.isWritable()) {
       d->colorSpaceSelector->bnUploadProfile->setEnabled( true );
       return;
   }
   d->colorSpaceSelector->bnUploadProfile->setEnabled( false );
}

#include "kis_color_space_selector.moc"
