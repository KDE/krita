/* This file is part of the KDE project
   Copyright (C) 2002, 2003 Laurent Montel <lmontel@mandrakesoft.com>
   Copyright (C) 2006-2007,2010-2011 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>

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

#include "KoConfigMiscPage.h"

#include <KoUnit.h>
#include <KoDocument.h>
#include <KoUnitDoubleSpinBox.h>
#include <KoDocumentResourceManager.h>
#include <KoPart.h>

#include <kcomponentdata.h>
#include <kcombobox.h>
#include <knuminput.h>

#include <kdialog.h>
#include <kconfig.h>

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QCheckBox>

class KoConfigMiscPage::Private
{
public:
    Private(KoDocument* doc, KoDocumentResourceManager *documentResources)
            : doc(doc), docResources(documentResources)
    {}

    KoDocument* doc;
    KSharedConfigPtr config;
    KoDocumentResourceManager *docResources;

    KoUnit oldUnit;
    QComboBox *unit;
    KIntNumInput * handleRadius;
    uint oldHandleRadius;
    KIntNumInput * grabSensitivity;
    uint oldGrabSensitivity;
    KoUnitDoubleSpinBox* pasteOffset;
    qreal oldPasteOffset;
    QCheckBox *pasteAtCursor;
    bool oldPasteAtCursor;
};

KoConfigMiscPage::KoConfigMiscPage(KoDocument* doc, KoDocumentResourceManager *documentResources, char* name)
        : d(new Private(doc, documentResources))
{
    setObjectName(name);

    d->config = d->doc->documentPart()->componentData().config();

    d->oldGrabSensitivity = d->docResources->grabSensitivity();
    d->oldHandleRadius = d->docResources->handleRadius();
    d->oldPasteOffset = d->docResources->pasteOffset();
    d->oldPasteAtCursor = d->docResources->pasteAtCursor();

    const KoUnit documentUnit = d->doc->unit();

    QGroupBox* tmpQGroupBox = new QGroupBox(i18n("Misc"), this);

    QGridLayout* grid = new QGridLayout();
    grid->setSpacing(KDialog::spacingHint());
    grid->setMargin(KDialog::marginHint());

    //#################"laurent
    //don't load unitType from config file because unit is
    //depend from words file => unit can be different from config file

    grid->addWidget(new QLabel(i18n("Units:"), tmpQGroupBox), 0, 0);

    d->unit = new KComboBox(tmpQGroupBox);
    d->unit->addItems(KoUnit::listOfUnitNameForUi(KoUnit::HidePixel));
    grid->addWidget(d->unit, 0, 1);
    d->oldUnit = documentUnit;
    d->unit->setCurrentIndex(d->oldUnit.indexInListForUi(KoUnit::HidePixel));

    grid->addWidget(new QLabel(i18n("Handle radius:"), tmpQGroupBox), 1, 0);

    d->handleRadius = new KIntNumInput(tmpQGroupBox);
    d->handleRadius->setRange(3, 20, 1);
    d->handleRadius->setSuffix(" px");
    d->handleRadius->setValue(d->oldHandleRadius);
    grid->addWidget(d->handleRadius, 1, 1);

    grid->addWidget(new QLabel(i18n("Grab sensitivity:"), tmpQGroupBox), 2, 0);

    d->grabSensitivity = new KIntNumInput(tmpQGroupBox);
    d->grabSensitivity->setRange(3, 20, 1);
    d->grabSensitivity->setSuffix(" px");
    d->grabSensitivity->setValue(d->oldGrabSensitivity);
    grid->addWidget(d->grabSensitivity, 2, 1);

    grid->addWidget(new QLabel(i18n("Paste offset:"), tmpQGroupBox), 3, 0);

    d->pasteOffset = new KoUnitDoubleSpinBox(tmpQGroupBox);
    d->pasteOffset->setMinMaxStep(-1000, 1000, 0.1);
    d->pasteOffset->setValue(d->oldPasteOffset);
    d->pasteOffset->setUnit(documentUnit);
    d->pasteOffset->setDisabled(d->oldPasteAtCursor);
    grid->addWidget(d->pasteOffset, 3, 1);

    grid->addWidget(new QLabel(i18n("Paste at Cursor"), tmpQGroupBox), 4, 0);
    d->pasteAtCursor = new QCheckBox(tmpQGroupBox);
    d->pasteAtCursor->setChecked(d->oldPasteAtCursor);
    grid->addWidget(d->pasteAtCursor, 4, 1);

    grid->setRowStretch(5, 1);

    tmpQGroupBox->setLayout(grid);

    connect(d->unit, SIGNAL(activated(int)), SLOT(slotUnitChanged(int)));
    connect(d->pasteAtCursor, SIGNAL(clicked(bool)), d->pasteOffset, SLOT(setDisabled(bool)));
}

KoConfigMiscPage::~KoConfigMiscPage()
{
    delete d;
}

void KoConfigMiscPage::apply()
{
    KConfigGroup miscGroup = d->config->group("Misc");

    int currentUnitIndex = d->unit->currentIndex();
    if (d->oldUnit.indexInListForUi(KoUnit::HidePixel) != currentUnitIndex) {
        d->oldUnit = KoUnit::fromListForUi(currentUnitIndex, KoUnit::HidePixel);
        d->doc->setUnit(d->oldUnit);
        miscGroup.writeEntry("Units", d->oldUnit.symbol());
    }

    uint currentHandleRadius = d->handleRadius->value();
    if (currentHandleRadius != d->oldHandleRadius) {
        miscGroup.writeEntry( "HandleRadius", currentHandleRadius );
        d->docResources->setHandleRadius(currentHandleRadius);
    }

    uint currentGrabSensitivity = d->grabSensitivity->value();
    if (currentGrabSensitivity != d->oldGrabSensitivity) {
        miscGroup.writeEntry("GrabSensitivity", currentGrabSensitivity);
        d->docResources->setGrabSensitivity(currentGrabSensitivity);
    }

    qreal currentCopyOffset = d->pasteOffset->value();
    if (currentCopyOffset != d->oldPasteOffset) {
        miscGroup.writeEntry("CopyOffset", currentCopyOffset);
        d->docResources->setPasteOffset(currentCopyOffset);
    }

    const bool currentPasteAtCursor = d->pasteAtCursor->isChecked();
    if (currentPasteAtCursor != d->oldPasteAtCursor) {
        miscGroup.writeEntry("PasteAtCursor", currentPasteAtCursor);
        d->docResources->enablePasteAtCursor(currentPasteAtCursor);
    }
}

void KoConfigMiscPage::slotDefault()
{
    d->unit->setCurrentIndex(0);
}

void KoConfigMiscPage::slotUnitChanged(int u)
{
    const KoUnit unit = KoUnit::fromListForUi(u, KoUnit::HidePixel);

    d->pasteOffset->blockSignals(true);
    d->pasteOffset->setUnit(unit);
    d->pasteOffset->blockSignals(false);

    emit unitChanged(unit);
}

#include <KoConfigMiscPage.moc>

