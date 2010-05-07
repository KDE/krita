/* This file is part of the KDE project
   Copyright (C) 2002, 2003 Laurent Montel <lmontel@mandrakesoft.com>
   Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>
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

#include <kcomponentdata.h>
#include <kcombobox.h>
#include <knuminput.h>

#include <kdialog.h>
#include <kconfig.h>

#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>

class KoConfigMiscPage::Private
{
public:
    Private(KoDocument* doc)
    : doc(doc)
    {}

    KoDocument* doc;
    KSharedConfigPtr config;

    KoUnit oldUnit;
    QComboBox *unit;
    KIntNumInput * handleRadius;
    uint oldHandleRadius;
    KIntNumInput * grabSensitivity;
    uint oldGrabSensitivity;
    KoUnitDoubleSpinBox* copyOffset;
    qreal oldCopyOffset;
};

KoConfigMiscPage::KoConfigMiscPage(KoDocument* doc, char* name)
: d(new Private(doc))
{
    setObjectName(name);

    d->config = d->doc->componentData().config();

    d->oldGrabSensitivity = 3;
    d->oldHandleRadius = 3;
    d->oldCopyOffset = 10.0;

    if (d->config->hasGroup("Misc")) {
        KConfigGroup miscGroup = d->config->group("Misc");

        d->oldGrabSensitivity = miscGroup.readEntry("GrabSensitivity", d->oldGrabSensitivity);
        d->oldHandleRadius = miscGroup.readEntry("HandleRadius", d->oldHandleRadius);
        d->oldCopyOffset = miscGroup.readEntry("CopyOffset", d->oldCopyOffset);
    }

    KoUnit unit = d->doc->unit();

    QGroupBox* tmpQGroupBox = new QGroupBox(i18n("Misc"), this);

    QGridLayout* grid = new QGridLayout();
    grid->setSpacing(KDialog::spacingHint());
    grid->setMargin(KDialog::marginHint());

    QString unitType = KoUnit::unitName(unit);
    //#################"laurent
    //don't load unitType from config file because unit is
    //depend from kword file => unit can be different from config file

    grid->addWidget(new QLabel(i18n("Units:"), tmpQGroupBox), 0, 0);

    d->unit = new KComboBox(tmpQGroupBox);
    d->unit->addItems(KoUnit::listOfUnitName());
    grid->addWidget(d->unit, 0, 1);
    d->oldUnit = KoUnit::unit(unitType);
    d->unit->setCurrentIndex(d->oldUnit.indexInList());

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

    grid->addWidget(new QLabel(i18n("Copy offset:"), tmpQGroupBox), 3, 0);

    d->copyOffset = new KoUnitDoubleSpinBox(tmpQGroupBox);
    d->copyOffset->setMinMaxStep(-1000, 1000, 0.1);
    d->copyOffset->setValue(d->oldCopyOffset);
    d->copyOffset->setUnit(unit);
    grid->addWidget(d->copyOffset, 3, 1);

    grid->setRowStretch(4, 1);

    tmpQGroupBox->setLayout(grid);

    connect(d->unit, SIGNAL(activated(int)), SIGNAL(unitChanged(int)));
    connect(d->unit, SIGNAL(activated(int)), SLOT(slotUnitChanged(int)));
}

KoConfigMiscPage::~KoConfigMiscPage()
{
    delete d;
}

void KoConfigMiscPage::apply()
{
    KConfigGroup miscGroup = d->config->group("Misc");

    int currentUnit = d->unit->currentIndex();
    if ( currentUnit >= 0 && d->oldUnit.indexInList() != static_cast<uint>(currentUnit)) {
        d->oldUnit = KoUnit((KoUnit::Unit)currentUnit);
        d->doc->setUnit(d->oldUnit);
        miscGroup.writeEntry("Units", KoUnit::unitName(d->oldUnit));
    }

    uint currentHandleRadius = d->handleRadius->value();
    if (currentHandleRadius != d->oldHandleRadius) {
        miscGroup.writeEntry( "HandleRadius", currentHandleRadius );
    }

    uint currentGrabSensitivity = d->grabSensitivity->value();
    if (currentGrabSensitivity != d->oldGrabSensitivity) {
        miscGroup.writeEntry("GrabSensitivity", currentGrabSensitivity);
    }

    qreal currentCopyOffset = d->copyOffset->value();
    if (currentCopyOffset != d->oldCopyOffset) {
        miscGroup.writeEntry("CopyOffset", currentCopyOffset);
    }

    // FIXME how is the handle radius and grap sensitivitiy set?
}

void KoConfigMiscPage::slotDefault()
{
    d->unit->setCurrentIndex(0);
}

void KoConfigMiscPage::slotUnitChanged(int u)
{
    KoUnit unit = KoUnit((KoUnit::Unit) u);
    d->copyOffset->blockSignals(true);
    d->copyOffset->setUnit(unit);
    d->copyOffset->blockSignals(false);
}

#include <KoConfigMiscPage.moc>

