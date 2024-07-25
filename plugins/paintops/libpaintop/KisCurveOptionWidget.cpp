/*
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2009 Sven Langkamp <sven.langkamp@gmail.com>
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 * SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisCurveOptionWidget.h"

#include "ui_wdgcurveoption2.h"
#include "widgets/kis_curve_widget.h"
#include "kis_global.h"
#include "kis_icon_utils.h"

#include <kis_signals_blocker.h>
#include <KisCurveOptionModel.h>
#include <KisWidgetConnectionUtils.h>

#include <kis_algebra_2d.h>

#include <KisMultiSensorsSelector.h>
#include <KisDynamicSensorFactoryRegistry.h>
#include <KisCurveWidgetConnectionHelper.h>
#include <KisCurveRangeModel.h>
#include <KisCurveOptionInputControlsStrategy.h>

struct KisCurveOptionWidget::Private
{
    Private(lager::cursor<KisCurveOptionDataCommon> optionData,
            lager::reader<bool> enabledLink,
            std::optional<lager::reader<std::tuple<qreal, qreal>>> strengthRangeReader,
            qreal strengthDisplayMultiplier,
            KisCurveRangeModelFactory curveRangeFactory)
        : model(optionData, enabledLink, strengthRangeReader, strengthDisplayMultiplier, curveRangeFactory)
    {}

    KisCurveOptionModel model;
};

KisCurveOptionWidget::KisCurveOptionWidget(lager::cursor<KisCurveOptionDataCommon> optionData,
                                             PaintopCategory category,
                                             lager::reader<bool> enabledLink,
                                             std::optional<lager::reader<std::tuple<qreal, qreal>>> strengthRangeReader)
    : KisCurveOptionWidget(optionData,
                            category,
                            i18n("0%"), i18n("100%"),
                            enabledLink,
                            strengthRangeReader)
{
}

KisCurveOptionWidget::KisCurveOptionWidget(lager::cursor<KisCurveOptionDataCommon> optionData,
                                             PaintopCategory category,
                                             const QString &curveMinLabel, const QString &curveMaxLabel,
                                             lager::reader<bool> enabledLink,
                                             std::optional<lager::reader<std::tuple<qreal, qreal>>> strengthRangeReader)
    : KisCurveOptionWidget(optionData,
                            category,
                            curveMinLabel, curveMaxLabel,
                            0, 100, i18n("%"),
                            enabledLink,
                            strengthRangeReader)
{

}

KisCurveOptionWidget::KisCurveOptionWidget(lager::cursor<KisCurveOptionDataCommon> optionData,
                                             PaintopCategory category,
                                             const QString &curveMinLabel, const QString &curveMaxLabel,
                                             int curveMinValue, int curveMaxValue, const QString &curveValueSuffix,
                                             lager::reader<bool> enabledLink,
                                             std::optional<lager::reader<std::tuple<qreal, qreal>>> strengthRangeReader)
    : KisCurveOptionWidget(optionData,
                            category,
                            curveMinLabel, curveMaxLabel,
                            curveMinValue, curveMaxValue, curveValueSuffix,
                            i18n("Strength: "), i18n("%"), 100.0,
                            enabledLink,
                            strengthRangeReader)
{
}

KisCurveOptionWidget::KisCurveOptionWidget(lager::cursor<KisCurveOptionDataCommon> optionData,
                                             PaintopCategory category,
                                             const QString &curveMinLabel, const QString &curveMaxLabel,
                                             int curveMinValue, int curveMaxValue, const QString &curveValueSuffix,
                                             const QString &strengthPrefix, const QString &strengthSuffix,
                                             qreal strengthDisplayMultiplier,
                                             lager::reader<bool> enabledLink,
                                             std::optional<lager::reader<std::tuple<qreal, qreal>>> strengthRangeReader)
    : KisCurveOptionWidget(optionData,
                            category,
                            strengthPrefix, strengthSuffix, strengthDisplayMultiplier,
                            enabledLink,
                            strengthRangeReader,
                            KisCurveRangeModel::factory(curveMinLabel,
                                                        curveMaxLabel,
                                                        curveMinValue,
                                                        curveMaxValue,
                                                        curveValueSuffix),
                            KisCurveOptionInputControlsStrategyInt::factory(),
                            {},
                            SupportsCommonCurve | SupportsCurveMode)
{
}

KisCurveOptionWidget::KisCurveOptionWidget(lager::cursor<KisCurveOptionDataCommon> optionData,
                                             PaintopCategory category,
                                             const QString &strengthPrefix, const QString &strengthSuffix,
                                             qreal strengthDisplayMultiplier,
                                             lager::reader<bool> enabledLink,
                                             std::optional<lager::reader<std::tuple<qreal, qreal>>> strengthRangeReader,
                                             KisCurveRangeModelFactory curveRangeModelFactory,
                                             KisCurveOptionInputControlsStrategyFactory inputControlsFactory,
                                             KisCurveOptionRangeControlsStrategyFactory rangeControlsFactory,
                                             Flags flags)
    : KisPaintOpOption(optionData->id.name(), category, optionData[&KisCurveOptionDataCommon::isChecked], enabledLink)
    , m_widget(new QWidget)
    , m_curveOptionWidget(new Ui_WdgCurveOption2())
    , m_d(new Private(optionData, enabledLink, strengthRangeReader,
                      strengthDisplayMultiplier,
                      curveRangeModelFactory))
{
    using namespace KisWidgetConnectionUtils;

    setObjectName("KisCurveOptionWidget");

    m_curveOptionWidget->setupUi(m_widget);
    m_curveOptionWidget->sensorSelector->setOptionDataCursor(m_d->model.optionData);

    KIS_SAFE_ASSERT_RECOVER (inputControlsFactory) {
        inputControlsFactory = KisCurveOptionInputControlsStrategyInt::factory();
    }

    m_curveInputControlsStrategy.reset(
        inputControlsFactory(m_d->model.rangeModel.get(),
                             m_curveOptionWidget->curveWidget,
                             m_curveOptionWidget->intInPlaceholder,
                             m_curveOptionWidget->intOutPlaceholder));

    if (rangeControlsFactory) {
        m_curveRangeControlsStrategy.reset(
            rangeControlsFactory(m_d->model.rangeModel.get(),
                                 m_curveOptionWidget->wdgRangeControlsPlaceholder));
    } else {
        m_curveOptionWidget->wdgRangeControlsPlaceholder->setVisible(false);
    }

    setConfigurationPage(m_widget);

    connect(&m_d->model, &KisCurveOptionModel::curveXMinLabelChanged,
            m_curveOptionWidget->label_xmin, &QLabel::setText);
    m_curveOptionWidget->label_xmin->setText(m_d->model.curveXMinLabel());

    connect(&m_d->model, &KisCurveOptionModel::curveXMaxLabelChanged,
            m_curveOptionWidget->label_xmax, &QLabel::setText);
    m_curveOptionWidget->label_xmax->setText(m_d->model.curveXMaxLabel());

    connect(&m_d->model, &KisCurveOptionModel::curveYMinLabelChanged,
            m_curveOptionWidget->label_ymin, &QLabel::setText);
    m_curveOptionWidget->label_ymin->setText(m_d->model.curveYMinLabel());

    connect(&m_d->model, &KisCurveOptionModel::curveYMaxLabelChanged,
            m_curveOptionWidget->label_ymax, &QLabel::setText);
    m_curveOptionWidget->label_ymax->setText(m_d->model.curveYMaxLabel());

    m_d->model.activeSensorIdData.bind(
        std::bind(qOverload<const QString&>(&KisMultiSensorsSelector::setCurrent),
                  m_curveOptionWidget->sensorSelector,
                  std::placeholders::_1));
    connect(m_curveOptionWidget->sensorSelector,
            &KisMultiSensorsSelector::highlightedSensorChanged,
            std::bind(&KisCurveOptionModel::setactiveSensorId, &m_d->model, std::placeholders::_1));

    // set all the icons for the curve preset shapes
    updateThemedIcons();

    // various curve preset buttons with predefined curves
    connect(m_curveOptionWidget->linearCurveButton, SIGNAL(clicked(bool)), this, SLOT(changeCurveLinear()));
    connect(m_curveOptionWidget->revLinearButton, SIGNAL(clicked(bool)), this, SLOT(changeCurveReverseLinear()));
    connect(m_curveOptionWidget->jCurveButton, SIGNAL(clicked(bool)), this, SLOT(changeCurveJShape()));
    connect(m_curveOptionWidget->lCurveButton, SIGNAL(clicked(bool)), this, SLOT(changeCurveLShape()));
    connect(m_curveOptionWidget->sCurveButton, SIGNAL(clicked(bool)), this, SLOT(changeCurveSShape()));
    connect(m_curveOptionWidget->reverseSCurveButton, SIGNAL(clicked(bool)), this, SLOT(changeCurveReverseSShape()));
    connect(m_curveOptionWidget->uCurveButton, SIGNAL(clicked(bool)), this, SLOT(changeCurveUShape()));
    connect(m_curveOptionWidget->revUCurveButton, SIGNAL(clicked(bool)), this, SLOT(changeCurveArchShape()));

    m_curveOptionWidget->strengthSlider->setPrefix(strengthPrefix);
    m_curveOptionWidget->strengthSlider->setSuffix(strengthSuffix);

    connect(m_curveOptionWidget->strengthSlider, qOverload<qreal>(&KisDoubleSliderSpinBox::valueChanged),
            &m_d->model, &KisCurveOptionModel::setstrengthValueDenorm);

    const bool useFloatingPointStrength = flags & UseFloatingPointStrength;

    if (useFloatingPointStrength) {
        m_curveOptionWidget->strengthSlider->setSingleStep(0.01);
    }

    m_d->model.LAGER_QT(effectiveStrengthStateDenorm).bind(
                kismpl::unzip_wrapper([this, useFloatingPointStrength] (qreal value, qreal min, qreal max) {
                    KisSignalsBlocker b(m_curveOptionWidget->strengthSlider);
                    m_curveOptionWidget->strengthSlider->setRange(min, max, useFloatingPointStrength ? 2 : 0);
                    m_curveOptionWidget->strengthSlider->setValue(value);
                }));

    connectControl(m_curveOptionWidget->curveWidget, &m_d->model, "displayedCurve");

    m_d->model.LAGER_QT(useCurve).bind(std::bind(&KisCurveOptionWidget::setCurveWidgetsEnabled, this, std::placeholders::_1));
    connectControl(m_curveOptionWidget->checkBoxUseCurve, &m_d->model, "useCurve");

    if (flags & SupportsCommonCurve) {
        connectControl(m_curveOptionWidget->checkBoxUseSameCurve, &m_d->model, "useSameCurve");
    } else {
        m_curveOptionWidget->checkBoxUseSameCurve->setVisible(false);
    }

    if (flags & SupportsCurveMode) {
        connectControl(m_curveOptionWidget->curveMode, &m_d->model, "curveMode");
    } else {
        m_curveOptionWidget->curveMode->setVisible(false);
        m_curveOptionWidget->lblCurveMode->setVisible(false);
    }

    m_d->model.optionData.bind(std::bind(&KisCurveOptionWidget::emitSettingChanged, this));
}

KisCurveOptionWidget::~KisCurveOptionWidget()
{
    delete m_curveOptionWidget;
}

void KisCurveOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    m_d->model.bakedOptionData().write(setting.data());
}

void KisCurveOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisCurveOptionDataCommon data(*m_d->model.optionData);
    data.read(setting.data());
    m_d->model.optionData.set(data);
}

bool KisCurveOptionWidget::isCheckable() const
{
    return m_d->model.isCheckable();
}

lager::cursor<qreal> KisCurveOptionWidget::strengthValueDenorm()
{
    return m_d->model.LAGER_QT(strengthValueDenorm);
}

void KisCurveOptionWidget::setCurveWidgetsEnabled(bool value)
{
    m_curveOptionWidget->checkBoxUseSameCurve->setEnabled(value);
    m_curveOptionWidget->curveWidget->setEnabled(value);
    m_curveOptionWidget->sensorSelector->setEnabled(value);
    m_curveOptionWidget->label_xmax->setEnabled(value);
    m_curveOptionWidget->label_xmin->setEnabled(value);
    m_curveOptionWidget->label_ymax->setEnabled(value);
    m_curveOptionWidget->label_ymin->setEnabled(value);
    m_curveOptionWidget->intInPlaceholder->setEnabled(value);
    m_curveOptionWidget->intOutPlaceholder->setEnabled(value);

    m_curveOptionWidget->linearCurveButton->setEnabled(value);
    m_curveOptionWidget->revLinearButton->setEnabled(value);
    m_curveOptionWidget->jCurveButton->setEnabled(value);
    m_curveOptionWidget->lCurveButton->setEnabled(value);
    m_curveOptionWidget->revUCurveButton->setEnabled(value);
    m_curveOptionWidget->reverseSCurveButton->setEnabled(value);
    m_curveOptionWidget->uCurveButton->setEnabled(value);
    m_curveOptionWidget->sCurveButton->setEnabled(value);

    m_curveOptionWidget->curveMode->setEnabled(value);
    m_curveOptionWidget->lblCurveMode->setEnabled(value);

}

QWidget* KisCurveOptionWidget::curveWidget()
{
    return m_widget;
}

void KisCurveOptionWidget::changeCurveLinear()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,0));
    points.push_back(QPointF(1,1));
    m_d->model.setdisplayedCurve(KisCubicCurve(points).toString());
}

void KisCurveOptionWidget::changeCurveReverseLinear()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,1));
    points.push_back(QPointF(1,0));
    m_d->model.setdisplayedCurve(KisCubicCurve(points).toString());
}

void KisCurveOptionWidget::changeCurveSShape()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,0));
    points.push_back(QPointF(0.25,0.1));
    points.push_back(QPointF(0.75,0.9));
    points.push_back(QPointF(1, 1));
    m_d->model.setdisplayedCurve(KisCubicCurve(points).toString());
}


void KisCurveOptionWidget::changeCurveReverseSShape()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,1));
    points.push_back(QPointF(0.25,0.9));
    points.push_back(QPointF(0.75,0.1));
    points.push_back(QPointF(1,0));
    m_d->model.setdisplayedCurve(KisCubicCurve(points).toString());
}

void KisCurveOptionWidget::changeCurveJShape()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,0));
    points.push_back(QPointF(0.35,0.1));
    points.push_back(QPointF(1,1));
    m_d->model.setdisplayedCurve(KisCubicCurve(points).toString());
}

void KisCurveOptionWidget::changeCurveLShape()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,1));
    points.push_back(QPointF(0.25,0.48));
    points.push_back(QPointF(1,0));
    m_d->model.setdisplayedCurve(KisCubicCurve(points).toString());
}

void KisCurveOptionWidget::changeCurveUShape()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,1));
    points.push_back(QPointF(0.5,0));
    points.push_back(QPointF(1,1));
    m_d->model.setdisplayedCurve(KisCubicCurve(points).toString());
}

void KisCurveOptionWidget::changeCurveArchShape()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,0));
    points.push_back(QPointF(0.5,1));
    points.push_back(QPointF(1,0));
    m_d->model.setdisplayedCurve(KisCubicCurve(points).toString());
}

void KisCurveOptionWidget::updateThemedIcons()
{
    // set all the icons for the curve preset shapes
    m_curveOptionWidget->linearCurveButton->setIcon(KisIconUtils::loadIcon("curve-preset-linear"));
    m_curveOptionWidget->revLinearButton->setIcon(KisIconUtils::loadIcon("curve-preset-linear-reverse"));
    m_curveOptionWidget->jCurveButton->setIcon(KisIconUtils::loadIcon("curve-preset-j"));
    m_curveOptionWidget->lCurveButton->setIcon(KisIconUtils::loadIcon("curve-preset-l"));
    m_curveOptionWidget->sCurveButton->setIcon(KisIconUtils::loadIcon("curve-preset-s"));
    m_curveOptionWidget->reverseSCurveButton->setIcon(KisIconUtils::loadIcon("curve-preset-s-reverse"));
    m_curveOptionWidget->uCurveButton->setIcon(KisIconUtils::loadIcon("curve-preset-u"));
    m_curveOptionWidget->revUCurveButton->setIcon(KisIconUtils::loadIcon("curve-preset-arch"));

    // this helps make the checkboxes show themselves on the dark color themes
    QPalette pal = m_curveOptionWidget->sensorSelector->palette();
    QPalette newPalette = pal;
    newPalette.setColor(QPalette::Active, QPalette::Window, pal.text().color() );
    m_curveOptionWidget->sensorSelector->setPalette(newPalette);

}
