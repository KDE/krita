/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2009 Edward Apap <schumifer@hotmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_WDG_GAUSSIAN_BLUR_H_
#define _KIS_WDG_GAUSSIAN_BLUR_H_

#include <kis_config_widget.h>

class Ui_WdgGaussianBlur;

class KisWdgGaussianBlur : public KisConfigWidget
{
    Q_OBJECT
public:
    KisWdgGaussianBlur(bool useForMasks, QWidget * parent);
    ~KisWdgGaussianBlur() override;
    inline const Ui_WdgGaussianBlur* widget() const {
        return m_widget;
    }
    void setConfiguration(const KisPropertiesConfigurationSP) override;
    KisPropertiesConfigurationSP configuration() const override;

private Q_SLOTS:
    void horizontalRadiusChanged(qreal);
    void verticalRadiusChanged(qreal);
    void aspectLockChanged(bool);

private:
    Ui_WdgGaussianBlur* m_widget;
};

#endif
