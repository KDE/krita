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

#include "KoConfigGridPage.h"

#include <KoDocument.h>
#include <KoGridData.h>
#include <KoUnitDoubleSpinBox.h>
#include <KoAspectButton.h>
#include <KoPart.h>

#include <kcolorbutton.h>
#include <kdialog.h>

#include <QCheckBox>
#include <QGroupBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>

class KoConfigGridPage::Private
{
public:
    Private(KoDocument* doc)
    : doc(doc)
    {}

    KoDocument *doc;

    KoUnitDoubleSpinBox* spaceHorizUSpin;
    KoUnitDoubleSpinBox* spaceVertUSpin;
    QCheckBox* gridChBox;
    QCheckBox* snapChBox;
    KColorButton* gridColorBtn;
    KSharedConfigPtr config;
    KoAspectButton * bnLinkSpacing;
};

KoConfigGridPage::KoConfigGridPage(KoDocument* doc, char* name)
: d(new Private(doc))
{
    setObjectName(name);

    d->config = d->doc->documentPart()->componentData().config();

    KoUnit unit = d->doc->unit();
    KoGridData &gd = d->doc->gridData();

    QGroupBox* generalGrp = new QGroupBox(i18n("Grid"), this);
    QFormLayout *layoutGeneral = new QFormLayout(generalGrp);
    d->gridChBox = new QCheckBox(generalGrp);
    d->gridChBox->setChecked(gd.showGrid());
    d->snapChBox = new QCheckBox(generalGrp);
    d->snapChBox->setChecked(gd.snapToGrid());
    d->gridColorBtn = new KColorButton(gd.gridColor(), generalGrp);
    d->gridColorBtn->setAlphaChannelEnabled(true);
    layoutGeneral->addRow(i18n("Show grid:"), d->gridChBox);
    layoutGeneral->addRow(i18n("Snap to grid:"), d->snapChBox);
    layoutGeneral->addRow(i18n("Grid color:"), d->gridColorBtn);

    QGroupBox* spacingGrp = new QGroupBox(i18n("Spacing"), this);
    QHBoxLayout *hboxLayout = new QHBoxLayout(spacingGrp);
    QFormLayout *layoutSpacingGrp = new QFormLayout();
    d->spaceHorizUSpin = new KoUnitDoubleSpinBox(spacingGrp);
    d->spaceHorizUSpin->setMinMaxStep(0.0, 1000, 0.1);
    d->spaceHorizUSpin->setUnit(unit);
    d->spaceHorizUSpin->changeValue(gd.gridX());
    d->spaceVertUSpin = new KoUnitDoubleSpinBox(spacingGrp);
    d->spaceVertUSpin->setMinMaxStep(0.0, 1000, 0.1);
    d->spaceVertUSpin->setUnit(unit);
    d->spaceVertUSpin->changeValue(gd.gridY());
    layoutSpacingGrp->addRow(i18nc("Horizontal grid spacing", "&Horizontal:"), d->spaceHorizUSpin);
    layoutSpacingGrp->addRow(i18nc("Vertical grid spacing", "&Vertical:"), d->spaceVertUSpin);
    hboxLayout->addLayout(layoutSpacingGrp);
    d->bnLinkSpacing = new KoAspectButton(spacingGrp);
    d->bnLinkSpacing->setKeepAspectRatio(gd.gridX() == gd.gridY());
    hboxLayout->addWidget(d->bnLinkSpacing);
    hboxLayout->addStretch();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->addWidget(generalGrp);
    mainLayout->addWidget(spacingGrp);
    mainLayout->addStretch();

    setValuesFromGrid(d->doc->gridData());

    connect(d->spaceHorizUSpin, SIGNAL(valueChangedPt(qreal)),this,SLOT(spinBoxHSpacingChanged(qreal)));
    connect(d->spaceVertUSpin, SIGNAL(valueChangedPt(qreal)),this,SLOT(spinBoxVSpacingChanged(qreal)));
}

KoConfigGridPage::~KoConfigGridPage()
{
    delete d;
}

void KoConfigGridPage::slotUnitChanged(const KoUnit &unit)
{
    d->spaceHorizUSpin->blockSignals(true);
    d->spaceVertUSpin->blockSignals(true);
    d->spaceHorizUSpin->setUnit(unit);
    d->spaceVertUSpin->setUnit(unit);
    d->spaceHorizUSpin->blockSignals(false);
    d->spaceVertUSpin->blockSignals(false);
}

void KoConfigGridPage::apply()
{
    KoGridData &gd = d->doc->gridData();
    gd.setGrid(d->spaceHorizUSpin->value(), d->spaceVertUSpin->value());
    gd.setShowGrid(d->gridChBox->isChecked());
    gd.setSnapToGrid(d->snapChBox->isChecked());
    gd.setGridColor(d->gridColorBtn->color());

    KConfigGroup gridGroup = d->config->group("Grid");
    gridGroup.writeEntry("SpacingX", gd.gridX());
    gridGroup.writeEntry("SpacingY", gd.gridY());
    gridGroup.writeEntry("Color", gd.gridColor());
}

void KoConfigGridPage::slotDefault()
{
    KoGridData defGrid;
    setValuesFromGrid(defGrid);
}

void KoConfigGridPage::setValuesFromGrid(const KoGridData &grid)
{
    d->spaceHorizUSpin->changeValue(grid.gridX());
    d->spaceVertUSpin->changeValue(grid.gridY());

    d->gridChBox->setChecked(grid.showGrid());
    d->snapChBox->setChecked(grid.snapToGrid());
    d->gridColorBtn->setColor(grid.gridColor());
}

void KoConfigGridPage::spinBoxHSpacingChanged(qreal v)
{
    if (d->bnLinkSpacing->keepAspectRatio())
        d->spaceVertUSpin->changeValue(v);
}

void KoConfigGridPage::spinBoxVSpacingChanged(qreal v)
{
    if (d->bnLinkSpacing->keepAspectRatio())
        d->spaceHorizUSpin->changeValue(v);
}

#include <KoConfigGridPage.moc>
