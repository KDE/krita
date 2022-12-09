/*
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2009 Sven Langkamp <sven.langkamp@gmail.com>
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 * SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisCurveOptionWidget2.h"

#include "ui_wdgcurveoption2.h"
#include "widgets/kis_curve_widget.h"
#include "kis_global.h"
#include "kis_icon_utils.h"

#include <kis_signals_blocker.h>
#include <KisCurveOptionModel.h>
#include <KisWidgetConnectionUtils.h>

#include <KisZug.h>
#include <kis_algebra_2d.h>

#include <KisMultiSensorsSelector2.h>
#include <KisCurveRangeModel.h>
#include <KisCurveOptionInputControlsStrategy.h>

struct KisCurveOptionWidget2::Private
{
    Private(lager::cursor<KisCurveOptionDataCommon> optionData,
            lager::reader<bool> enabledLink,
            std::optional<lager::reader<std::tuple<qreal, qreal>>> rangeReader,
            KisCurveRangeModelFactory curveRangeFactory)
        : model(optionData, enabledLink, rangeReader, curveRangeFactory)
    {}

    KisCurveOptionModel model;

    int curveMinValue;
    int curveMaxValue;
    QString curveValueSuffix;
};

KisCurveOptionWidget2::KisCurveOptionWidget2(lager::cursor<KisCurveOptionDataCommon> optionData,
                                             PaintopCategory category,
                                             lager::reader<bool> enabledLink,
                                             std::optional<lager::reader<std::tuple<qreal, qreal>>> rangeReader,
                                             KisCurveRangeModelFactory curveRangeFactory,
                                             KisCurveOptionInputControlsStrategyFactory inputControlsFactory,
                                             KisCurveOptionRangeControlsStrategyFactory rangeControlsFactory)
    : KisCurveOptionWidget2(optionData,
                            category,
                            i18n("0%"), i18n("100%"),
                            enabledLink,
                            rangeReader,
                            curveRangeFactory,
                            inputControlsFactory,
                            rangeControlsFactory)
{
}

KisCurveOptionWidget2::KisCurveOptionWidget2(lager::cursor<KisCurveOptionDataCommon> optionData,
                                             PaintopCategory category,
                                             const QString &curveMinLabel, const QString &curveMaxLabel,
                                             lager::reader<bool> enabledLink,
                                             std::optional<lager::reader<std::tuple<qreal, qreal>>> rangeReader,
                                             KisCurveRangeModelFactory curveRangeFactory,
                                             KisCurveOptionInputControlsStrategyFactory inputControlsFactory,
                                             KisCurveOptionRangeControlsStrategyFactory rangeControlsFactory)
    : KisCurveOptionWidget2(optionData,
                            category,
                            curveMinLabel, curveMaxLabel,
                            0, 100, i18n("%"),
                            i18n("Strength: "), i18n("%"),
                            enabledLink,
                            rangeReader,
                            curveRangeFactory,
                            inputControlsFactory,
                            rangeControlsFactory)
{

}

KisCurveOptionWidget2::KisCurveOptionWidget2(lager::cursor<KisCurveOptionDataCommon> optionData,
                                             PaintopCategory category,
                                             const QString &curveMinLabel, const QString &curveMaxLabel,
                                             int curveMinValue, int curveMaxValue, const QString &curveValueSuffix,
                                             lager::reader<bool> enabledLink,
                                             std::optional<lager::reader<std::tuple<qreal, qreal>>> rangeReader,
                                             KisCurveRangeModelFactory curveRangeFactory,
                                             KisCurveOptionInputControlsStrategyFactory inputControlsFactory,
                                             KisCurveOptionRangeControlsStrategyFactory rangeControlsFactory)
    : KisCurveOptionWidget2(optionData,
                            category,
                            curveMinLabel, curveMaxLabel,
                            curveMinValue, curveMaxValue, curveValueSuffix,
                            i18n("Strength: "), i18n("%"),
                            enabledLink,
                            rangeReader,
                            curveRangeFactory,
                            inputControlsFactory,
                            rangeControlsFactory)
{
}

KisCurveOptionWidget2::KisCurveOptionWidget2(lager::cursor<KisCurveOptionDataCommon> optionData,
                                             PaintopCategory category,
                                             const QString &curveMinLabel, const QString &curveMaxLabel,
                                             int curveMinValue, int curveMaxValue, const QString &curveValueSuffix,
                                             const QString &strengthPrefix, const QString &strengthSuffix,
                                             lager::reader<bool> enabledLink,
                                             std::optional<lager::reader<std::tuple<qreal, qreal>>> rangeReader,
                                             KisCurveRangeModelFactory curveRangeFactory,
                                             KisCurveOptionInputControlsStrategyFactory inputControlsFactory,
                                             KisCurveOptionRangeControlsStrategyFactory rangeControlsFactory)
    : KisPaintOpOption(optionData->id.name(), category, optionData[&KisCurveOptionDataCommon::isChecked], enabledLink)
    , m_widget(new QWidget)
    , m_curveOptionWidget(new Ui_WdgCurveOption2())
    , m_d(new Private(optionData, enabledLink, rangeReader, 
                      curveRangeFactory ?
                          curveRangeFactory :
                          KisCurveRangeModel::factory(curveMinLabel,
                                                      curveMaxLabel,
                                                      curveMinValue,
                                                      curveMaxValue,
                                                      curveValueSuffix)))
{
    using namespace KisWidgetConnectionUtils;

    setObjectName("KisCurveOptionWidget2");

    m_curveOptionWidget->setupUi(m_widget);
    m_curveOptionWidget->sensorSelector->setOptionDataCursor(m_d->model.optionData);

    if (!inputControlsFactory) {
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
    hideRangeLabelsAndBoxes(true);

    m_d->curveMinValue = curveMinValue;
    m_d->curveMaxValue = curveMaxValue;
    m_d->curveValueSuffix = curveValueSuffix;

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
        std::bind(qOverload<const QString&>(&KisMultiSensorsSelector2::setCurrent),
                  m_curveOptionWidget->sensorSelector,
                  std::placeholders::_1));
    connect(m_curveOptionWidget->sensorSelector,
            &KisMultiSensorsSelector2::highlightedSensorChanged,
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

    m_d->model.LAGER_QT(effectiveStrengthStateDenorm).bind(
                kismpl::unzip_wrapper([this] (qreal value, qreal min, qreal max) {
                    KisSignalsBlocker b(m_curveOptionWidget->strengthSlider);
                    m_curveOptionWidget->strengthSlider->setRange(min, max);
                    m_curveOptionWidget->strengthSlider->setValue(value);
                }));

    m_d->model.LAGER_QT(displayedCurve).bind(
                std::bind(&KisCurveWidget::setCurve,
                                    m_curveOptionWidget->curveWidget,
                                    std::placeholders::_1));

    connect(m_curveOptionWidget->curveWidget,
            &KisCurveWidget::curveChanged,
            this,
            &KisCurveOptionWidget2::slotCurveChanged);

    m_d->model.LAGER_QT(useCurve).bind(std::bind(&KisCurveOptionWidget2::setCurveWidgetsEnabled, this, std::placeholders::_1));
    connectControl(m_curveOptionWidget->checkBoxUseCurve, &m_d->model, "useCurve");
    connectControl(m_curveOptionWidget->checkBoxUseSameCurve, &m_d->model, "useSameCurve");
    connectControl(m_curveOptionWidget->curveMode, &m_d->model, "curveMode");

    m_d->model.optionData.bind(std::bind(&KisCurveOptionWidget2::emitSettingChanged, this));
}

KisCurveOptionWidget2::~KisCurveOptionWidget2()
{
    delete m_curveOptionWidget;
}

void KisCurveOptionWidget2::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    m_d->model.bakedOptionData().write(setting.data());
}

void KisCurveOptionWidget2::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisCurveOptionDataCommon data(*m_d->model.optionData);
    data.read(setting.data());
    m_d->model.optionData.set(data);
}

bool KisCurveOptionWidget2::isCheckable() const
{
    return m_d->model.isCheckable();
}

void KisCurveOptionWidget2::setCurveWidgetsEnabled(bool value)
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

QWidget* KisCurveOptionWidget2::curveWidget()
{
    return m_widget;
}

void KisCurveOptionWidget2::slotCurveChanged(const KisCubicCurve &curve)
{
    const QString string = curve.toString();
    m_d->model.setdisplayedCurve(string);
}

void KisCurveOptionWidget2::changeCurveLinear()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,0));
    points.push_back(QPointF(1,1));
    m_d->model.setdisplayedCurve(KisCubicCurve(points).toString());
}

void KisCurveOptionWidget2::changeCurveReverseLinear()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,1));
    points.push_back(QPointF(1,0));
    m_d->model.setdisplayedCurve(KisCubicCurve(points).toString());
}

void KisCurveOptionWidget2::changeCurveSShape()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,0));
    points.push_back(QPointF(0.25,0.1));
    points.push_back(QPointF(0.75,0.9));
    points.push_back(QPointF(1, 1));
    m_d->model.setdisplayedCurve(KisCubicCurve(points).toString());
}


void KisCurveOptionWidget2::changeCurveReverseSShape()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,1));
    points.push_back(QPointF(0.25,0.9));
    points.push_back(QPointF(0.75,0.1));
    points.push_back(QPointF(1,0));
    m_d->model.setdisplayedCurve(KisCubicCurve(points).toString());
}

void KisCurveOptionWidget2::changeCurveJShape()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,0));
    points.push_back(QPointF(0.35,0.1));
    points.push_back(QPointF(1,1));
    m_d->model.setdisplayedCurve(KisCubicCurve(points).toString());
}

void KisCurveOptionWidget2::changeCurveLShape()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,1));
    points.push_back(QPointF(0.25,0.48));
    points.push_back(QPointF(1,0));
    m_d->model.setdisplayedCurve(KisCubicCurve(points).toString());
}

void KisCurveOptionWidget2::changeCurveUShape()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,1));
    points.push_back(QPointF(0.5,0));
    points.push_back(QPointF(1,1));
    m_d->model.setdisplayedCurve(KisCubicCurve(points).toString());
}

void KisCurveOptionWidget2::changeCurveArchShape()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,0));
    points.push_back(QPointF(0.5,1));
    points.push_back(QPointF(1,0));
    m_d->model.setdisplayedCurve(KisCubicCurve(points).toString());
}

void KisCurveOptionWidget2::updateThemedIcons()
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
    newPalette.setColor(QPalette::Active, QPalette::Background, pal.text().color() );
    m_curveOptionWidget->sensorSelector->setPalette(newPalette);

}

void KisCurveOptionWidget2::hideRangeLabelsAndBoxes(bool isHidden) {

    m_curveOptionWidget->xMaxBox->setHidden(isHidden);
    m_curveOptionWidget->xMinBox->setHidden(isHidden);
    m_curveOptionWidget->yMaxBox->setHidden(isHidden);
    m_curveOptionWidget->yMinBox->setHidden(isHidden);

    m_curveOptionWidget->xRangeLabel->setHidden(isHidden);
    m_curveOptionWidget->yRangeLabel->setHidden(isHidden);
    m_curveOptionWidget->toLabel1->setHidden(isHidden);
    m_curveOptionWidget->toLabel2->setHidden(isHidden);

}
