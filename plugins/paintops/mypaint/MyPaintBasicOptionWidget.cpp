/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "MyPaintBasicOptionWidget.h"

#include "KisWidgetConnectionUtils.h"

#include "MyPaintBasicOptionModel.h"
#include "ui_wdgmypaintoptions.h"


struct MyPaintBasicOptionWidget::Private
{
    Private(lager::cursor<MyPaintBasicOptionData> optionData,
            lager::cursor<qreal> radiusCursor,
            lager::cursor<qreal> hardnessCursor,
            lager::cursor<qreal> opacityCursor)
        : model(optionData, radiusCursor, hardnessCursor, opacityCursor)
    {
    }

    MyPaintBasicOptionModel model;
};

MyPaintBasicOptionWidget::MyPaintBasicOptionWidget(lager::cursor<MyPaintBasicOptionData> optionData,
                                                   lager::cursor<qreal> radiusCursor,
                                                   lager::cursor<qreal> hardnessCursor,
                                                   lager::cursor<qreal> opacityCursor)
    : KisPaintOpOption(i18nc("MyPaint option name", "Basic"), KisPaintOpOption::GENERAL, true),
    m_d(new Private(optionData, radiusCursor, hardnessCursor, opacityCursor))
{
    using namespace KisWidgetConnectionUtils;
    m_checkable = false;

    setObjectName("KisCompositeOpOption");

    QWidget* widget = new QWidget();

    Ui::WdgMyPaintOptions ui;
    ui.setupUi(widget);

    ui.radiusSPBox->setRange(0.01, 8.0, 2);
    ui.radiusSPBox->setSingleStep(0.01);

    ui.hardnessSPBox->setRange(0.02, 1.0, 2);
    ui.hardnessSPBox->setSingleStep(0.01);

    ui.opacitySPBox->setRange(0.0, 1.0, 2);
    ui.opacitySPBox->setSingleStep(0.01);

    connectControl(ui.eraserBox, &m_d->model, "eraserMode");
    connectControl(ui.radiusSPBox, &m_d->model, "radius");
    connectControl(ui.hardnessSPBox, &m_d->model, "hardness");
    connectControl(ui.opacitySPBox, &m_d->model, "opacity");

    setConfigurationPage(widget);
    m_d->model.optionData.bind(std::bind(&MyPaintBasicOptionWidget::emitSettingChanged, this));
}

MyPaintBasicOptionWidget::~MyPaintBasicOptionWidget()
{
}

void MyPaintBasicOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    m_d->model.optionData->write(setting.data());
}

void MyPaintBasicOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    MyPaintBasicOptionData data = *m_d->model.optionData;
    data.read(setting.data());
    m_d->model.optionData.set(data);
}
