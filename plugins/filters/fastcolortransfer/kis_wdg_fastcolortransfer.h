/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_WDG_FASTCOLORTRANSFER_H
#define KIS_WDG_FASTCOLORTRANSFER_H

#include <kis_config_widget.h>

class Ui_WdgFastColorTransfer;

/**
 @author Cyrille Berger <cberger@cberger.net>
*/
class KisWdgFastColorTransfer : public KisConfigWidget
{
public:
    KisWdgFastColorTransfer(QWidget * parent);
    ~KisWdgFastColorTransfer() override;
    void setConfiguration(const KisPropertiesConfigurationSP) override;
    inline const Ui_WdgFastColorTransfer* widget() const {
        return m_widget;
    }
    KisPropertiesConfigurationSP configuration() const override;
private:
    Ui_WdgFastColorTransfer* m_widget;
};

#endif
