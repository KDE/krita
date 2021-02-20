/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_WDG_NOISE_H
#define KIS_WDG_NOISE_H

#include <kis_config_widget.h>

class Ui_WdgNoiseOptions;
class KisFilter;

class KisWdgNoise : public KisConfigWidget
{
    Q_OBJECT
public:
    KisWdgNoise(KisFilter* nfilter, QWidget* parent = 0);
    ~KisWdgNoise() override;
public:
    inline const Ui_WdgNoiseOptions* widget() const {
        return m_widget;
    }
    void setConfiguration(const KisPropertiesConfigurationSP) override;
    KisPropertiesConfigurationSP configuration() const override;
private:
    Ui_WdgNoiseOptions* m_widget;
    int m_seedThreshold, m_seedRed, m_seedGreen, m_seedBlue;
};

#endif

