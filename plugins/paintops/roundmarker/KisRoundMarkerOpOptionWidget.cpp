/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 * 
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisRoundMarkerOpOptionWidget.h"

#include "kis_image_config.h"

#include <lager/constant.hpp>
#include "ui_kis_roundmarker_option.h"

#include "KisRoundMarkerOpOptionModel.h"

namespace {


class KisRoundMarkerOpWidget: public QWidget, public Ui::WdgKisRoundMarkerOption
{
public:
    KisRoundMarkerOpWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);

        const int maxBrushSize = KisImageConfig(true).maxBrushSize();

        dblDiameter->setRange(0.01, maxBrushSize, 2);
        dblDiameter->setSuffix(i18n(" px"));
    }
};


}


struct KisRoundMarkerOpOptionWidget::Private
{
    Private(lager::cursor<KisRoundMarkerOpOptionData> optionData)
        : model(optionData)
    {
    }

    KisRoundMarkerOpOptionModel model;
};


KisRoundMarkerOpOptionWidget::KisRoundMarkerOpOptionWidget(lager::cursor<KisRoundMarkerOpOptionData> optionData)
    : KisPaintOpOption(i18n("Brush"), KisPaintOpOption::GENERAL, true)
    , m_d(new Private(optionData))
{
	
	KisRoundMarkerOpWidget *widget = new KisRoundMarkerOpWidget();
	setObjectName("KisRoundMarkerOption");
	
	m_checkable = false;
	
    using namespace KisWidgetConnectionUtils;

    connectControl(widget->dblDiameter, &m_d->model, "diameter");
    connectControl(widget->spacingWidget, &m_d->model, "aggregatedSpacing");
    
    m_d->model.optionData.bind(std::bind(&KisRoundMarkerOpOptionWidget::emitSettingChanged, this));
    
    setConfigurationPage(widget);
}

KisRoundMarkerOpOptionWidget::~KisRoundMarkerOpOptionWidget()
{
}

void KisRoundMarkerOpOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisRoundMarkerOpOptionData data = *m_d->model.optionData;
    data.write(setting.data());
}

void KisRoundMarkerOpOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisRoundMarkerOpOptionData data = *m_d->model.optionData;
    data.read(setting.data());
    m_d->model.optionData.set(data);
}
