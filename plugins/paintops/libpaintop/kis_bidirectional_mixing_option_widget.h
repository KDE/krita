/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2008 Emanuele Tamponi <emanuele@valinor.it>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_BIDIRECTIONAL_MIXING_OPTION_WIDGET_H
#define KIS_BIDIRECTIONAL_MIXING_OPTION_WIDGET_H

#include "kis_paintop_option.h"
#include <kis_types.h>
#include <kritapaintop_export.h>

class KisPropertiesConfiguration;
class QLabel;

const QString BIDIRECTIONAL_MIXING_ENABLED = "BidirectionalMixing/Enabled";

/**
 * The bidirectional mixing option uses the painterly framework to
 * implement bidirectional paint mixing (that is, paint on the canvas
 * dirties the brush, and the brush mixes its color with that on the
 * canvas.
 *
 * Taken from the complex paintop
 */
class PAINTOP_EXPORT KisBidirectionalMixingOptionWidget : public KisPaintOpOption
{
public:
    KisBidirectionalMixingOptionWidget();
    ~KisBidirectionalMixingOptionWidget() override;

    ///Reimplemented
    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;

    ///Reimplemented
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    QLabel * m_optionWidget;
};

#endif
