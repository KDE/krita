/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_WDG_WAVE_H
#define KIS_WDG_WAVE_H

#include <kis_config_widget.h>

class Ui_WdgWaveOptions;
class KisFilter;

class KisWdgWave : public KisConfigWidget
{
    Q_OBJECT
public:
    KisWdgWave(KisFilter* nfilter, QWidget* parent = 0);
    ~KisWdgWave() override;
public:
    inline const Ui_WdgWaveOptions* widget() const {
        return m_widget;
    }
    void setConfiguration(const KisPropertiesConfigurationSP) override;
    KisPropertiesConfigurationSP configuration() const override;
private:
    Ui_WdgWaveOptions* m_widget;
};

#endif

