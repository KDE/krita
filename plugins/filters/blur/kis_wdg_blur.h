/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_WDG_BLUR_H_
#define _KIS_WDG_BLUR_H_

#include <kis_config_widget.h>

class Ui_WdgBlur;

class KisWdgBlur : public KisConfigWidget
{
    Q_OBJECT
public:
    KisWdgBlur(QWidget * parent);
    ~KisWdgBlur() override;
    inline const Ui_WdgBlur* widget() const {
        return m_widget;
    }
    void setConfiguration(const KisPropertiesConfigurationSP) override;
    KisPropertiesConfigurationSP configuration() const override;

private Q_SLOTS:

    void linkSpacingToggled(bool);
    void sldHalfWidthChanged(int);
    void sldHalfHeightChanged(int);

private:

    bool m_halfSizeLink;
    Ui_WdgBlur* m_widget;
};

#endif
