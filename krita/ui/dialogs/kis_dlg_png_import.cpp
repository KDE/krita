/*
 * Copyright (C) 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "kis_dlg_png_import.h"

#include <QLabel>

#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceEngine.h>
#include <KoID.h>
#include "widgets/squeezedcombobox.h"

KisDlgPngImport::KisDlgPngImport(const QString &path, const QString &colorModelID, const QString &colorDepthID, QWidget *parent)
    : KDialog(parent)
{
    setButtons(Ok);
    setDefaultButton(Ok);
    QWidget *page = new QWidget(this);
    dlgWidget.setupUi(page);
    setMainWidget(page);

    dlgWidget.lblFilename->setText(path);

    QString s = KoColorSpaceRegistry::instance()->colorSpaceId(colorModelID, colorDepthID);
    dlgWidget.cmbProfile->clear();
    const KoColorSpaceFactory * csf = KoColorSpaceRegistry::instance()->colorSpaceFactory(s);
    if (csf) {
        QList<const KoColorProfile *>  profileList = KoColorSpaceRegistry::instance()->profilesFor(csf);

        foreach(const KoColorProfile *profile, profileList) {
            dlgWidget.cmbProfile->addSqueezedItem(profile->name());
        }
        dlgWidget.cmbProfile->setCurrent(csf->defaultProfile());
    }
}

QString KisDlgPngImport::profile() const
{
    return dlgWidget.cmbProfile->itemHighlighted();
}
