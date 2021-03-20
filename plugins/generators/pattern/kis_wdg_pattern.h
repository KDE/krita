/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_WDG_PATTERN_H
#define KIS_WDG_PATTERN_H

#include <kis_config_widget.h>

class Ui_WdgPatternOptions;

class KisWdgPattern : public KisConfigWidget
{
    Q_OBJECT
public:
    KisWdgPattern(QWidget* parent = 0);
    ~KisWdgPattern() override;
public:
    inline const Ui_WdgPatternOptions* widget() const {
        return m_widget;
    }
    void setConfiguration(const KisPropertiesConfigurationSP) override;
    KisPropertiesConfigurationSP configuration() const override;
private Q_SLOTS:

    void slotWidthChanged(double w);
    void slotHeightChanged(double h);
private:
    Ui_WdgPatternOptions* m_widget;
};

#endif

