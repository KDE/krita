/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Silvio Heinrich <plassy@web.de>
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

#include <klocale.h>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>

#include "kis_smudge_option_widget.h"
#include "kis_smudge_option.h"


KisSmudgeOptionWidget::KisSmudgeOptionWidget()
    : KisCurveOptionWidget(new KisSmudgeOption(), i18n("0.0"), i18n("1.0"))
{
    setObjectName("KisSmudgeOptionWidget");

    mCbSmudgeMode = new QComboBox();
    mCbSmudgeMode->addItem(i18n("Smearing"), KisSmudgeOption::SMEARING_MODE);
    mCbSmudgeMode->addItem("dulling-placeholder" , KisSmudgeOption::DULLING_MODE);

    // the text for the second item is initialized here
    updateBrushPierced(false);

    QHBoxLayout* h = new QHBoxLayout();
    h->addWidget(new QLabel(i18n("Smudge mode:")));
    h->addWidget(mCbSmudgeMode, 1);

    QVBoxLayout* v = new QVBoxLayout();
    v->setMargin(0);
    QWidget*     w = new QWidget();

    v->addLayout(h);
    v->addWidget(curveWidget());
    w->setLayout(v);

    KisCurveOptionWidget::setConfigurationPage(w);

    connect(mCbSmudgeMode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCurrentIndexChanged(int)));
}

void KisSmudgeOptionWidget::slotCurrentIndexChanged(int index)
{
    static_cast<KisSmudgeOption*>(curveOption())->setMode((KisSmudgeOption::Mode)index);
    emitSettingChanged();
}

void KisSmudgeOptionWidget::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    KisCurveOptionWidget::readOptionSetting(setting);

    KisSmudgeOption::Mode mode = static_cast<KisSmudgeOption*>(curveOption())->getMode();
    mCbSmudgeMode->setCurrentIndex(mode == KisSmudgeOption::SMEARING_MODE ? 0 : 1);
}

void KisSmudgeOptionWidget::updateBrushPierced(bool pierced)
{
    QString dullingText = i18n("Dulling");
    QString toolTip;

    if (pierced) {
        dullingText += i18n(" (caution, pierced brush!)");
        toolTip = i18nc("@info:tooltip", "This brush has transparent pixels in its center. \"Dulling\" mode may give unstable results. Consider using \"Smearing\" mode instead.");
    }

    mCbSmudgeMode->setItemText(1, dullingText);
    mCbSmudgeMode->setToolTip(toolTip);
}
