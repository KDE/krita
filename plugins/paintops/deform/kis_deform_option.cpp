/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_deform_option.h"
#include "ui_wdgdeformoptions.h"

#include <brushengine/kis_paintop_lod_limitations.h>


class KisDeformOptionsWidget: public QWidget, public Ui::WdgDeformOptions
{
public:
    KisDeformOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);

        deformAmount->setRange(0.0, 1.0, 2);
        deformAmount->setValue(0.20);
    }
};

KisDeformOption::KisDeformOption()
    : KisPaintOpOption(KisPaintOpOption::COLOR, false)
{
    setObjectName("KisDeformOption");

    m_checkable = false;
    m_options = new KisDeformOptionsWidget();

    connect(m_options->deformAmount, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->interpolationChBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->useCounter, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->useOldData, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));

    connect(m_options->growBtn, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));
    connect(m_options->shrinkBtn, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));
    connect(m_options->swirlCWBtn, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));
    connect(m_options->swirlCCWBtn, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));
    connect(m_options->moveBtn, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));
    connect(m_options->lensBtn, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));
    connect(m_options->lensOutBtn, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));
    connect(m_options->colorBtn, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));

    setConfigurationPage(m_options);
}

KisDeformOption::~KisDeformOption()
{
    delete m_options;
}

void  KisDeformOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    DeformOption op;
    op.readOptionSetting(setting);


    m_options->deformAmount->setValue(op.deform_amount);
    m_options->interpolationChBox->setChecked(op.deform_use_bilinear);
    m_options->useCounter->setChecked(op.deform_use_counter);
    m_options->useOldData->setChecked(op.deform_use_old_data);

    int deformAction = op.deform_action;
    if (deformAction == 1) {
        m_options->growBtn->setChecked(true);
    }
    else if (deformAction == 2) {
        m_options->shrinkBtn->setChecked(true);
    }
    else if (deformAction == 3) {
        m_options->swirlCWBtn->setChecked(true);
    }
    else if (deformAction == 4) {
        m_options->swirlCCWBtn->setChecked(true);
    }
    else if (deformAction == 5) {
        m_options->moveBtn->setChecked(true);
    }
    else if (deformAction == 6) {
        m_options->lensBtn->setChecked(true);
    }
    else if (deformAction == 7) {
        m_options->lensOutBtn->setChecked(true);
    }
    else if (deformAction == 8) {
        m_options->colorBtn->setChecked(true);
    }
}


void KisDeformOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    DeformOption op;

    op.deform_amount = m_options->deformAmount->value();
    op.deform_action = deformAction();
    op.deform_use_bilinear = m_options->interpolationChBox->isChecked();
    op.deform_use_counter = m_options->useCounter->isChecked();
    op.deform_use_old_data = m_options->useOldData->isChecked();

    op.writeOptionSetting(setting);
}

void KisDeformOption::lodLimitations(KisPaintopLodLimitations *l) const
{
    l->blockers << KoID("deform-brush", i18nc("PaintOp instant preview limitation", "Deform Brush (unsupported)"));
}

int  KisDeformOption::deformAction() const
{
    //TODO: make it nicer using enums or something
    if (m_options->growBtn->isChecked()) {
        return 1;
    }
    else if (m_options->shrinkBtn->isChecked()) {
        return 2;
    }
    else if (m_options->swirlCWBtn->isChecked()) {
        return 3;
    }
    else if (m_options->swirlCCWBtn->isChecked()) {
        return 4;
    }
    else if (m_options->moveBtn->isChecked()) {
        return 5;
    }
    else if (m_options->lensBtn->isChecked()) {
        return 6;
    }
    else if (m_options->lensOutBtn->isChecked()) {
        return 7;
    }
    else if (m_options->colorBtn->isChecked()) {
        return 8;
    }
    else {
        return -1;
    }
}

