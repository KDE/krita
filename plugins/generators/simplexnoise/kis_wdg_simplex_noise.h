/*
 * KDE. Krita Project.
 *
 * Copyright (c) 2019 Eoin O'Neill <eoinoneill1991@gmail.com>
 * Copyright (c) 2019 Emmet O'Neill <emmetoneill.pdx@gmail.com>
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

