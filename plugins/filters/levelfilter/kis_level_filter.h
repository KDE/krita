/*
 * This file is part of Krita
 *
 * Copyright (c) 2006 Frederic Coiffier <fcoiffie@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev) const override;

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
