/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_particleop_option.h"
#include <klocalizedstring.h>

#include <QWidget>
#include <QRadioButton>

#include <brushengine/kis_paintop_lod_limitations.h>

#include "ui_wdgparticleoptions.h"

class KisParticleOpOptionsWidget: public QWidget, public Ui::WdgParticleOptions
{
public:
    KisParticleOpOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);
    }
};

KisParticleOpOption::KisParticleOpOption()
    : KisPaintOpOption(i18n("Brush size"), KisPaintOpOption::GENERAL, false)
{
    setObjectName("KisParticleOpOption");

    m_checkable = false;
    m_options = new KisParticleOpOptionsWidget();

    m_options->gravSPBox->setRange(-1.0, 1.0, 3);
    m_options->gravSPBox->setSingleStep(0.001);
    m_options->gravSPBox->setValue(0.989);

    m_options->dySPBox->setRange(-10.0, 10.0, 2);
    m_options->dySPBox->setSingleStep(0.01);
    m_options->dySPBox->setValue(0.3);

    m_options->dxSPBox->setRange(-10.0, 10.0, 2);
    m_options->dxSPBox->setSingleStep(0.01);
    m_options->dxSPBox->setValue(0.3);

    m_options->weightSPBox->setRange(0.01, 1.0, 2);
    m_options->weightSPBox->setSingleStep(0.01);
    m_options->weightSPBox->setValue(0.2);

    m_options->particleSpinBox->setRange(1.0, 500.0, 0);
    m_options->particleSpinBox->setValue(50);
    m_options->particleSpinBox->setExponentRatio(3.0);

    m_options->itersSPBox->setRange(1, 200, 0);
    m_options->itersSPBox->setValue(10);


    connect(m_options->particleSpinBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->itersSPBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->gravSPBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->weightSPBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->dxSPBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->dySPBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));

    setConfigurationPage(m_options);
}

KisParticleOpOption::~KisParticleOpOption()
{
    delete m_options;
}

int KisParticleOpOption::particleCount() const
{
    return m_options->particleSpinBox->value();
}

qreal KisParticleOpOption::weight() const
{
    return m_options->weightSPBox->value();
}


QPointF KisParticleOpOption::scale() const
{
    return QPointF(m_options->dxSPBox->value(), m_options->dySPBox->value());
}


int KisParticleOpOption::iterations() const
{
    return m_options->itersSPBox->value();
}



qreal KisParticleOpOption::gravity() const
{
    return m_options->gravSPBox->value();
}

void KisParticleOpOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    ParticleOption op;

    op.particle_count = particleCount();
    op.particle_iterations = iterations();
    op.particle_gravity = gravity();
    op.particle_weight = weight();
    op.particle_scale_x = scale().x();
    op.particle_scale_y = scale().y();

    op.writeOptionSetting(setting);
}

void KisParticleOpOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    ParticleOption op;
    op.readOptionSetting(setting);

    m_options->particleSpinBox->setValue(op.particle_count);
    m_options->itersSPBox->setValue(op.particle_iterations);
    m_options->gravSPBox->setValue(op.particle_gravity);
    m_options->weightSPBox->setValue(op.particle_weight);
    m_options->dxSPBox->setValue(op.particle_scale_x);
    m_options->dySPBox->setValue(op.particle_scale_y);
}

void KisParticleOpOption::lodLimitations(KisPaintopLodLimitations *l) const
{
    l->blockers << KoID("particle-brush", i18nc("PaintOp instant preview limitation", "Particle Brush (not supported)"));
}
