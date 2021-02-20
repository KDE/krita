/*
 * SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "kis_dlg_png_import.h"

#include <QLabel>

#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceEngine.h>
#include <KoID.h>
#include <KisSqueezedComboBox.h>
#include "kis_config.h"

KisDlgPngImport::KisDlgPngImport(const QString &path, const QString &colorModelID, const QString &colorDepthID, QWidget *parent)
    : KoDialog(parent)
{
    setButtons(Ok);
    setDefaultButton(Ok);
    QWidget *page = new QWidget(this);
    dlgWidget.setupUi(page);
    setMainWidget(page);

    dlgWidget.lblFilename->setText(path);

    const QString colorSpaceId = KoColorSpaceRegistry::instance()->colorSpaceId(colorModelID, colorDepthID);
    dlgWidget.cmbProfile->clear();
    QList<const KoColorProfile *>  profileList = KoColorSpaceRegistry::instance()->profilesFor(colorSpaceId);
    QStringList profileNames;
    Q_FOREACH (const KoColorProfile *profile, profileList) {
        profileNames.append(profile->name());
    }
    std::sort(profileNames.begin(), profileNames.end());
    Q_FOREACH (QString stringName, profileNames) {
        dlgWidget.cmbProfile->addSqueezedItem(stringName);
    }
    KisConfig cfg(true);
    QString profile = cfg.readEntry<QString>("pngImportProfile", KoColorSpaceRegistry::instance()->defaultProfileForColorSpace(colorSpaceId));
    dlgWidget.cmbProfile->setCurrent(profile);
}

QString KisDlgPngImport::profile() const
{
    QString p = dlgWidget.cmbProfile->currentUnsqueezedText();
    KisConfig cfg(false);
    cfg.writeEntry("pngImportProfile", p);
    return p;
}
