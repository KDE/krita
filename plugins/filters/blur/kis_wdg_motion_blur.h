/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2010 Edward Apap <schumifer@hotmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_WDG_MOTION_BLUR_H_
#define _KIS_WDG_MOTION_BLUR_H_

#include <kis_config_widget.h>

class Ui_WdgMotionBlur;

class KisWdgMotionBlur : public KisConfigWidget
{
    Q_OBJECT
public:
    KisWdgMotionBlur(QWidget * parent);
    ~KisWdgMotionBlur() override;
    inline const Ui_WdgMotionBlur* widget() const {
        return m_widget;
    }
    void setConfiguration(const KisPropertiesConfigurationSP) override;
    KisPropertiesConfigurationSP configuration() const override;
    
private:
    Ui_WdgMotionBlur* m_widget;
};

#endif
