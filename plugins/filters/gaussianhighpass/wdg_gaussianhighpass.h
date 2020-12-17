/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2019 Miguel Lopez <reptillia39@live.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_WDG_GAUSSIANHIGHPASS_H_
#define _KIS_WDG_GAUSSIANHIGHPASS_H_

#include <kis_config_widget.h>

class Ui_WdgGaussianHighPass;

class KisWdgGaussianHighPass : public KisConfigWidget
{
    Q_OBJECT
public:
    KisWdgGaussianHighPass(QWidget * parent);
    ~KisWdgGaussianHighPass() override;
    inline const Ui_WdgGaussianHighPass* widget() const {
        return m_widget;
    }
    void setConfiguration(const KisPropertiesConfigurationSP) override;
    KisPropertiesConfigurationSP configuration() const override;
private:
    Ui_WdgGaussianHighPass* m_widget;
};

#endif
