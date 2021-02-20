/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_WDG_RANDOMPICK_H
#define KIS_WDG_RANDOMPICK_H

#include <kis_config_widget.h>

class Ui_WdgRandomPickOptions;
class KisFilter;

class KisWdgRandomPick : public KisConfigWidget
{
    Q_OBJECT
public:
    KisWdgRandomPick(KisFilter* nfilter, QWidget* parent = 0);
    ~KisWdgRandomPick() override;
public:
    inline const Ui_WdgRandomPickOptions* widget() const {
        return m_widget;
    }
    void setConfiguration(const KisPropertiesConfigurationSP) override;
    KisPropertiesConfigurationSP configuration() const override;
private:
    Ui_WdgRandomPickOptions* m_widget;
    int m_seedH, m_seedV, m_seedThreshold;
};

#endif

