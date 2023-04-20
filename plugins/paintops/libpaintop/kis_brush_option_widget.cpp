/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "kis_brush_option_widget.h"
#include <klocalizedstring.h>

#include <kis_image.h>
#include <kis_image_config.h>

#include "kis_brush_selection_widget.h"
#include "kis_brush.h"

#include <lager/state.hpp>
#include "KisBrushModel.h"
#include "kis_precision_option.h"
#include "kis_paintop_lod_limitations.h"

#include "KisAutoBrushModel.h"
#include "KisPredefinedBrushModel.h"
#include "KisTextBrushModel.h"

struct KisBrushOptionWidget::Private
{
    Private(KisBrushOptionWidgetFlags flags)
        : commonBrushSizeData(778.0)
        , autoBrushModel(brushData[&BrushData::common],
                         brushData[&BrushData::autoBrush],
                         commonBrushSizeData)
        , predefinedBrushModel(brushData[&BrushData::common],
                               brushData[&BrushData::predefinedBrush],
                               commonBrushSizeData,
                               flags & KisBrushOptionWidgetFlag::SupportsHSLBrushMode)
        , textBrushModel(brushData[&BrushData::common],
                         brushData[&BrushData::textBrush])
    {
    }

    lager::state<BrushData, lager::automatic_tag> brushData;
    lager::state<PrecisionData, lager::automatic_tag> brushPrecisionData;
    lager::state<qreal, lager::automatic_tag> commonBrushSizeData;

    KisAutoBrushModel autoBrushModel;
    KisPredefinedBrushModel predefinedBrushModel;
    KisTextBrushModel textBrushModel;

    KisBrushOptionWidgetFlags flags;
};

KisBrushOptionWidget::KisBrushOptionWidget(KisBrushOptionWidgetFlags flags)
    : KisPaintOpOption(i18n("Brush Tip"), KisPaintOpOption::GENERAL, true),
      m_d(new Private(flags))
{
    m_d->flags = flags;

    m_checkable = false;

    m_brushSelectionWidget = new KisBrushSelectionWidget(KisImageConfig(true).maxBrushSize(),
                                                         &m_d->autoBrushModel,
                                                         &m_d->predefinedBrushModel,
                                                         &m_d->textBrushModel,
                                                         m_d->brushData[&BrushData::type],
                                                         m_d->brushPrecisionData,
                                                         flags);
    m_brushSelectionWidget->hide();
    setConfigurationPage(m_brushSelectionWidget);

    setObjectName("KisBrushOptionWidget");

    // TODO: merge them into a single struct to avoid double updates
    lager::watch(m_d->brushData, std::bind(&KisBrushOptionWidget::emitSettingChanged, this));
    lager::watch(m_d->brushPrecisionData, std::bind(&KisBrushOptionWidget::emitSettingChanged, this));
    lager::watch(m_d->commonBrushSizeData, std::bind(&KisBrushOptionWidget::emitSettingChanged, this));
}

KisBrushOptionWidget::~KisBrushOptionWidget() = default;

KisBrushSP KisBrushOptionWidget::brush() const
{
    return m_brushSelectionWidget->brush();
}


void KisBrushOptionWidget::setImage(KisImageWSP image)
{
    m_brushSelectionWidget->setImage(image);
}

void KisBrushOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP settings) const
{
    using namespace KisBrushModel;

    BrushData data = m_d->brushData.get();

    data.autoBrush = m_d->autoBrushModel.bakedOptionData();
    data.predefinedBrush = m_d->predefinedBrushModel.bakedOptionData();

    data.write(settings.data());

    if (m_d->flags & KisBrushOptionWidgetFlag::SupportsPrecision) {
        m_d->brushPrecisionData->write(settings.data());
    }
}

void KisBrushOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    using namespace KisBrushModel;

    std::optional<BrushData> data = BrushData::read(setting.data(), resourcesInterface());
    KIS_SAFE_ASSERT_RECOVER_RETURN(data);
    m_d->brushData.set(*data);
    m_d->commonBrushSizeData.set(effectiveSizeForBrush(data->type, data->autoBrush, data->predefinedBrush, data->textBrush));

    if (m_d->flags & KisBrushOptionWidgetFlag::SupportsPrecision) {
        m_d->brushPrecisionData.set(KisBrushModel::PrecisionData::read(setting.data()));
    }
}

void KisBrushOptionWidget::hideOptions(const QStringList &options)
{
    m_brushSelectionWidget->hideOptions(options);
}

lager::reader<bool> KisBrushOptionWidget::lightnessModeEnabled() const
{
    return m_brushSelectionWidget->lightnessModeEnabled();
}

lager::reader<qreal> KisBrushOptionWidget::effectiveBrushSize() const
{
    return m_d->commonBrushSizeData;
}

lager::reader<BrushData> KisBrushOptionWidget::bakedBrushData() const
{
    return lager::with(m_d->brushData, m_d->commonBrushSizeData)
            .map([this] (BrushData data, qreal commonBrushSize) {

        Q_UNUSED(commonBrushSize); // we keep it as a dep only for updates!

        data.autoBrush = m_d->autoBrushModel.bakedOptionData();
        data.predefinedBrush = m_d->predefinedBrushModel.bakedOptionData();

        return data;
    });
}

KisPaintOpOption::OptionalLodLimitationsReader KisBrushOptionWidget::lodLimitationsReader() const
{
    return m_d->brushData.map(&KisBrushModel::brushLodLimitations);
}

#include "moc_kis_brush_option_widget.cpp"
