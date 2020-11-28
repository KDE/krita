/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2019 Eoin O 'Neill <eoinoneill1991@gmail.com>
 * SPDX-FileCopyrightText: 2019 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_WDG_NOISE_H
#define KIS_WDG_NOISE_H

#include <kis_config_widget.h>
#include <kis_signal_compressor.h>

class Ui_WdgSimplexNoiseOptions;
class KisFilter;

class KisWdgSimplexNoise : public KisConfigWidget
{
    Q_OBJECT
public:
    KisWdgSimplexNoise(KisFilter* nfilter, QWidget* parent = 0);
    ~KisWdgSimplexNoise() override;
public:
    inline const Ui_WdgSimplexNoiseOptions* widget() const {
        return m_widget;
    }
    void setConfiguration(const KisPropertiesConfigurationSP) override;
    KisPropertiesConfigurationSP configuration() const override;

private:
    Ui_WdgSimplexNoiseOptions* m_widget;
    uint seed;
    KisSignalCompressor updateCompressor;

};

#endif

