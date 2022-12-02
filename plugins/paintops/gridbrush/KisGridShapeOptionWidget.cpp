/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý (lukast.dev@gmail.com)
 * 
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisGridShapeOptionWidget.h"

#include <QButtonGroup>
#include <QMetaProperty>

#include <lager/constant.hpp>
#include "ui_wdggridbrushshapeoptions.h"

#include "KisGridShapeOptionModel.h"

namespace {


class KisShapeOptionsWidgetUI: public QWidget, public Ui::WdgGridBrushShapeOptions
{
public:
    KisShapeOptionsWidgetUI(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);
    }
};
}

struct KisGridShapeOptionWidget::Private
{
    Private(lager::cursor<KisGridShapeOptionData> optionData)
        : model(optionData)
    {
    }

    KisGridShapeOptionModel model;
    KisShapeOptionsWidgetUI* options {0};
};


KisGridShapeOptionWidget::KisGridShapeOptionWidget(lager::cursor<KisGridShapeOptionData> optionData)
    : KisPaintOpOption(i18n("Particle type"), KisPaintOpOption::GENERAL, true)
    , m_d(new Private(optionData))
{
	
	m_d->options = new KisShapeOptionsWidgetUI();
	setObjectName("KisGridShapeOption");

    m_checkable = false;
	
    using namespace KisWidgetConnectionUtils;
    
    
    connectControl(m_d->options->shapeCBox, &m_d->model, "shape");
    
    
    m_d->model.optionData.bind(std::bind(&KisGridShapeOptionWidget::emitSettingChanged, this));
    
    setConfigurationPage(m_d->options);
}

KisGridShapeOptionWidget::~KisGridShapeOptionWidget()
{
}

void KisGridShapeOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisGridShapeOptionData data = *m_d->model.optionData;
    data.write(setting.data());
}

void KisGridShapeOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisGridShapeOptionData data = *m_d->model.optionData;
    data.read(setting.data());
    m_d->model.optionData.set(data);
}
