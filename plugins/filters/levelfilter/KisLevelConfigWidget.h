/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Frederic Coiffier <fcoiffie@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_LEVEL_CONFIG_WIDGET_H_
#define _KIS_LEVEL_CONFIG_WIDGET_H_

#include "kis_config_widget.h"
#include "ui_wdg_level.h"

class WdgLevel;
class QWidget;
class KisHistogram;

class KisLevelConfigWidget : public KisConfigWidget
{
    Q_OBJECT
public:
    KisLevelConfigWidget(QWidget * parent, KisPaintDeviceSP dev);
    ~KisLevelConfigWidget() override;

    KisPropertiesConfigurationSP configuration() const override;
    void setConfiguration(const KisPropertiesConfigurationSP config) override;
    Ui::WdgLevel m_page;

protected Q_SLOTS:
    void slotDrawHistogram(bool isLogarithmic);

    void slotModifyInBlackLimit(int);
    void slotModifyInWhiteLimit(int);
    void slotModifyOutBlackLimit(int);
    void slotModifyOutWhiteLimit(int);

    void slotAutoLevel(void);
    void slotInvert(void);

    void resetOutSpinLimit();

protected:
    QScopedPointer<KisHistogram> m_histogram;
    bool m_isLogarithmic;
    bool m_inverted;
};

#endif
