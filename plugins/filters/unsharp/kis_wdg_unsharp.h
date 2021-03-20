/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_WDG_UNSHARP_H_
#define _KIS_WDG_UNSHARP_H_

#include <kis_config_widget.h>

class Ui_WdgUnsharp;

class KisWdgUnsharp : public KisConfigWidget
{
    Q_OBJECT
public:
    KisWdgUnsharp(QWidget * parent);
    ~KisWdgUnsharp() override;
    inline const Ui_WdgUnsharp* widget() const {
        return m_widget;
    }
    void setConfiguration(const KisPropertiesConfigurationSP) override;
    KisPropertiesConfigurationSP configuration() const override;
private:
    Ui_WdgUnsharp* m_widget;
};

#endif
