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
#include "filter/kis_filter_configuration.h"
#include "kis_config_widget.h"
#include "ui_wdg_level.h"

class WdgLevel;
class QWidget;
class KisColorAdjustment;
class KisHistogram;


/**
 * This class affect Intensity Y of the image
 */
class KisLevelFilter : public KisColorTransformationFilter
{

public:

    KisLevelFilter();
    ~KisLevelFilter();

public:

//     virtual KisFilterConfiguration* factoryConfiguration(const KisPaintDeviceSP) const;
    virtual KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, const KisImageWSP image = 0) const;

    virtual KoColorTransformation* createTransformation(const KoColorSpace* cs, const KisFilterConfiguration* config) const;

    static inline KoID id() {
        return KoID("levels", i18n("Levels"));
    };

    virtual bool workWith(KoColorSpace* cs) const;
};


class KisLevelConfigWidget : public KisConfigWidget
{
    Q_OBJECT
public:
    KisLevelConfigWidget(QWidget * parent, KisPaintDeviceSP dev);
    virtual ~KisLevelConfigWidget();

    virtual KisPropertiesConfiguration* configuration() const;
    void setConfiguration(const KisPropertiesConfiguration* config);
    Ui::WdgLevel m_page;

protected slots:
    void slotDrawHistogram(bool logarithmic = false);

    void slotModifyInBlackLimit(int);
    void slotModifyInWhiteLimit(int);
    void slotModifyOutBlackLimit(int);
    void slotModifyOutWhiteLimit(int);

protected:
    KisHistogram *histogram;
    bool m_histlog;
};

#endif
