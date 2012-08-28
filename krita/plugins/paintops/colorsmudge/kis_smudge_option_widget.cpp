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

KisSmudgeOptionWidget::KisSmudgeOptionWidget(const QString& label, const QString& sliderLabel, const QString& name, bool checked):
    KisCurveOptionWidget(new KisSmudgeOption(name, label, checked))
{
    mCbSmudgeMode = new QComboBox();
    mCbSmudgeMode->addItem(i18n("Smearing"), KisSmudgeOption::SMEARING_MODE);
    mCbSmudgeMode->addItem(i18n("Dulling") , KisSmudgeOption::DULLING_MODE);
    
    QHBoxLayout* h = new QHBoxLayout();
    h->addWidget(new QLabel(i18n("Smudge Mode")));
    h->addWidget(mCbSmudgeMode, 1);
    
    QVBoxLayout* v = new QVBoxLayout();
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
    emit sigSettingChanged();
}

void KisSmudgeOptionWidget::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    KisCurveOptionWidget::readOptionSetting(setting);
    
    KisSmudgeOption::Mode mode = static_cast<KisSmudgeOption*>(curveOption())->getMode();
    
    mCbSmudgeMode->blockSignals(true);
    mCbSmudgeMode->setCurrentIndex(mode == KisSmudgeOption::SMEARING_MODE ? 0 : 1);
    mCbSmudgeMode->blockSignals(false);
}
