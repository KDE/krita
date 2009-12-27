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

#include <kcolorbutton.h>
#include <kdialog.h>

#include <QCheckBox>
#include <QGroupBox>
#include <QGridLayout>
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

    d->config = d->doc->componentData().config();

    KoUnit unit = d->doc->unit();
    KoGridData &gd = d->doc->gridData();

    QGroupBox* generalGrp = new QGroupBox(i18n("Grid"), this);
    QGridLayout *layoutGeneral = new QGridLayout(generalGrp);
    QLabel * showGridLabel = new QLabel(i18n("Show grid:"), generalGrp);
    d->gridChBox = new QCheckBox("", generalGrp);
    d->gridChBox->setChecked(gd.showGrid());
    QLabel * snapGridLabel = new QLabel(i18n("Snap to grid:"), generalGrp);
    d->snapChBox = new QCheckBox("", generalGrp);
    d->snapChBox->setChecked(gd.snapToGrid());
    QLabel* gridColorLbl = new QLabel(i18n("Grid color:"), generalGrp);
    d->gridColorBtn = new KColorButton(gd.gridColor(), generalGrp);
    gridColorLbl->setBuddy(d->gridColorBtn);
    layoutGeneral->addWidget(showGridLabel, 0, 0);
    layoutGeneral->addWidget(d->gridChBox, 0, 1);
    layoutGeneral->addWidget(snapGridLabel, 1, 0);
    layoutGeneral->addWidget(d->snapChBox, 1, 1);
    layoutGeneral->addWidget(gridColorLbl, 2, 0);
    layoutGeneral->addWidget(d->gridColorBtn, 2, 1);

    QGroupBox* spacingGrp = new QGroupBox(i18n("Spacing"), this);
    QHBoxLayout *hboxLayout = new QHBoxLayout(spacingGrp);
    QGridLayout* layoutSpacingGrp = new QGridLayout();
    QLabel* spaceHorizLbl = new QLabel(i18nc("Horizontal grid spacing", "&Horizontal:"));
    d->spaceHorizUSpin = new KoUnitDoubleSpinBox(spacingGrp);
    d->spaceHorizUSpin->setMinMaxStep(0.0, 1000, 0.1);
    d->spaceHorizUSpin->setUnit(unit);
    d->spaceHorizUSpin->changeValue(gd.gridX());
    spaceHorizLbl->setBuddy(d->spaceHorizUSpin);
    QLabel* spaceVertLbl = new QLabel(i18nc("Vertical grid spacing", "&Vertical:"));
    d->spaceVertUSpin = new KoUnitDoubleSpinBox(spacingGrp);
    d->spaceVertUSpin->setMinMaxStep(0.0, 1000, 0.1);
    d->spaceVertUSpin->setUnit(unit);
    d->spaceVertUSpin->changeValue(gd.gridY());
    spaceVertLbl->setBuddy(d->spaceVertUSpin);
    layoutSpacingGrp->addWidget(spaceHorizLbl, 0, 0);
    layoutSpacingGrp->addWidget(d->spaceHorizUSpin, 0, 1);
    layoutSpacingGrp->addWidget(spaceVertLbl, 1, 0);
    layoutSpacingGrp->addWidget(d->spaceVertUSpin, 1, 1);
    hboxLayout->addLayout(layoutSpacingGrp);
    d->bnLinkSpacing = new KoAspectButton(spacingGrp);
    d->bnLinkSpacing->setKeepAspectRatio(gd.gridX() == gd.gridY());
    hboxLayout->addWidget(d->bnLinkSpacing);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QGridLayout* gl = new QGridLayout();
    gl->setSpacing(KDialog::spacingHint());
    gl->setMargin(KDialog::marginHint());
    gl->addWidget(generalGrp, 0, 0, 1, 2);
    gl->addItem(new QSpacerItem(0, 0), 1, 1);
    gl->addWidget(spacingGrp, 2, 0, 1, 2);
    gl->addItem(new QSpacerItem(0, 0), 4, 0, 1, 2);
    mainLayout->addLayout(gl);
    mainLayout->addStretch();

    setValuesFromGrid(d->doc->gridData());

    connect(d->spaceHorizUSpin, SIGNAL(valueChangedPt(qreal)),this,SLOT(spinBoxHSpacingChanged(qreal)));
    connect(d->spaceVertUSpin, SIGNAL(valueChangedPt(qreal)),this,SLOT(spinBoxVSpacingChanged(qreal)));
}

KoConfigGridPage::~KoConfigGridPage()
{
    delete d;
}

void KoConfigGridPage::slotUnitChanged(int u)
{
    KoUnit unit = KoUnit((KoUnit::Unit) u);
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
