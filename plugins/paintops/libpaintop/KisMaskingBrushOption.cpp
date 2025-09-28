/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisMaskingBrushOption.h"

#include "kis_predefined_brush_chooser.h"
#include "kis_brush_selection_widget.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>

#include <QDomDocument>
#include "kis_brush.h"
#include "kis_image.h"
#include "kis_image_config.h"
#include "kis_brush_option.h"

#include "KisMaskingBrushOptionProperties.h"
#include <strokes/KisMaskingBrushCompositeOpFactory.h>
#include <KoCompositeOpRegistry.h>
#include <brushengine/KisPaintopSettingsIds.h>
#include <brushengine/kis_paintop_lod_limitations.h>
#include <lager/state.hpp>
#include <lager/constant.hpp>
#include <KisWidgetConnectionUtils.h>
#include <functional>
#include "KisAutoBrushModel.h"
#include "KisPredefinedBrushModel.h"
#include "KisTextBrushModel.h"


using namespace KisBrushModel;
using namespace KisWidgetConnectionUtils;


namespace detail {

QString warningLabelText(qreal realBrushSize, qreal theoreticalMaskingBrushSize)
{
    return
        i18nc("warning about too big size of the masked brush",
              "WARNING: Dependent size of the masked brush grew too big (%1 pixels). Its value has been cropped to %2 pixels.",
              theoreticalMaskingBrushSize,
              realBrushSize);
}

bool warningLabelVisible(qreal theoreticalBrushSize) {
    KisImageConfig cfg(true);
    return theoreticalBrushSize > cfg.maxMaskingBrushSize();
}


}

class MaskingBrushModel : public QObject
{
    Q_OBJECT
public:
    MaskingBrushModel(lager::cursor<MaskingBrushData> maskingData, lager::cursor<qreal> commonBrushSizeData, lager::reader<qreal> masterBrushSize)
        : m_maskingData(maskingData),
          m_commonBrushSizeData(commonBrushSizeData),
          m_masterBrushSize(masterBrushSize),
          m_preserveMode(false),
          autoBrushModel(m_maskingData[&MaskingBrushData::brush][&BrushData::common],
                         m_maskingData[&MaskingBrushData::brush][&BrushData::autoBrush],
                         m_commonBrushSizeData),
          predefinedBrushModel(m_maskingData[&MaskingBrushData::brush][&BrushData::common],
                               m_maskingData[&MaskingBrushData::brush][&BrushData::predefinedBrush],
                               m_commonBrushSizeData,
                               false),
          textBrushModel(m_maskingData[&MaskingBrushData::brush][&BrushData::common],
                         m_maskingData[&MaskingBrushData::brush][&BrushData::textBrush]),
          LAGER_QT(isEnabled) {m_maskingData[&MaskingBrushData::isEnabled]},
          LAGER_QT(compositeOpId) {m_maskingData[&MaskingBrushData::compositeOpId]},
          LAGER_QT(theoreticalBrushSize) {
              lager::with(m_maskingData[&MaskingBrushData::masterSizeCoeff],
                          m_masterBrushSize)
                      .map(std::multiplies<qreal>{})},
          LAGER_QT(realBrushSize) {m_commonBrushSizeData},
          LAGER_QT(warningLabelVisible) {
              lager::with(m_preserveMode,
                          LAGER_QT(theoreticalBrushSize).map(&detail::warningLabelVisible))
                      .map(std::logical_and<bool>{})},
          LAGER_QT(warningLabelText) {
              lager::with(LAGER_QT(realBrushSize),
                          LAGER_QT(theoreticalBrushSize))
                      .map(&detail::warningLabelText)},
          m_maskingBrushCursor(m_maskingData[&MaskingBrushData::brush])
    {
        lager::watch(m_commonBrushSizeData, std::bind(&MaskingBrushModel::updatePreserveMode, this));
        lager::watch(m_maskingBrushCursor, std::bind(&MaskingBrushModel::updatePreserveMode, this));
        lager::watch(m_masterBrushSize, std::bind(&MaskingBrushModel::updatePreserveMode, this));
    }

    // the state must be declared **before** any cursors or readers
    lager::cursor<MaskingBrushData> m_maskingData;
    lager::cursor<qreal> m_commonBrushSizeData;
    lager::reader<qreal> m_masterBrushSize;
    lager::state<bool, lager::automatic_tag> m_preserveMode;

    KisAutoBrushModel autoBrushModel;
    KisPredefinedBrushModel predefinedBrushModel;
    KisTextBrushModel textBrushModel;

    LAGER_QT_CURSOR(bool, isEnabled);
    LAGER_QT_CURSOR(QString, compositeOpId);
    LAGER_QT_READER(qreal, theoreticalBrushSize);
    LAGER_QT_READER(qreal, realBrushSize);
    LAGER_QT_READER(bool, warningLabelVisible);
    LAGER_QT_READER(QString, warningLabelText);

    void updatePreserveMode()
    {
        if (!m_preserveMode.get() || !m_originalBrushInfo) return;

        if (m_originalBrushInfo->brush != m_maskingData->brush ||
            !qFuzzyCompare(m_originalBrushInfo->masterSize, m_masterBrushSize.get()) ||
            !qFuzzyCompare(m_originalBrushInfo->commonSize, m_commonBrushSizeData.get())) {

            m_originalBrushInfo = std::nullopt;
            m_preserveMode.set(false);
        }
    }

    void startPreserveMode()
    {
        m_originalBrushInfo =
            {m_maskingData->brush,
             m_masterBrushSize.get(),
             m_commonBrushSizeData.get()};

        m_preserveMode.set(true);
    }

    MaskingBrushData bakedOptionData() const {
        MaskingBrushData data = m_maskingData.get();

        data.brush.autoBrush = autoBrushModel.bakedOptionData();
        data.brush.predefinedBrush = predefinedBrushModel.bakedOptionData();
        return data;
    }

private:
    struct OriginalBrushInfo {
        BrushData brush;
        qreal masterSize;
        qreal commonSize;
    };

    std::optional<OriginalBrushInfo> m_originalBrushInfo;
    lager::reader<BrushData> m_maskingBrushCursor;

};


struct KisMaskingBrushOption::Private
{
    Private(lager::reader<qreal> effectiveBrushSize)
        : ui(new QWidget())
        ,  commonBrushSizeData(777.0)
        ,  masterBrushSize(effectiveBrushSize)

        ,  maskingModel(maskingData, commonBrushSizeData, effectiveBrushSize)
    {
        compositeSelector = new QComboBox(ui.data());

        const QStringList supportedComposites = KisMaskingBrushCompositeOpFactory::supportedCompositeOpIds();
        Q_FOREACH (const QString &id, supportedComposites) {
            const QString name = KoCompositeOpRegistry::instance().getKoID(id).name();
            compositeSelector->addItem(name, id);
        }
        compositeSelector->setCurrentIndex(0);

        QHBoxLayout *compositeOpLayout = new QHBoxLayout();
        compositeOpLayout->addWidget(new QLabel(i18n("Blending Mode:")), 0);
        compositeOpLayout->addWidget(compositeSelector, 1);

        brushSizeWarningLabel = new QLabel(ui.data());
        brushSizeWarningLabel->setVisible(false);
        brushSizeWarningLabel->setWordWrap(true);

        brushChooser = new KisBrushSelectionWidget(KisImageConfig(true).maxMaskingBrushSize(), &maskingModel.autoBrushModel, &maskingModel.predefinedBrushModel, &maskingModel.textBrushModel, maskingData[&MaskingBrushData::brush][&BrushData::type], brushPrecisionData, KisBrushOptionWidgetFlag::None, ui.data());

        QVBoxLayout *layout  = new QVBoxLayout(ui.data());
        layout->addLayout(compositeOpLayout, 0);
        layout->addWidget(brushSizeWarningLabel, 0);
        layout->addWidget(brushChooser, 1);
    }

    QScopedPointer<QWidget> ui;
    KisBrushSelectionWidget *brushChooser = 0;
    QComboBox *compositeSelector = 0;
    QLabel *brushSizeWarningLabel = 0;

    lager::state<KisBrushModel::MaskingBrushData, lager::automatic_tag> maskingData;
    lager::state<qreal, lager::automatic_tag> commonBrushSizeData;
    lager::reader<qreal> masterBrushSize;
    MaskingBrushModel maskingModel;

    /// we don't use precision data, we just need it to pass
    /// to the brush selection widget
    lager::state<KisBrushModel::PrecisionData, lager::automatic_tag> brushPrecisionData;
};

KisMaskingBrushOption::KisMaskingBrushOption(lager::reader<qreal> effectiveBrushSize)
    : KisPaintOpOption(i18n("Brush Tip"), KisPaintOpOption::MASKING_BRUSH, true)
    , m_d(new Private(effectiveBrushSize))
{
    setObjectName("KisMaskingBrushOption");
    setConfigurationPage(m_d->ui.data());

    connect(&m_d->maskingModel, &MaskingBrushModel::isEnabledChanged,
            this, &KisMaskingBrushOption::setChecked);
    connect(this, &KisMaskingBrushOption::sigCheckedChanged,
            &m_d->maskingModel, &MaskingBrushModel::setisEnabled);
    m_d->maskingModel.LAGER_QT(isEnabled).nudge();

    connect(&m_d->maskingModel, &MaskingBrushModel::compositeOpIdChanged,
            this, &KisMaskingBrushOption::slotCompositeModePropertyChanged);
    connect(m_d->compositeSelector, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &KisMaskingBrushOption::slotCompositeModeWidgetChanged);
    m_d->maskingModel.LAGER_QT(compositeOpId).nudge();

    connect(&m_d->maskingModel, &MaskingBrushModel::warningLabelVisibleChanged,
            m_d->brushSizeWarningLabel, &QLabel::setVisible);
    m_d->maskingModel.LAGER_QT(warningLabelVisible).nudge();

    connect(&m_d->maskingModel, &MaskingBrushModel::warningLabelTextChanged,
            m_d->brushSizeWarningLabel, &QLabel::setText);
    m_d->maskingModel.LAGER_QT(warningLabelText).nudge();

    m_d->maskingData.watch(std::bind(&KisMaskingBrushOption::emitSettingChanged, this));
    m_d->commonBrushSizeData.watch(std::bind(&KisMaskingBrushOption::emitSettingChanged, this));
}

KisMaskingBrushOption::~KisMaskingBrushOption()
{

}

void KisMaskingBrushOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    using namespace KisBrushModel;

    if (m_d->maskingData->useMasterSize &&
        !m_d->maskingModel.m_preserveMode.get()) {

        MaskingBrushData tempData = m_d->maskingModel.bakedOptionData();
        tempData.masterSizeCoeff = m_d->commonBrushSizeData.get() / m_d->masterBrushSize.get();
        tempData.write(setting.data());
    } else {
        m_d->maskingModel.bakedOptionData().write(setting.data());
    }
}

void KisMaskingBrushOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    MaskingBrushData data = MaskingBrushData::read(setting.data(), m_d->masterBrushSize.get(), resourcesInterface());

    m_d->commonBrushSizeData.set(effectiveSizeForBrush(data.brush.type,
                                                       data.brush.autoBrush,
                                                       data.brush.predefinedBrush,
                                                       data.brush.textBrush));
    m_d->maskingData.set(data);
    m_d->maskingModel.startPreserveMode();
}

void KisMaskingBrushOption::setImage(KisImageWSP image)
{
    m_d->brushChooser->setImage(image);
}

void KisMaskingBrushOption::lodLimitations(KisPaintopLodLimitations *l) const
{
    *l |= KisBrushModel::brushLodLimitations(m_d->maskingData->brush);
}

lager::reader<bool> KisMaskingBrushOption::maskingBrushEnabledReader() const
{
    return m_d->maskingData[&MaskingBrushData::isEnabled];
}

void KisMaskingBrushOption::slotCompositeModeWidgetChanged(int index)
{
    m_d->maskingModel.setcompositeOpId(m_d->compositeSelector->itemData(index).toString());
}

void KisMaskingBrushOption::slotCompositeModePropertyChanged(const QString &value)
{
    const int index = m_d->compositeSelector->findData(QVariant::fromValue(value));
    KIS_SAFE_ASSERT_RECOVER_RETURN(index >= 0);
    m_d->compositeSelector->setCurrentIndex(index);
}

#include "KisMaskingBrushOption.moc"
