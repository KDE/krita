/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2008, 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 * 
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSprayShapeDynamicsOptionWidget.h"

#include <QButtonGroup>
#include <QMetaProperty>
#include <QMetaObject>



#include <lager/constant.hpp>
#include "ui_wdgshapedynamicsoptions.h"

#include "KisSprayShapeDynamicsOptionModel.h"

namespace {
class KisShapeDynamicsOptionWidget: public QWidget, public Ui::WdgShapeDynamicsOptions
{
public:
    KisShapeDynamicsOptionWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);
        
        drawingAngleWeight->setRange(0.0, 1.0, 2);
		drawingAngleWeight->setSingleStep(0.01);
		
		followCursorWeight->setRange(0.0, 1.0, 2);
		followCursorWeight->setSingleStep(0.01);
		
		randomAngleWeight->setRange(0.0, 1.0, 2);
		randomAngleWeight->setSingleStep(0.01);
		
		fixedAngleBox->setDecimals(0);
		fixedAngleBox->setIncreasingDirection(KisAngleGauge::IncreasingDirection_Clockwise);
    }
};
}

struct KisSprayShapeDynamicsOptionWidget::Private
{
    Private(lager::cursor<KisSprayShapeDynamicsOptionData> optionData)
        : model(optionData)
    {
    }

    KisSprayShapeDynamicsOptionModel model;
};

KisSprayShapeDynamicsOptionWidget::KisSprayShapeDynamicsOptionWidget(lager::cursor<KisSprayShapeDynamicsOptionData> optionData)
    : KisPaintOpOption(i18n("Shape dynamics"), KisPaintOpOption::GENERAL, optionData[&KisSprayShapeDynamicsOptionData::enabled])
    , m_d(new Private(optionData))
{
	
	KisShapeDynamicsOptionWidget *widget = new KisShapeDynamicsOptionWidget();
	setObjectName("KisSprayShapeDynamicsOptionWidget");
	
	m_checkable = true;
	//m_checked = true;
	
    using namespace KisWidgetConnectionUtils;
    
    connectControl(widget->randomSizeCHBox, &m_d->model, "randomSize");
    
    connectControl(widget->fixedRotation, &m_d->model, "fixedRotation");
    connectControl(widget->randomRotation, &m_d->model, "randomRotation");
    connectControl(widget->followCursor, &m_d->model, "followCursor");
    connectControl(widget->drawingAngle, &m_d->model, "followDrawingAngle");

    connectControl(widget->fixedAngleBox, &m_d->model, "fixedAngle");
    connectControl(widget->randomAngleWeight, &m_d->model, "randomRotationWeight");
    connectControl(widget->followCursorWeight, &m_d->model, "followCursorWeight");
    connectControl(widget->drawingAngleWeight, &m_d->model, "followDrawingAngleWeight");
    
    
    widget->fixedAngleBox->setEnabled(m_d->model.fixedRotation());
    connect(&m_d->model, &KisSprayShapeDynamicsOptionModel::fixedRotationChanged, widget->fixedAngleBox, &KisAngleSelector::setEnabled);
    
    widget->randomAngleWeight->setEnabled(m_d->model.randomRotation());
    connect(&m_d->model, &KisSprayShapeDynamicsOptionModel::randomRotationChanged, widget->randomAngleWeight, &KisDoubleSliderSpinBox::setEnabled);
    
    widget->followCursorWeight->setEnabled(m_d->model.followCursor());
    connect(&m_d->model, &KisSprayShapeDynamicsOptionModel::followCursorChanged, widget->followCursorWeight, &KisDoubleSliderSpinBox::setEnabled);
    
    widget->drawingAngleWeight->setEnabled(m_d->model.followDrawingAngle());
    connect(&m_d->model, &KisSprayShapeDynamicsOptionModel::followDrawingAngleChanged, widget->drawingAngleWeight, &KisDoubleSliderSpinBox::setEnabled);
    
    
    m_d->model.optionData.bind(std::bind(&KisSprayShapeDynamicsOptionWidget::emitSettingChanged, this));
    
    setConfigurationPage(widget);
}

KisSprayShapeDynamicsOptionWidget::~KisSprayShapeDynamicsOptionWidget()
{
}

void KisSprayShapeDynamicsOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisSprayShapeDynamicsOptionData data = *m_d->model.optionData;
    data.write(setting.data());
}

void KisSprayShapeDynamicsOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisSprayShapeDynamicsOptionData data = *m_d->model.optionData;
    data.read(setting.data());
    m_d->model.optionData.set(data);
}
