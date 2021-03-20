/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_DEFORM_OPTION_H
#define KIS_DEFORM_OPTION_H

#include <kis_paintop_option.h>

class KisDeformOptionsWidget;
class KisPaintopLodLimitations;

const QString DEFORM_AMOUNT = "Deform/deformAmount";
const QString DEFORM_ACTION = "Deform/deformAction";
const QString DEFORM_USE_BILINEAR = "Deform/bilinear";
const QString DEFORM_USE_MOVEMENT_PAINT = "Deform/useMovementPaint";
const QString DEFORM_USE_COUNTER = "Deform/useCounter";
const QString DEFORM_USE_OLD_DATA = "Deform/useOldData";

class KisDeformOption : public KisPaintOpOption
{
public:
    KisDeformOption();
    ~KisDeformOption() override;

    double deformAmount() const;
    int deformAction() const;
    bool bilinear() const;
    bool useMovementPaint() const;
    bool useCounter() const;
    bool useOldData() const;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

    void lodLimitations(KisPaintopLodLimitations *l) const override;

private:
    KisDeformOptionsWidget * m_options;

};

struct DeformOption {
    qreal deform_amount;
    bool deform_use_bilinear;
    bool deform_use_counter;
    bool deform_use_old_data;
    int deform_action;

    void readOptionSetting(const KisPropertiesConfigurationSP config) {
        deform_amount = config->getDouble(DEFORM_AMOUNT);
        deform_use_bilinear = config->getBool(DEFORM_USE_BILINEAR);
        deform_use_counter = config->getBool(DEFORM_USE_COUNTER);
        deform_use_old_data = config->getBool(DEFORM_USE_OLD_DATA);
        deform_action = config->getInt(DEFORM_ACTION);
    }


    void writeOptionSetting(KisPropertiesConfigurationSP config) const {
        config->setProperty(DEFORM_AMOUNT, deform_amount);
        config->setProperty(DEFORM_ACTION, deform_action);
        config->setProperty(DEFORM_USE_BILINEAR, deform_use_bilinear);
        config->setProperty(DEFORM_USE_COUNTER, deform_use_counter);
        config->setProperty(DEFORM_USE_OLD_DATA, deform_use_old_data);
    }
};

#endif
