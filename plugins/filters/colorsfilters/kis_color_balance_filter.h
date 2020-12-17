/*
 *  SPDX-FileCopyrightText: 2013 Sahil Nagpal <nagpal.sahil01@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KIS_COLOR_BALANCE_FILTER_H_
#define _KIS_COLOR_BALANCE_FILTER_H_

#include <QList>
#include "filter/kis_filter.h"
#include "kis_config_widget.h"
#include "ui_wdg_color_balance.h"
#include "filter/kis_color_transformation_filter.h"


class QWidget;
class KoColorTransformation;

class KisColorBalanceFilter : public  KisColorTransformationFilter
{

public:

	KisColorBalanceFilter();
public:
    enum Type {
      SHADOWS,
      MIDTONES,
      HIGHLIGHTS
    };

public:

	KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;

    KoColorTransformation* createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const override;

	static inline KoID id() {
        return KoID("colorbalance", i18n("Color Balance"));
	}

    KisFilterConfigurationSP  defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;

};

class KisColorBalanceConfigWidget : public KisConfigWidget
{

	Q_OBJECT

public:
    KisColorBalanceConfigWidget(QWidget * parent);
	~KisColorBalanceConfigWidget() override;

	KisPropertiesConfigurationSP  configuration() const override;
	void setConfiguration(const KisPropertiesConfigurationSP config) override;
    Ui_Form * m_page;
    QString m_id;

public Q_SLOTS:
    void slotShadowsClear();
    void slotMidtonesClear();
    void slotHighlightsClear();
};

#endif
