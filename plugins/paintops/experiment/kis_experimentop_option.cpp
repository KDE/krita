/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_experimentop_option.h"
#include <klocalizedstring.h>

#include <brushengine/kis_paintop_lod_limitations.h>

#include "ui_wdgexperimentoptions.h"

class KisExperimentOpOptionsWidget: public QWidget, public Ui::WdgExperimentOptions
{
public:
    KisExperimentOpOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);

        speed->setRange(0.0, 100.0, 0);
        speed->setSuffix(QChar(Qt::Key_Percent));
        speed->setValue(42.0);
        speed->setSingleStep(1.0);

        smoothThreshold->setRange(0.0, 100.0, 0);
        smoothThreshold->setSuffix(i18n(" px"));
        smoothThreshold->setValue(20.0);
        smoothThreshold->setSingleStep(1.0);

        displaceStrength->setRange(0.0, 100.0, 0);
        displaceStrength->setSuffix(QChar(Qt::Key_Percent));
        displaceStrength->setValue(42.0);
        displaceStrength->setSingleStep(1.0);
    }
};

KisExperimentOpOption::KisExperimentOpOption()
    : KisPaintOpOption(KisPaintOpOption::GENERAL, false)
{
    setObjectName("KisExperimentOpOption");

    m_checkable = false;
    m_options = new KisExperimentOpOptionsWidget();

    connect(m_options->displaceCHBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->displaceStrength, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->speedCHBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->speed, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->smoothCHBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->smoothThreshold, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));

    connect(m_options->displaceStrength, SIGNAL(valueChanged(qreal)), SLOT(enableDisplacement(qreal)));
    connect(m_options->speed, SIGNAL(valueChanged(qreal)), SLOT(enableSpeed(qreal)));
    connect(m_options->smoothThreshold, SIGNAL(valueChanged(qreal)), SLOT(enableSmooth(qreal)));

    connect(m_options->windingFillCHBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->hardEdgeCHBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));

    connect(m_options->patternButton, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->solidColorButton, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));

    setConfigurationPage(m_options);
}

KisExperimentOpOption::~KisExperimentOpOption()
{
    delete m_options;
}

void KisExperimentOpOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    ExperimentOption op;

    op.isDisplacementEnabled = m_options->displaceCHBox->isChecked();
    op.displacement = m_options->displaceStrength->value();
    op.isSpeedEnabled = m_options->speedCHBox->isChecked();
    op.speed = m_options->speed->value();
    op.isSmoothingEnabled = m_options->smoothCHBox->isChecked();
    op.smoothing = m_options->smoothThreshold->value();
    op.windingFill = m_options->windingFillCHBox->isChecked();
    op.hardEdge = m_options->hardEdgeCHBox->isChecked();

    if (m_options->patternButton->isChecked()) {
        op.fillType = ExperimentFillType::Pattern;
    } else {
        op.fillType = ExperimentFillType::SolidColor;
    }

    op.writeOptionSetting(setting);
}

void KisExperimentOpOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    ExperimentOption op;
    op.readOptionSetting(setting);

    m_options->displaceStrength->setValue(op.displacement);
    m_options->speed->setValue(op.speed);
    m_options->smoothThreshold->setValue(op.smoothing);
    m_options->windingFillCHBox->setChecked(op.windingFill);
    m_options->hardEdgeCHBox->setChecked(op.hardEdge);

    m_options->speedCHBox->setChecked(op.isSpeedEnabled);
    m_options->smoothCHBox->setChecked(op.isSmoothingEnabled);
    m_options->displaceCHBox->setChecked(op.isDisplacementEnabled);


    if (op.fillType == ExperimentFillType::Pattern) {
        m_options->patternButton->setChecked(true);
    } else {
        m_options->solidColorButton->setChecked(true);
    }
}

inline void enableCheckBox(QCheckBox *checkBox, qreal sliderValue)
{
    checkBox->setChecked(sliderValue > 0);
}

void KisExperimentOpOption::enableSpeed(qreal value)
{
    enableCheckBox(m_options->speedCHBox, value);
}

void KisExperimentOpOption::enableSmooth(qreal value)
{
    enableCheckBox(m_options->smoothCHBox, value);
}

void KisExperimentOpOption::enableDisplacement(qreal value)
{
    enableCheckBox(m_options->displaceCHBox, value);
}

void KisExperimentOpOption::lodLimitations(KisPaintopLodLimitations *l) const
{
    if (m_options->displaceCHBox->isChecked()) {
        l->blockers << KoID("experiment-displacement", i18nc("PaintOp instant preview limitation", "Displacement Option"));
    }
}
