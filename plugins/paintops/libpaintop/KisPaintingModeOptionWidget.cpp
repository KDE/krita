/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisPaintingModeOptionWidget.h"

#include <QButtonGroup>

#include <lager/constant.hpp>
#include "ui_wdgincremental.h"

#include "KisPaintingModeOptionModel.h"

namespace {

class KisPaintingModeWidget: public QWidget, public Ui::WdgIncremental
{
public:
    KisPaintingModeWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);
    }
};

QString calcButtonGroupToolTip(bool maskingBrushEnabled) {
    return maskingBrushEnabled ?
        i18nc("@info:tooltip",
              "Only wash mode is possible when using a masked brush.") :
        "";
}

}


struct KisPaintingModeOptionWidget::Private
{
    Private(lager::cursor<KisPaintingModeOptionData> optionData, lager::reader<bool> maskingBrushEnabled)
        : model(optionData, maskingBrushEnabled)
        , buttonGroupToolTip {maskingBrushEnabled.map(&calcButtonGroupToolTip)}
    {
    }

    KisPaintingModeOptionModel model;
    lager::reader<QString> buttonGroupToolTip;
};


KisPaintingModeOptionWidget::KisPaintingModeOptionWidget(lager::cursor<KisPaintingModeOptionData> optionData)
    : KisPaintingModeOptionWidget(optionData, lager::make_constant(false))
{
}

KisPaintingModeOptionWidget::KisPaintingModeOptionWidget(lager::cursor<KisPaintingModeOptionData> optionData, lager::reader<bool> maskingBrushEnabled)
    : KisPaintOpOption(i18n("Painting Mode"), KisPaintOpOption::COLOR, true)
    , m_d(new Private(optionData, maskingBrushEnabled))
{
    using namespace KisWidgetConnectionUtils;

    setObjectName("KisPaintActionTypeOption");
    m_checkable = false;

    KisPaintingModeWidget *widget = new KisPaintingModeWidget();

    QButtonGroup *group = new QButtonGroup(widget);
    group->addButton(widget->radioBuildup, static_cast<int>(enumPaintingMode::BUILDUP));
    group->addButton(widget->radioWash, static_cast<int>(enumPaintingMode::WASH));
    group->setExclusive(true);

    setConfigurationPage(widget);

    connectControlState(group, &m_d->model, "paintingModeState", "paintingMode");
    m_d->buttonGroupToolTip.bind(std::bind(&QWidget::setToolTip, widget->radioBuildup, std::placeholders::_1));
    m_d->buttonGroupToolTip.bind(std::bind(&QWidget::setToolTip, widget->radioWash, std::placeholders::_1));
    m_d->buttonGroupToolTip.bind(std::bind(&QWidget::setToolTip, widget->label, std::placeholders::_1));

    m_d->model.optionData.bind(std::bind(&KisPaintingModeOptionWidget::emitSettingChanged, this));
}

KisPaintingModeOptionWidget::~KisPaintingModeOptionWidget()
{
}

void KisPaintingModeOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    m_d->model.bakedOptionData().write(setting.data());
}

void KisPaintingModeOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisPaintingModeOptionData data = *m_d->model.optionData;
    data.read(setting.data());
    m_d->model.optionData.set(data);
}
