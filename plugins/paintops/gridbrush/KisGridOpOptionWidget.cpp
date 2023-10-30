/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý (lukast.dev@gmail.com)
 * 
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisGridOpOptionWidget.h"

#include <QButtonGroup>
#include <QMetaProperty>

#include <lager/constant.hpp>
#include "ui_wdggridoptions.h"

#include <KisDoubleSpinBoxPluralHelper.h>
#include "KisGridOpOptionModel.h"

namespace {


class KisGridOpOptionsWidgetUI: public QWidget, public Ui::WdgGridOptions
{
public:
    KisGridOpOptionsWidgetUI(QWidget *parent = 0)
        : QWidget(parent) {
			
			
        setupUi(this);
		
		// initialize slider values
		diameterSPBox->setRange(1,999,0);
		diameterSPBox->setValue(25);
		diameterSPBox->setSuffix(i18n(" px"));
		diameterSPBox->setExponentRatio(3.0);


		gridWidthSPBox->setRange(1, 999, 0);
		gridWidthSPBox->setValue(25);
		gridWidthSPBox->setSuffix(i18n(" px"));
		gridWidthSPBox->setExponentRatio(3.0);


		gridHeightSPBox->setRange(1, 999, 0);
		gridHeightSPBox->setValue(25);
		gridHeightSPBox->setSuffix(i18n(" px"));
		gridHeightSPBox->setExponentRatio(3.0);


		horizontalOffsetSPBox->setRange(-50, 50, 2);
		horizontalOffsetSPBox->setValue(0);
		KisDoubleSpinBoxPluralHelper::install(horizontalOffsetSPBox, [](double value) {
		    return i18nc("{n} is the number value, % is the percent sign", "{n}%", value);
		});


		verticalOffsetSPBox->setRange(-50, 50, 2);
		verticalOffsetSPBox->setValue(0);
		KisDoubleSpinBoxPluralHelper::install(verticalOffsetSPBox, [](double value) {
		    return i18nc("{n} is the number value, % is the percent sign", "{n}%", value);
		});


		divisionLevelSPBox->setRange(0, 25, 0);
		divisionLevelSPBox->setValue(2);

		scaleDSPBox->setRange(0.1, 10.0, 2);
		scaleDSPBox->setSingleStep(0.01);
		scaleDSPBox->setValue(1.0);
		scaleDSPBox->setExponentRatio(3.0);

		vertBorderDSPBox->setRange(0, 100, 2);
		vertBorderDSPBox->setSingleStep(0.01);
		vertBorderDSPBox->setValue(0.0);


		horizBorderDSPBox->setRange(0, 100, 2);
		horizBorderDSPBox->setSingleStep(0.01);
		horizBorderDSPBox->setValue(0.0);
    }
};
}

struct KisGridOpOptionWidget::Private
{
    Private(lager::cursor<KisGridOpOptionData> optionData)
        : model(optionData)
    {
    }

    KisGridOpOptionModel model;
    KisGridOpOptionsWidgetUI* options {0};
};

KisGridOpOptionWidget::KisGridOpOptionWidget(lager::cursor<KisGridOpOptionData> optionData)
    : KisPaintOpOption(i18n("Brush size"), KisPaintOpOption::GENERAL, true)
    , m_d(new Private(optionData))
{
	
	m_d->options = new KisGridOpOptionsWidgetUI();
	setObjectName("KisGridOpOption");

    m_checkable = false;
	
    using namespace KisWidgetConnectionUtils;
    
    connectControl(m_d->options->diameterSPBox, &m_d->model, "diameter");
    connectControl(m_d->options->gridWidthSPBox, &m_d->model, "grid_width");
    connectControl(m_d->options->gridHeightSPBox, &m_d->model, "grid_height");
    
    connectControl(m_d->options->horizontalOffsetSPBox, &m_d->model, "horizontal_offset");
    connectControl(m_d->options->verticalOffsetSPBox, &m_d->model, "vertical_offset");
    connectControl(m_d->options->divisionLevelSPBox, &m_d->model, "grid_division_level");
    
    connectControl(m_d->options->divisionPressureCHBox, &m_d->model, "grid_pressure_division");
    connectControl(m_d->options->scaleDSPBox, &m_d->model, "grid_scale");
    connectControl(m_d->options->vertBorderDSPBox, &m_d->model, "grid_vertical_border");
    
    connectControl(m_d->options->horizBorderDSPBox, &m_d->model, "grid_horizontal_border");
    connectControl(m_d->options->jitterBorderCHBox, &m_d->model, "grid_random_border");
    
    m_d->model.optionData.bind(std::bind(&KisGridOpOptionWidget::emitSettingChanged, this));
    
    setConfigurationPage(m_d->options);
}

KisGridOpOptionWidget::~KisGridOpOptionWidget()
{
}

void KisGridOpOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisGridOpOptionData data = *m_d->model.optionData;
    data.write(setting.data());
}

void KisGridOpOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisGridOpOptionData data = *m_d->model.optionData;
    data.read(setting.data());
    m_d->model.optionData.set(data);
}
