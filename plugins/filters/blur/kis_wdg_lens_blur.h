/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2010 Edward Apap <schumifer@hotmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_WDG_LENS_BLUR_H_
#define _KIS_WDG_LENS_BLUR_H_

#include <kis_config_widget.h>

class Ui_WdgLensBlur;

class KisWdgLensBlur : public KisConfigWidget
{
    Q_OBJECT
public:
    KisWdgLensBlur(QWidget * parent);
    ~KisWdgLensBlur() override;
    inline const Ui_WdgLensBlur* widget() const {
        return m_widget;
    }
    void setConfiguration(const KisPropertiesConfigurationSP) override;
    KisPropertiesConfigurationSP configuration() const override;
private:
    Ui_WdgLensBlur* m_widget;
    QMap<QString, QString> m_shapeTranslations;
};

#endif
