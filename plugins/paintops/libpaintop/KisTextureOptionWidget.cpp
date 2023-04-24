/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisTextureOptionWidget.h"

#include <kis_paintop_lod_limitations.h>
#include <KisTextureOptionModel.h>
#include <KisWidgetConnectionUtils.h>

#include "kis_texture_chooser.h"


struct KisTextureOptionWidget::Private
{
    Private(lager::cursor<KisTextureOptionData> optionData, KisResourcesInterfaceSP resourcesInterface)
        : model(optionData, resourcesInterface)
    {
    }

    KisTextureOptionModel model;
};


KisTextureOptionWidget::KisTextureOptionWidget(lager::cursor<KisTextureOptionData> optionData, KisResourcesInterfaceSP resourcesInterface, KisBrushTextureFlags flags)
    : KisPaintOpOption(i18n("Pattern"), KisPaintOpOption::TEXTURE, optionData[&KisTextureOptionData::isEnabled])
    , m_d(new Private(optionData, resourcesInterface))
{
    using namespace KisWidgetConnectionUtils;
    setObjectName("KisTextureOption");

    KisTextureChooser *chooserWidget = new KisTextureChooser(flags);

    setConfigurationPage(chooserWidget);

    connect(&m_d->model, &KisTextureOptionModel::textureResourceChanged,
            chooserWidget->textureSelectorWidget, &KisPatternChooser::setCurrentPattern);
    connect(chooserWidget->textureSelectorWidget, &KisPatternChooser::resourceSelected,
            &m_d->model, &KisTextureOptionModel::settextureResource);
    chooserWidget->textureSelectorWidget->setCurrentPattern(m_d->model.textureResource());

    connect(&m_d->model, &KisTextureOptionModel::cutOffLeftNormalizedChanged,
            chooserWidget->cutoffSlider, &KisInputLevelsSlider::setBlackPoint);
    connect(chooserWidget->cutoffSlider, &KisInputLevelsSlider::blackPointChanged,
            &m_d->model, &KisTextureOptionModel::setcutOffLeftNormalized);
    chooserWidget->cutoffSlider->setBlackPoint(m_d->model.cutOffLeftNormalized());

    connect(&m_d->model, &KisTextureOptionModel::cutOffRightNormalizedChanged,
            chooserWidget->cutoffSlider, &KisInputLevelsSlider::setWhitePoint);
    connect(chooserWidget->cutoffSlider, &KisInputLevelsSlider::whitePointChanged,
            &m_d->model, &KisTextureOptionModel::setcutOffRightNormalized);
    chooserWidget->cutoffSlider->setWhitePoint(m_d->model.cutOffRightNormalized());

    connectControl(chooserWidget->scaleSlider, &m_d->model, "scale");
    connectControl(chooserWidget->brightnessSlider, &m_d->model, "brightness");
    connectControl(chooserWidget->contrastSlider, &m_d->model, "contrast");
    connectControl(chooserWidget->neutralPointSlider, &m_d->model, "neutralPoint");
    connectControl(chooserWidget->offsetSliderX, &m_d->model, "offsetX");
    connectControl(chooserWidget->randomOffsetX, &m_d->model, "isRandomOffsetX");
    connectControl(chooserWidget->randomOffsetY, &m_d->model, "isRandomOffsetY");
    connectControl(chooserWidget->offsetSliderY, &m_d->model, "offsetY");
    connectControl(chooserWidget->cmbTexturingMode, &m_d->model, "texturingMode");
    connectControl(chooserWidget->cmbCutoffPolicy, &m_d->model, "cutOffPolicy");
    connectControl(chooserWidget->chkInvert, &m_d->model, "invert");

    m_d->model.LAGER_QT(maximumOffsetX).bind(std::bind(&KisSliderSpinBox::setMaximum, chooserWidget->offsetSliderX, std::placeholders::_1, true));
    m_d->model.LAGER_QT(maximumOffsetY).bind(std::bind(&KisSliderSpinBox::setMaximum, chooserWidget->offsetSliderY, std::placeholders::_1, true));

    m_d->model.optionData.bind(std::bind(&KisTextureOptionWidget::emitSettingChanged, this));
}

KisTextureOptionWidget::~KisTextureOptionWidget()
{
}

void KisTextureOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    m_d->model.bakedOptionData().write(setting.data());
}

void KisTextureOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisTextureOptionData data = *m_d->model.optionData;
    data.read(setting.data());
    m_d->model.optionData.set(data);
}

KisPaintOpOption::OptionalLodLimitationsReader KisTextureOptionWidget::lodLimitationsReader() const
{
    return m_d->model.optionData.map(std::mem_fn(&KisTextureOptionData::lodLimitations));
}
