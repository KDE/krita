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

#include <KisCurveOptionModel.h>
#include <KisWidgetConnectionUtils.h>

#include <KisZug.h>

#include <KisMultiSensorsSelector2.h>
#include <KisDynamicSensorFactoryRegistry.h>

struct KisCurveOptionWidget2::Private
{
    Private(lager::cursor<KisCurveOptionData> _optionData)
        : optionData(_optionData),
          model(optionData)
    {}

    lager::cursor<KisCurveOptionData> optionData;
    KisCurveOptionModel model;

    int minValue;
    int maxValue;
    QString valueSuffix;
};

KisCurveOptionWidget2::KisCurveOptionWidget2(lager::cursor<KisCurveOptionData> optionData,
                                             const QString &minLabel, const QString &maxLabel,
                                             int minValue, int maxValue, const QString &valueSuffix,
                                             lager::reader<bool> enabledLink)
    : KisPaintOpOption(optionData->id.name(), optionData->category, optionData->isChecked)
    , m_widget(new QWidget)
    , m_curveOptionWidget(new Ui_WdgCurveOption2())
    , m_d(new Private(optionData))
{
    setObjectName("KisCurveOptionWidget2");

    m_d->minValue = minValue;
    m_d->maxValue = maxValue;
    m_d->valueSuffix = valueSuffix;

    enabledLink.bind(std::bind(&QWidget::setEnabled, m_widget, std::placeholders::_1));

    m_curveOptionWidget->setupUi(m_widget);
    m_curveOptionWidget->sensorSelector->setOptionDataCursor(m_d->optionData);

    setConfigurationPage(m_widget);

    hideRangeLabelsAndBoxes(true);

    using namespace KisWidgetConnectionUtils;

    m_d->model.m_activeSensorIdData.bind(
        std::bind(qOverload<const QString&>(&KisMultiSensorsSelector2::setCurrent),
                  m_curveOptionWidget->sensorSelector,
                  std::placeholders::_1));
    connect(m_curveOptionWidget->sensorSelector,
            &KisMultiSensorsSelector2::highlightedSensorChanged,
            std::bind(&KisCurveOptionModel::setactiveSensorId, &m_d->model, std::placeholders::_1));

    m_d->model.LAGER_QT(labelsState).bind(
        kismpl::unzip_wrapper(
            std::bind(&KisCurveOptionWidget2::updateSensorCurveLabels, this, std::placeholders::_1, std::placeholders::_2)));

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


    m_curveOptionWidget->label_ymin->setText(minLabel);
    m_curveOptionWidget->label_ymax->setText(maxLabel);

    m_curveOptionWidget->strengthSlider->setPrefix(i18n("Strength: "));
    m_curveOptionWidget->strengthSlider->setSuffix(i18n("%"));

    connectControl(m_curveOptionWidget->strengthSlider, &m_d->model, "value");

    m_d->model.LAGER_QT(range).bind(
                kismpl::unzip_wrapper(std::bind(&KisDoubleSliderSpinBox::setRange,
                                              m_curveOptionWidget->strengthSlider,
                                              std::placeholders::_1,
                                              std::placeholders::_2,
                                              0, true)));

    m_d->model.LAGER_QT(activeCurve).bind(
                zug::comp(std::bind(&KisCurveWidget::setCurve,
                                    m_curveOptionWidget->curveWidget,
                                    std::placeholders::_1),
                          [] (const QString &str) {KisCubicCurve curve; curve.fromString(str); return curve;}));

    connect(m_curveOptionWidget->curveWidget,
            &KisCurveWidget::curveChanged,
            this,
            &KisCurveOptionWidget2::slotCurveChanged);

    connectControl(m_curveOptionWidget->checkBoxUseCurve, &m_d->model, "useCurve");
    connectControl(m_curveOptionWidget->checkBoxUseSameCurve, &m_d->model, "useSameCurve");
    connectControl(m_curveOptionWidget->curveMode, &m_d->model, "curveMode");
}

KisCurveOptionWidget2::~KisCurveOptionWidget2()
{
    delete m_curveOptionWidget;
}

void KisCurveOptionWidget2::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    Q_UNUSED(setting);
}

void KisCurveOptionWidget2::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    Q_UNUSED(setting);
}

bool KisCurveOptionWidget2::isCheckable() const
{
    return m_d->model.isCheckable();
}

bool KisCurveOptionWidget2::isChecked() const
{
    return m_d->model.isChecked();
}

void KisCurveOptionWidget2::setChecked(bool checked)
{
    m_d->model.setisChecked(checked);
}

void KisCurveOptionWidget2::setEnabled(bool enabled)
{
    m_widget->setEnabled(enabled);
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
}

QWidget* KisCurveOptionWidget2::curveWidget()
{
    return m_widget;
}

void KisCurveOptionWidget2::slotCurveChanged(const KisCubicCurve &curve)
{
    const QString string = curve.toString();
    m_d->model.setactiveCurve(string);
}

void KisCurveOptionWidget2::updateSensorCurveLabels(const QString &sensorId, const int length)
{
    KisDynamicSensorFactory *factory =
            KisDynamicSensorFactoryRegistry::instance()->get(sensorId);

    KIS_SAFE_ASSERT_RECOVER_RETURN(factory);

    m_curveOptionWidget->label_xmin->setText(factory->minimumLabel());
    m_curveOptionWidget->label_xmax->setText(factory->maximumLabel(length));

    const int inMinValue = factory->minimumValue();
    const int inMaxValue = factory->maximumValue(length);
    const QString inSuffix = factory->valueSuffix();

    const int outMinValue = m_d->minValue;
    const int outMaxValue = m_d->maxValue;
    const QString outSuffix = m_d->valueSuffix;

    m_curveOptionWidget->intIn->setSuffix(inSuffix);
    m_curveOptionWidget->intOut->setSuffix(outSuffix);

    m_curveOptionWidget->curveWidget->setupInOutControls(m_curveOptionWidget->intIn,m_curveOptionWidget->intOut,
                                                         inMinValue,inMaxValue,outMinValue,outMaxValue);
}

void KisCurveOptionWidget2::changeCurveLinear()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,0));
    points.push_back(QPointF(1,1));
    m_d->model.setactiveCurve(KisCubicCurve(points).toString());
}

void KisCurveOptionWidget2::changeCurveReverseLinear()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,1));
    points.push_back(QPointF(1,0));
    m_d->model.setactiveCurve(KisCubicCurve(points).toString());
}

void KisCurveOptionWidget2::changeCurveSShape()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,0));
    points.push_back(QPointF(0.25,0.1));
    points.push_back(QPointF(0.75,0.9));
    points.push_back(QPointF(1, 1));
    m_d->model.setactiveCurve(KisCubicCurve(points).toString());
}


void KisCurveOptionWidget2::changeCurveReverseSShape()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,1));
    points.push_back(QPointF(0.25,0.9));
    points.push_back(QPointF(0.75,0.1));
    points.push_back(QPointF(1,0));
    m_d->model.setactiveCurve(KisCubicCurve(points).toString());
}

void KisCurveOptionWidget2::changeCurveJShape()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,0));
    points.push_back(QPointF(0.35,0.1));
    points.push_back(QPointF(1,1));
    m_d->model.setactiveCurve(KisCubicCurve(points).toString());
}

void KisCurveOptionWidget2::changeCurveLShape()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,1));
    points.push_back(QPointF(0.25,0.48));
    points.push_back(QPointF(1,0));
    m_d->model.setactiveCurve(KisCubicCurve(points).toString());
}

void KisCurveOptionWidget2::changeCurveUShape()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,1));
    points.push_back(QPointF(0.5,0));
    points.push_back(QPointF(1,1));
    m_d->model.setactiveCurve(KisCubicCurve(points).toString());
}

void KisCurveOptionWidget2::changeCurveArchShape()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,0));
    points.push_back(QPointF(0.5,1));
    points.push_back(QPointF(1,0));
    m_d->model.setactiveCurve(KisCubicCurve(points).toString());
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
