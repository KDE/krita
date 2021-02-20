/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_WDG_COLOR_H
#define KIS_WDG_COLOR_H

#include <kis_config_widget.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

class Ui_WdgColorOptions;

class KisWdgColor : public KisConfigWidget
{
    Q_OBJECT
public:
    KisWdgColor(QWidget* parent = 0, const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8());
    ~KisWdgColor() override;
public:
    inline const Ui_WdgColorOptions* widget() const {
        return m_widget;
    }
    void setConfiguration(const KisPropertiesConfigurationSP) override;
    KisPropertiesConfigurationSP configuration() const override;
private:
    Ui_WdgColorOptions* m_widget;
    const KoColorSpace *m_cs;
};

#endif

