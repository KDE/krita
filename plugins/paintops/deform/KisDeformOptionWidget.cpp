/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisDeformOptionWidget.h"

#include <QButtonGroup>

#include <lager/constant.hpp>
#include "ui_wdgdeformoptions.h"

#include "KisDeformOptionModel.h"

namespace {


class KisDeformOptionsWidget: public QWidget, public Ui::WdgDeformOptions
{
public:
    KisDeformOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);

        deformAmount->setRange(0.0, 1.0, 2);
        deformAmount->setSingleStep(0.01);
    }
};


}


struct KisDeformOptionWidget::Private
{
    Private(lager::cursor<KisDeformOptionData> optionData)
        : model(optionData)
    {
    }

    KisDeformOptionModel model;
};


KisDeformOptionWidget::KisDeformOptionWidget(lager::cursor<KisDeformOptionData> optionData)
    : KisPaintOpOption(i18n("Deform Options"), KisPaintOpOption::COLOR, true)
    , m_d(new Private(optionData))
{
	KisDeformOptionsWidget *widget = new KisDeformOptionsWidget();
	setObjectName("KisDeformOption");

	m_checkable = false;

    using namespace KisWidgetConnectionUtils;

    connectControl(widget->deformAmount, &m_d->model, "deformAmount");
    connectControl(widget->interpolationChBox, &m_d->model, "deformUseBilinear");
    connectControl(widget->useCounter, &m_d->model, "deformUseCounter");
    connectControl(widget->useOldData, &m_d->model, "deformUseOldData");

    QButtonGroup *group = new QButtonGroup(widget);
    group->addButton(widget->growBtn, static_cast<int>(DeformModes::GROW));
    group->addButton(widget->shrinkBtn, static_cast<int>(DeformModes::SHRINK));
    group->addButton(widget->swirlCWBtn, static_cast<int>(DeformModes::SWIRL_CW));
    group->addButton(widget->swirlCCWBtn, static_cast<int>(DeformModes::SWIRL_CCW));
    group->addButton(widget->moveBtn, static_cast<int>(DeformModes::MOVE));
    group->addButton(widget->lensBtn, static_cast<int>(DeformModes::LENS_IN));
    group->addButton(widget->lensOutBtn, static_cast<int>(DeformModes::LENS_OUT));
    group->addButton(widget->colorBtn, static_cast<int>(DeformModes::DEFORM_COLOR));
    group->setExclusive(true);
    connectControl(group, &m_d->model, "deformAction");

    m_d->model.optionData.bind(std::bind(&KisDeformOptionWidget::emitSettingChanged, this));

    setConfigurationPage(widget);
}

KisDeformOptionWidget::~KisDeformOptionWidget()
{
}

void KisDeformOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisDeformOptionData data = *m_d->model.optionData;
    data.write(setting.data());
}

void KisDeformOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisDeformOptionData data = *m_d->model.optionData;
    data.read(setting.data());
    m_d->model.optionData.set(data);
}
