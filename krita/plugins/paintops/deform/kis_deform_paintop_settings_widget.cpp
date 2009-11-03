/*
 *  Copyright (c) 2008 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "kis_deform_paintop_settings_widget.h"
#include "kis_deform_paintop_settings.h"

KisDeformPaintOpSettingsWidget:: KisDeformPaintOpSettingsWidget(QWidget* parent)
        : KisPaintOpSettingsWidget(parent)
{
    m_options = new Ui::WdgDeformOptions();
    m_options->setupUi(this);
}

KisDeformPaintOpSettingsWidget::~ KisDeformPaintOpSettingsWidget()
{
}

void  KisDeformPaintOpSettingsWidget::setConfiguration(const KisPropertiesConfiguration * config)
{
    m_options->deformRadiusSPBox->setValue(config->getInt("radius"));
    m_options->deformAmountSPBox->setValue(config->getDouble("deform_amount"));
    m_options->interpolationChBox->setChecked(config->getBool("bilinear"));
    m_options->addPaintChBox->setChecked(config->getBool("use_movement_paint"));
    m_options->useCounter->setChecked(config->getBool("use_counter"));
    m_options->useOldData->setChecked(config->getBool("use_old_data"));
    m_options->spacingKDNumInp->setValue(config->getDouble("spacing"));

    int deformAction = config->getInt("deform_action");
    if (deformAction == 1) {
        m_options->growBtn->setChecked(true);
    } else if (deformAction == 2) {
        m_options->shrinkBtn->setChecked(true);
    } else if (deformAction == 3) {
        m_options->swirlCWBtn->setChecked(true);
    } else if (deformAction == 4) {
        m_options->swirlCCWBtn->setChecked(true);
    } else if (deformAction == 5) {
        m_options->moveBtn->setChecked(true);
    } else if (deformAction == 6) {
        m_options->lensBtn->setChecked(true);
    } else if (deformAction == 7) {
        m_options->lensOutBtn->setChecked(true);
    } else if (deformAction == 8) {
        m_options->colorBtn->setChecked(true);
    }
}

KisPropertiesConfiguration*  KisDeformPaintOpSettingsWidget::configuration() const
{
    KisDeformPaintOpSettings* settings = new KisDeformPaintOpSettings();
    settings->setOptionsWidget(const_cast<KisDeformPaintOpSettingsWidget*>(this));
    return settings;
}

void KisDeformPaintOpSettingsWidget::writeConfiguration(KisPropertiesConfiguration* config) const
{
    config->setProperty("paintop", "deformbrush");  // XXX: make this a const id string
    config->setProperty("radius", radius());
    config->setProperty("deform_amount", deformAmount());
    config->setProperty("deform_action", deformAction());
    config->setProperty("bilinear", bilinear());
    config->setProperty("use_movement_paint", useMovementPaint());
    config->setProperty("use_counter", useCounter());
    config->setProperty("use_old_data", useOldData());
    config->setProperty("spacing", spacing());
}

int  KisDeformPaintOpSettingsWidget::radius() const
{
    return m_options->deformRadiusSPBox->value();
}

double  KisDeformPaintOpSettingsWidget::deformAmount() const
{
    return m_options->deformAmountSPBox->value();
}

int  KisDeformPaintOpSettingsWidget::deformAction() const
{
    //TODO: make it nicer using enums or something
    if (m_options->growBtn->isChecked()) {
        return 1;
    } else if (m_options->shrinkBtn->isChecked()) {
        return 2;
    } else if (m_options->swirlCWBtn->isChecked()) {
        return 3;
    } else if (m_options->swirlCCWBtn->isChecked()) {
        return 4;
    } else if (m_options->moveBtn->isChecked()) {
        return 5;
    } else if (m_options->lensBtn->isChecked()) {
        return 6;
    } else if (m_options->lensOutBtn->isChecked()) {
        return 7;
    } else if (m_options->colorBtn->isChecked()) {
        return 8;
    } else {
        return -1;
    }
}

bool  KisDeformPaintOpSettingsWidget::bilinear() const
{
    return m_options->interpolationChBox->isChecked();
}

bool KisDeformPaintOpSettingsWidget::useMovementPaint() const
{
    return m_options->addPaintChBox->isChecked();
}

bool KisDeformPaintOpSettingsWidget::useCounter() const
{
    return m_options->useCounter->isChecked();
}

bool KisDeformPaintOpSettingsWidget::useOldData() const
{
    return m_options->useOldData->isChecked();
}

qreal KisDeformPaintOpSettingsWidget::spacing() const
{
    return m_options->spacingKDNumInp->value();
}


void KisDeformPaintOpSettingsWidget::setRadius(int radius) const
{
    m_options->deformRadiusSPBox->setValue( radius );
}
