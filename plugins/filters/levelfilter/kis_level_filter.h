/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Frederic Coiffier <fcoiffie@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_LEVEL_FILTER_H_
#define _KIS_LEVEL_FILTER_H_

#include "filter/kis_color_transformation_filter.h"
#include "kis_config_widget.h"
#include "ui_wdg_level.h"

class WdgLevel;
class QWidget;
class KisHistogram;


/**
 * This class affect Intensity Y of the image
 */
class KisLevelFilter : public KisColorTransformationFilter
{

public:

    KisLevelFilter();
    ~KisLevelFilter() override;

public:

//     virtual KisFilterConfigurationSP factoryConfiguration() const;
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;

    KoColorTransformation* createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const override;

    static inline KoID id() {
        return KoID("levels", i18n("Levels"));
    }

};


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
