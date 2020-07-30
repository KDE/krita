#include "kis_mypaint_curve_option_widget.h"

#include "ui_wdgmypaintcurveoption.h"
#include "widgets/kis_curve_widget.h"
#include "kis_mypaintbrush_option.h"
#include "kis_my_paintop_settings_widget.h"
#include "kis_my_paintop_option.h"
#include "kis_global.h"
#include "kis_mypaint_curve_option.h"
#include "kis_signals_blocker.h"
#include "kis_icon_utils.h"
#include "kis_mypaint_curve_option.h"
#include "kis_my_paintop_option.h"
#include <QJsonDocument>
#include <QJsonObject>

inline void setLabel(QLabel* label, const KisCurveLabel& curve_label)
{
    if (curve_label.icon().isNull()) {
        label->setText(curve_label.name());
    }
    else {
        label->setPixmap(QPixmap::fromImage(curve_label.icon()));
    }
}

KisMyPaintCurveOptionWidget::KisMyPaintCurveOptionWidget(KisMyPaintCurveOption* curveOption, const QString &minLabel, const QString &maxLabel, bool hideSlider, KisMyPaintOpOption *baseOption)
    : KisPaintOpOption(curveOption->category(), curveOption->isChecked())
    , m_widget(new QWidget)
    , m_curveOptionWidget(new Ui_WdgMyPaintCurveOption())
    , m_curveOption(curveOption)
{
    setObjectName("KisMyPaintCurveOptionWidget");

    m_curveOptionWidget->setupUi(m_widget);
    setConfigurationPage(m_widget);    

    m_curveOptionWidget->sensorSelector->setCurveOption(curveOption);    

    updateSensorCurveLabels(m_curveOptionWidget->sensorSelector->currentHighlighted());
    updateCurve(m_curveOptionWidget->sensorSelector->currentHighlighted());

    connect(m_curveOptionWidget->curveWidget, SIGNAL(modified()), this, SLOT(slotModified()));
    connect(m_curveOptionWidget->sensorSelector, SIGNAL(parametersChanged()), SLOT(emitSettingChanged()));
    connect(m_curveOptionWidget->sensorSelector, SIGNAL(parametersChanged()), SLOT(updateLabelsOfCurrentSensor()));
    connect(m_curveOptionWidget->sensorSelector, SIGNAL(highlightedSensorChanged(KisDynamicOptionSP)), SLOT(updateSensorCurveLabels(KisDynamicOptionSP)));
    connect(m_curveOptionWidget->sensorSelector, SIGNAL(highlightedSensorChanged(KisDynamicOptionSP)), SLOT(updateCurve(KisDynamicOptionSP)));
    connect(m_curveOptionWidget->sensorSelector, SIGNAL(highlightedSensorChanged(KisDynamicOptionSP)), SLOT(updateRangeSpinBoxes(KisDynamicOptionSP)));
    connect(m_curveOptionWidget->checkBoxUseSameCurve, SIGNAL(stateChanged(int)), SLOT(slotUseSameCurveChanged()));
    connect(m_curveOptionWidget->xMinBox, SIGNAL(valueChanged(double)), SLOT(emitSettingChanged()));
    connect(m_curveOptionWidget->xMaxBox, SIGNAL(valueChanged(double)), SLOT(emitSettingChanged()));
    connect(m_curveOptionWidget->yMinBox, SIGNAL(valueChanged(double)), SLOT(emitSettingChanged()));
    connect(m_curveOptionWidget->yMaxBox, SIGNAL(valueChanged(double)), SLOT(emitSettingChanged()));

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


//    m_curveOptionWidget->label_ymin->setText(minLabel);
//    m_curveOptionWidget->label_ymax->setText(maxLabel);


    m_curveOptionWidget->strengthSlider->setRange(curveOption->minValue(), curveOption->maxValue(), 2);
    m_curveOptionWidget->strengthSlider->setValue(curveOption->value());
    m_curveOptionWidget->strengthSlider->setPrefix(i18n("Strength: "));
    m_curveOptionWidget->strengthSlider->setSuffix(i18n("%"));       

    if (hideSlider) {
         m_curveOptionWidget->strengthSlider->hide();
    }

    connect(m_curveOption, SIGNAL(unCheckUseCurve()), SLOT(slotUnCheckUseCurve()));
    connect(m_curveOptionWidget->checkBoxUseCurve, SIGNAL(stateChanged(int))  , SLOT(updateValues()));
    connect(m_curveOptionWidget->curveMode, SIGNAL(currentIndexChanged(int)), SLOT(updateMode()));
    connect(m_curveOptionWidget->strengthSlider, SIGNAL(valueChanged(qreal)), SLOT(updateValues()));  
}

KisMyPaintCurveOptionWidget::~KisMyPaintCurveOptionWidget()
{
    delete m_curveOption;
    delete m_curveOptionWidget;
}

void KisMyPaintCurveOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisDynamicOptionSP currentSensor = m_curveOptionWidget->sensorSelector->currentHighlighted();
    setting->setProperty(m_curveOption->name() + m_curveOptionWidget->sensorSelector->currentHighlighted()->id() + "XMIN", m_curveOptionWidget->xMinBox->value());
    setting->setProperty(m_curveOption->name() + m_curveOptionWidget->sensorSelector->currentHighlighted()->id() + "XMAX", m_curveOptionWidget->xMaxBox->value());
    setting->setProperty(m_curveOption->name() + m_curveOptionWidget->sensorSelector->currentHighlighted()->id() + "YMIN", m_curveOptionWidget->yMinBox->value());
    setting->setProperty(m_curveOption->name() + m_curveOptionWidget->sensorSelector->currentHighlighted()->id() + "YMAX", m_curveOptionWidget->yMaxBox->value());

    currentSensor->setXRangeMin(m_curveOptionWidget->xMinBox->value());
    currentSensor->setXRangeMax(m_curveOptionWidget->xMaxBox->value());
    currentSensor->setYRangeMin(m_curveOptionWidget->yMinBox->value());
    currentSensor->setYRangeMax(m_curveOptionWidget->yMaxBox->value());

    updateSensorCurveLabels(currentSensor);
    setBaseValue(setting, m_curveOptionWidget->strengthSlider->value());
    m_curveOption->writeOptionSetting(setting);    
}

void KisMyPaintCurveOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    //setting->dump();    
    updateValuesCalled = true;
    m_curveOption->readOptionSetting(setting);

    // Signals needs to be blocked, otherwise checking the checkbox will trigger
    //   setting the common curve to the widget curve, which is incorrect in this case.
    bool blockedBefore = m_curveOptionWidget->checkBoxUseSameCurve->blockSignals(true);
    m_curveOptionWidget->checkBoxUseSameCurve->setChecked(m_curveOption->isSameCurveUsed());
    m_curveOptionWidget->checkBoxUseSameCurve->blockSignals(blockedBefore);

    m_curveOptionWidget->checkBoxUseCurve->setChecked(m_curveOption->isCurveUsed());
    m_curveOptionWidget->strengthSlider->setValue(getBaseValue(setting));
    m_curveOptionWidget->curveMode->setCurrentIndex(m_curveOption->getCurveMode());

    disableWidgets(!m_curveOption->isCurveUsed());

    m_curveOptionWidget->sensorSelector->reload();
    if(m_curveOption->activeSensors().size() > 0)
        m_curveOptionWidget->sensorSelector->setCurrent(m_curveOption->activeSensors().first());
    updateSensorCurveLabels(m_curveOptionWidget->sensorSelector->currentHighlighted());
    updateCurve(m_curveOptionWidget->sensorSelector->currentHighlighted());
    updateRangeSpinBoxes(m_curveOptionWidget->sensorSelector->currentHighlighted());

    updateValuesCalled = false;
}

void KisMyPaintCurveOptionWidget::lodLimitations(KisPaintopLodLimitations *l) const
{
    m_curveOption->lodLimitations(l);
}

bool KisMyPaintCurveOptionWidget::isCheckable() const
{
    return m_curveOption->isCheckable();
}

bool KisMyPaintCurveOptionWidget::isChecked() const
{
    return m_curveOption->isChecked();
}

void KisMyPaintCurveOptionWidget::setChecked(bool checked)
{
    m_curveOption->setChecked(checked);
}

KisMyPaintCurveOption* KisMyPaintCurveOptionWidget::curveOption()
{
    return m_curveOption;
}

QWidget* KisMyPaintCurveOptionWidget::curveWidget()
{
    return m_widget;
}

void KisMyPaintCurveOptionWidget::slotModified()
{    
    if (!m_curveOption->isSameCurveUsed()) {        
        m_curveOptionWidget->sensorSelector->currentHighlighted()->setCurve(getWidgetCurve());     
    } else {
        m_curveOption->setCommonCurve(getWidgetCurve());
    }    
    emitSettingChanged();    
}

void KisMyPaintCurveOptionWidget::slotUseSameCurveChanged()
{
    // this is a slot that answers on "Share Curve across all settings" checkbox
    m_curveOption->setUseSameCurve(m_curveOptionWidget->checkBoxUseSameCurve->isChecked());
    if (m_curveOption->isSameCurveUsed()) {
        // !(UseSameCurve) => UseSameCurve
        // set the current curve to the common curve
        m_curveOption->setCommonCurve(getWidgetCurve());
    } else {
        updateCurve(m_curveOptionWidget->sensorSelector->currentHighlighted());
    }
    emitSettingChanged();
}

void KisMyPaintCurveOptionWidget::slotUnCheckUseCurve() {

    m_curveOptionWidget->checkBoxUseCurve->setChecked(false);
    updateValues();
}

void KisMyPaintCurveOptionWidget::updateSensorCurveLabels(KisDynamicOptionSP sensor) const
{
    if (sensor) {
//        m_curveOptionWidget->label_xmin->setText(KisMyPaintBrushOption::minimumLabel(sensor->sensorType()));
//        m_curveOptionWidget->label_xmax->setText(KisMyPaintBrushOption::maximumLabel(sensor->sensorType(), sensor->length()));

        m_curveOptionWidget->label_xmin->setText(sensor->minimumXLabel());
        m_curveOptionWidget->label_xmax->setText(sensor->maximumXLabel());
        m_curveOptionWidget->label_ymin->setText(sensor->minimumYLabel());
        m_curveOptionWidget->label_ymax->setText(sensor->maximumYLabel());

        int inMinValue = KisMyPaintBrushOption::minimumValue(sensor->sensorType());
        int inMaxValue = KisMyPaintBrushOption::maximumValue(sensor->sensorType(), sensor->length());
        QString inSuffix = KisMyPaintBrushOption::valueSuffix(sensor->sensorType());

        int outMinValue = m_curveOption->intMinValue();
        int outMaxValue = m_curveOption->intMaxValue();
        QString outSuffix = m_curveOption->valueSuffix();

        m_curveOptionWidget->intIn->setSuffix(inSuffix);
        m_curveOptionWidget->intOut->setSuffix(outSuffix);

        m_curveOptionWidget->curveWidget->setupInOutControls(m_curveOptionWidget->intIn,m_curveOptionWidget->intOut,
                                                         inMinValue,inMaxValue,outMinValue,outMaxValue);        
    }
}

void KisMyPaintCurveOptionWidget::updateCurve(KisDynamicOptionSP sensor)
{    
    if (sensor) {
        bool blockSignal = m_curveOptionWidget->curveWidget->blockSignals(true);
        KisCubicCurve curve = m_curveOption->isSameCurveUsed() ? m_curveOption->getCommonCurve() : sensor->curve();
        m_curveOptionWidget->curveWidget->setCurve(curve);
        m_curveOptionWidget->curveWidget->blockSignals(blockSignal);
    }    
}

void KisMyPaintCurveOptionWidget::updateRangeSpinBoxes(KisDynamicOptionSP sensor) const {

    m_curveOptionWidget->xMinBox->blockSignals(true);
    m_curveOptionWidget->xMaxBox->blockSignals(true);
    m_curveOptionWidget->yMinBox->blockSignals(true);
    m_curveOptionWidget->yMaxBox->blockSignals(true);

    m_curveOptionWidget->xMinBox->setValue( sensor->getXRangeMin());
    m_curveOptionWidget->xMaxBox->setValue( sensor->getXRangeMax());
    m_curveOptionWidget->yMinBox->setValue( sensor->getYRangeMin());
    m_curveOptionWidget->yMaxBox->setValue( sensor->getYRangeMax());

    m_curveOptionWidget->xMinBox->blockSignals(false);
    m_curveOptionWidget->xMaxBox->blockSignals(false);
    m_curveOptionWidget->yMinBox->blockSignals(false);
    m_curveOptionWidget->yMaxBox->blockSignals(false);
}

void KisMyPaintCurveOptionWidget::updateLabelsOfCurrentSensor()
{
    updateSensorCurveLabels(m_curveOptionWidget->sensorSelector->currentHighlighted());
    updateCurve(m_curveOptionWidget->sensorSelector->currentHighlighted());
}

void KisMyPaintCurveOptionWidget::updateValues()
{
    updateValuesCalled = true;
    m_curveOption->setValue(m_curveOptionWidget->strengthSlider->value());
    m_curveOption->setCurveUsed(m_curveOptionWidget->checkBoxUseCurve->isChecked());    
    disableWidgets(!m_curveOptionWidget->checkBoxUseCurve->isChecked());
    emitSettingChanged();
}

void KisMyPaintCurveOptionWidget::updateMode()
{
    m_curveOption->setCurveMode(m_curveOptionWidget->curveMode->currentIndex());
    emitSettingChanged();
}

void KisMyPaintCurveOptionWidget::changeCurveLinear()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,0));
    points.push_back(QPointF(1,1));
    m_curveOptionWidget->curveWidget->setCurve(KisCubicCurve(points));
}

void KisMyPaintCurveOptionWidget::changeCurveReverseLinear()
{
        QList<QPointF> points;
        points.push_back(QPointF(0,1));
        points.push_back(QPointF(1,0));
        m_curveOptionWidget->curveWidget->setCurve(KisCubicCurve(points));
}

void KisMyPaintCurveOptionWidget::changeCurveSShape()
{
        QList<QPointF> points;
        points.push_back(QPointF(0,0));
        points.push_back(QPointF(0.25,0.1));
        points.push_back(QPointF(0.75,0.9));
        points.push_back(QPointF(1, 1));
        m_curveOptionWidget->curveWidget->setCurve(KisCubicCurve(points));
}


void KisMyPaintCurveOptionWidget::changeCurveReverseSShape()
{
        QList<QPointF> points;
        points.push_back(QPointF(0,1));
        points.push_back(QPointF(0.25,0.9));
        points.push_back(QPointF(0.75,0.1));
        points.push_back(QPointF(1,0));
        m_curveOptionWidget->curveWidget->setCurve(KisCubicCurve(points));
}

void KisMyPaintCurveOptionWidget::changeCurveJShape()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,0));
    points.push_back(QPointF(0.35,0.1));
    points.push_back(QPointF(1,1));
    m_curveOptionWidget->curveWidget->setCurve(KisCubicCurve(points));
}

void KisMyPaintCurveOptionWidget::changeCurveLShape()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,1));
    points.push_back(QPointF(0.25,0.48));
    points.push_back(QPointF(1,0));
    m_curveOptionWidget->curveWidget->setCurve(KisCubicCurve(points));
}

void KisMyPaintCurveOptionWidget::changeCurveUShape()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,1));
    points.push_back(QPointF(0.5,0));
    points.push_back(QPointF(1,1));
    m_curveOptionWidget->curveWidget->setCurve(KisCubicCurve(points));
}

void KisMyPaintCurveOptionWidget::changeCurveArchShape()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,0));
    points.push_back(QPointF(0.5,1));
    points.push_back(QPointF(1,0));
    m_curveOptionWidget->curveWidget->setCurve(KisCubicCurve(points));
}

void KisMyPaintCurveOptionWidget::disableWidgets(bool disable)
{
    m_curveOptionWidget->checkBoxUseSameCurve->setDisabled(disable);
    m_curveOptionWidget->curveWidget->setDisabled(disable);
    m_curveOptionWidget->sensorSelector->setDisabled(disable);
    m_curveOptionWidget->label_xmax->setDisabled(disable);
    m_curveOptionWidget->label_xmin->setDisabled(disable);
    m_curveOptionWidget->label_ymax->setDisabled(disable);
    m_curveOptionWidget->label_ymin->setDisabled(disable);   
}


void KisMyPaintCurveOptionWidget::updateThemedIcons()
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

KisCubicCurve KisMyPaintCurveOptionWidget::getWidgetCurve()
{
    return m_curveOptionWidget->curveWidget->curve();
}

KisCubicCurve KisMyPaintCurveOptionWidget::getHighlightedSensorCurve()
{
    return m_curveOptionWidget->sensorSelector->currentHighlighted()->curve();
    KisCubicCurve curve;
    return curve;
}

float KisMyPaintCurveOptionWidget::getBaseValue(KisPropertiesConfigurationSP setting) {

    MyPaintBrush *brush = mypaint_brush_new();
    mypaint_brush_from_string(brush, setting->getProperty(MYPAINT_JSON).toByteArray());

    if(m_curveOption->currentSetting() == MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC)
        return log(setting->getFloat(MYPAINT_DIAMETER)/2);

    if(m_curveOption->currentSetting() == MYPAINT_BRUSH_SETTING_OPAQUE)
        return setting->getFloat(MYPAINT_OPACITY);

    if(m_curveOption->currentSetting() == MYPAINT_BRUSH_SETTING_HARDNESS)
        return setting->getFloat(MYPAINT_HARDNESS);

    return mypaint_brush_get_base_value(brush, m_curveOption->currentSetting());
}

void KisMyPaintCurveOptionWidget::setBaseValue(KisPropertiesConfigurationSP setting, float val) const {

    QJsonDocument doc = QJsonDocument::fromJson(setting->getProperty(MYPAINT_JSON).toByteArray());
    QJsonObject brush_json = doc.object();
    QVariantMap map = brush_json.toVariantMap();
    QVariantMap settings_map = map["settings"].toMap();
    QVariantMap name_map = settings_map[m_curveOption->name()].toMap();
    double base_val = name_map["base_value"].toDouble();

    name_map["base_value"] = val;
    settings_map[m_curveOption->name()] = name_map;
    map["settings"] = settings_map;

    QJsonObject json_obj = QJsonObject::fromVariantMap(map);
    QJsonDocument doc2(json_obj);

    setting->setProperty(MYPAINT_JSON, doc2.toJson());    

    if(m_curveOption->currentSetting() == MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC)
        setting->setProperty(MYPAINT_DIAMETER, exp(val)*2);

    if(m_curveOption->currentSetting() == MYPAINT_BRUSH_SETTING_HARDNESS)
        setting->setProperty(MYPAINT_HARDNESS, val);

    if(m_curveOption->currentSetting() == MYPAINT_BRUSH_SETTING_OPAQUE)
        setting->setProperty(MYPAINT_OPACITY, val);
}

KisDoubleSliderSpinBox* KisMyPaintCurveOptionWidget::slider() {

    return m_curveOptionWidget->strengthSlider;
}

void KisMyPaintCurveOptionWidget::refresh() {
    emitSettingChanged();
}
