/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2008, 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 * 
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSprayShapeOptionWidget.h"

#include <QButtonGroup>
#include <QMetaProperty>
#include <QMetaMethod>

#include <KisImportExportManager.h>
#include "kis_aspect_ratio_locker.h"

#include <lager/constant.hpp>
#include "ui_wdgsprayshapeoptions.h"
#include "kis_signals_blocker.h"

#include "KisSprayShapeOptionModel.h"


namespace {
class KisShapeOptionsWidget: public QWidget, public Ui::WdgSprayShapeOptions
{
public:
    KisShapeOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);
        imageUrlRequester->setMimeTypeFilters(KisImportExportManager::supportedMimeTypes(KisImportExportManager::Import));
        
        //initialize slider values
		widthSpin->setRange(1, 1000, 0);
		widthSpin->setValue(6);
		widthSpin->setSuffix(i18n(" px"));

		heightSpin->setRange(1, 1000, 0);
		heightSpin->setValue(6);
		heightSpin->setSuffix(i18n(" px"));
    }
};
}

struct KisSprayShapeOptionWidget::Private
{
    Private(lager::cursor<KisSprayShapeOptionData> optionData, lager::cursor<int> diameter, lager::cursor<qreal> scale)
        : model(optionData, diameter, scale)
    {
    }

    KisSprayShapeOptionModel model;
    KisAspectRatioLocker *sizeRatioLocker;
    KisShapeOptionsWidget *widget;
};

KisSprayShapeOptionWidget::KisSprayShapeOptionWidget(lager::cursor<KisSprayShapeOptionData> optionData, lager::cursor<int> diameter, lager::cursor<qreal> scale)
    : KisPaintOpOption(i18n("Spray Shape"), KisPaintOpOption::GENERAL, optionData[&KisSprayShapeOptionData::enabled])
    , m_d(new Private(optionData, diameter, scale))
{
	
	KisShapeOptionsWidget *widget = new KisShapeOptionsWidget();
	m_d->widget = widget;
	
	setObjectName("KisSprayShapeOptionWidget");
	
	m_checkable = true;
	
    using namespace KisWidgetConnectionUtils;
    
    m_d->sizeRatioLocker = new KisAspectRatioLocker(this);
    m_d->sizeRatioLocker->connectSpinBoxes(widget->widthSpin, widget->heightSpin, widget->aspectButton);
    
    connect(m_d->sizeRatioLocker, &KisAspectRatioLocker::sliderValueChanged, widget,
            [this, widget] () {
                m_d->model.seteffectiveSize(QSize(widget->widthSpin->value(), widget->heightSpin->value()));
            });
	
    m_d->model.LAGER_QT(effectiveSize).bind([this, widget] (QSize value) {
        KisSignalsBlocker b(widget->widthSpin);
        KisSignalsBlocker b2(widget->heightSpin);
        
        widget->widthSpin->setValue(value.width());
        widget->heightSpin->setValue(value.height());
        
        m_d->sizeRatioLocker->updateAspect();
    });
    
    connectControl(widget->shapeBox, &m_d->model, "shape");
    
    connectControl(widget->proportionalBox, &m_d->model, "effectiveProportional");

    connectControl(widget->imageUrlRequester, &m_d->model, "imageUrl");
    
    m_d->widget->widthSpin->setSuffix(m_d->model.sizeSuffix());
    m_d->widget->heightSpin->setSuffix(m_d->model.sizeSuffix());
    
    connect(&m_d->model, &KisSprayShapeOptionModel::sizeSuffixChanged, m_d->widget->widthSpin, &KisDoubleSliderSpinBox::setSuffix);
    connect(&m_d->model, &KisSprayShapeOptionModel::sizeSuffixChanged, m_d->widget->heightSpin, &KisDoubleSliderSpinBox::setSuffix);
    
    m_d->model.optionData.bind(std::bind(&KisSprayShapeOptionWidget::emitSettingChanged, this));
    
    setConfigurationPage(widget);
}

KisSprayShapeOptionWidget::~KisSprayShapeOptionWidget()
{
}

void KisSprayShapeOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisSprayShapeOptionData data = *m_d->model.optionData;
    data.write(setting.data());
}

void KisSprayShapeOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisSprayShapeOptionData data = *m_d->model.optionData;
    data.read(setting.data());
    m_d->model.optionData.set(data);
}
