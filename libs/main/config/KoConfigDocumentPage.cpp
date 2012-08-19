/* This file is part of the KDE project
Copyright (C) 2002, 2003 Laurent Montel <lmontel@mandrakesoft.com>
Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>

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
* Boston, MA 02110-1301, USA.
*/

#include "KoConfigDocumentPage.h"

#include <KoDocument.h>
#include <KoPart.h>

#include <kdialog.h>
#include <klocale.h>
#include <knuminput.h>
#include <kcomponentdata.h>
#include <kconfig.h>

#include <QFormLayout>
#include <QCheckBox>
#include <QGroupBox>

class KoConfigDocumentPage::Private
{
public:
    Private(KoDocument* doc)
    : doc(doc)
    {}

    KoDocument* doc;
    KSharedConfigPtr config;

    KIntNumInput* autoSave;
    int oldAutoSave;
    QCheckBox *createBackupFile;
    bool oldBackupFile;
};

KoConfigDocumentPage::KoConfigDocumentPage(KoDocument* doc, char* name)
: d(new Private(doc))
{
    setObjectName(name);

    d->config = d->doc->documentPart()->componentData().config();

    QGroupBox* gbDocumentSettings = new QGroupBox(i18n("Document Settings"), this);
    QFormLayout *layout = new QFormLayout(gbDocumentSettings);

    d->oldAutoSave = doc->defaultAutoSave() / 60;

    d->oldBackupFile = true;

    if(d->config->hasGroup("Interface")) {
        KConfigGroup interfaceGroup = d->config->group("Interface");
        d->oldAutoSave = interfaceGroup.readEntry("AutoSave", d->oldAutoSave);
        d->oldBackupFile = interfaceGroup.readEntry("BackupFile", d->oldBackupFile);
    }

    d->autoSave = new KIntNumInput(d->oldAutoSave, gbDocumentSettings);
    d->autoSave->setRange(0, 60, 1);
    d->autoSave->setSpecialValueText(i18n("No autosave"));
    d->autoSave->setSuffix(i18nc("unit symbol for minutes, leading space as separator", " min"));
    layout->addRow(i18n("Autosave interval:"), d->autoSave);

    d->createBackupFile = new QCheckBox(gbDocumentSettings);
    d->createBackupFile->setChecked(d->oldBackupFile);
    layout->addRow(i18n("Create backup file:"), d->createBackupFile);
}

KoConfigDocumentPage::~KoConfigDocumentPage()
{
    delete d;
}

void KoConfigDocumentPage::apply()
{
    KConfigGroup interfaceGroup = d->config->group("Interface");

    int autoSave = d->autoSave->value();

    if (autoSave != d->oldAutoSave) {
        interfaceGroup.writeEntry("AutoSave", autoSave);
        d->doc->setAutoSave(autoSave * 60);
        d->oldAutoSave = autoSave;
    }

    bool state = d->createBackupFile->isChecked();

    if (state != d->oldBackupFile) {
        interfaceGroup.writeEntry("BackupFile", state);
        d->doc->setBackupFile(state);
        d->oldBackupFile = state;
    }
}

void KoConfigDocumentPage::slotDefault()
{
    d->autoSave->setValue(d->doc->defaultAutoSave() / 60);
    d->createBackupFile->setChecked(true);
}

#include <KoConfigDocumentPage.moc>
