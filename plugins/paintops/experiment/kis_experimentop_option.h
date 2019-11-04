/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#ifndef KIS_EXPERIMENTOP_OPTION_H
#define KIS_EXPERIMENTOP_OPTION_H

#include <kis_paintop_option.h>
class KisPaintopLodLimitations;

const QString EXPERIMENT_DISPLACEMENT_ENABLED = "Experiment/displacementEnabled";
const QString EXPERIMENT_DISPLACEMENT_VALUE = "Experiment/displacement";
const QString EXPERIMENT_SMOOTHING_ENABLED = "Experiment/smoothing";
const QString EXPERIMENT_SMOOTHING_VALUE = "Experiment/smoothingValue";
const QString EXPERIMENT_SPEED_ENABLED = "Experiment/speedEnabled";
const QString EXPERIMENT_SPEED_VALUE = "Experiment/speed";
const QString EXPERIMENT_WINDING_FILL = "Experiment/windingFill";
const QString EXPERIMENT_HARD_EDGE = "Experiment/hardEdge";
const QString EXPERIMENT_FILL_TYPE = "Experiment/fillType";


enum ExperimentFillType {
    SolidColor,
    Pattern
};


class KisExperimentOpOptionsWidget;

class KisExperimentOpOption : public KisPaintOpOption
{
    Q_OBJECT
public:
    KisExperimentOpOption();
    ~KisExperimentOpOption() override;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

    void lodLimitations(KisPaintopLodLimitations *l) const override;

private Q_SLOTS:
    void enableSpeed(qreal value);
    void enableSmooth(qreal value);
    void enableDisplacement(qreal value);

private:
    KisExperimentOpOptionsWidget * m_options;

};

class ExperimentOption
{

public:
    bool isDisplacementEnabled;
    qreal displacement;
    bool isSpeedEnabled;
    qreal speed;
    bool isSmoothingEnabled;
    qreal smoothing;
    bool windingFill;
    bool hardEdge;
    ExperimentFillType fillType;

    void readOptionSetting(const KisPropertiesConfigurationSP setting) {
        isDisplacementEnabled = setting->getBool(EXPERIMENT_DISPLACEMENT_ENABLED);
        displacement = setting->getDouble(EXPERIMENT_DISPLACEMENT_VALUE, 50.0);
        isSpeedEnabled = setting->getBool(EXPERIMENT_SPEED_ENABLED);
        speed = setting->getDouble(EXPERIMENT_SPEED_VALUE, 50.0);
        isSmoothingEnabled = setting->getBool(EXPERIMENT_SMOOTHING_ENABLED);
        smoothing = setting->getDouble(EXPERIMENT_SMOOTHING_VALUE, 20.0);
        windingFill = setting->getBool(EXPERIMENT_WINDING_FILL);
        hardEdge = setting->getBool(EXPERIMENT_HARD_EDGE);
        fillType = (ExperimentFillType)setting->getInt(EXPERIMENT_FILL_TYPE, 0); // default to solid color
    }

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const {
        setting->setProperty(EXPERIMENT_DISPLACEMENT_ENABLED, isDisplacementEnabled);
        setting->setProperty(EXPERIMENT_DISPLACEMENT_VALUE, displacement);
        setting->setProperty(EXPERIMENT_SPEED_ENABLED, isSpeedEnabled);
        setting->setProperty(EXPERIMENT_SPEED_VALUE, speed);
        setting->setProperty(EXPERIMENT_SMOOTHING_ENABLED, isSmoothingEnabled);
        setting->setProperty(EXPERIMENT_SMOOTHING_VALUE, smoothing);
        setting->setProperty(EXPERIMENT_WINDING_FILL, windingFill);
        setting->setProperty(EXPERIMENT_HARD_EDGE, hardEdge);
        setting->setProperty(EXPERIMENT_FILL_TYPE, fillType);
    }



};

#endif
