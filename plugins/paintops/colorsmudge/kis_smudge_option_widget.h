/*
    SPDX-FileCopyrightText: 2012 Silvio Heinrich <plassy@web.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KIS_SMUDGE_OPTION_WIDGET_H
#define KIS_SMUDGE_OPTION_WIDGET_H

#include <kis_curve_option_widget.h>


class QComboBox;
class QCheckBox;

class KisSmudgeOptionWidget: public KisCurveOptionWidget
{
    Q_OBJECT

public:
    KisSmudgeOptionWidget();

    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

    void updateBrushPierced(bool pierced);

    void setUseNewEngineCheckboxEnabled(bool enabled);

    void setUseNewEngine(bool useNew);

private Q_SLOTS:
    void slotCurrentIndexChanged(int index);
    void slotSmearAlphaChanged(bool value);
    void slotUseNewEngineChanged(bool value);

private:
    QComboBox* mCbSmudgeMode;
    QCheckBox *mChkSmearAlpha;
    QCheckBox* mChkUseNewEngine;
};

#endif // KIS_SMUDGE_OPTION_WIDGET_H
