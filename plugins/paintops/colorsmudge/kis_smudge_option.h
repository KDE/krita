/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_SMUDGE_OPTION_H
#define KIS_SMUDGE_OPTION_H

#include "kis_rate_option.h"
#include <brushengine/kis_paint_information.h>
#include <kis_types.h>

// static const QString SMUDGE_MODE = "SmudgeMode";

class KisPropertiesConfiguration;
class KisPainter;

class KisSmudgeOption: public KisRateOption
{
public:
    KisSmudgeOption();

    enum Mode { SMEARING_MODE, DULLING_MODE };

    /**
     * Set the opacity of the painter based on the rate
     * and the curve (if checked)
     */
    void apply(KisPainter& painter, const KisPaintInformation& info, qreal scaleMin = 0.0, qreal scaleMax = 1.0, qreal multiplicator = 1.0) const;

    Mode getMode()          {
        return m_mode;
    }
    void setMode(Mode mode) {
        m_mode = mode;
    }

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

    bool getSmearAlpha() const;
    void setSmearAlpha(bool smearAlpha);

private:
    Mode m_mode;
    bool m_smearAlpha = true;
};

#endif // KIS_SMUDGE_OPTION_H
