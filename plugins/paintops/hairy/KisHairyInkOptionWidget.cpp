/*
 *  SPDX-FileCopyrightText: 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisHairyInkOptionWidget.h"

#include <lager/constant.hpp>
#include "ui_wdgInkOptions.h"

#include "KisHairyInkOptionModel.h"

namespace {


class KisInkOptionsWidget: public QWidget, public Ui::WdgInkOptions
{
public:
    KisInkOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);

        pressureSlider->setRange(0, 100, 0);
        pressureSlider->setSuffix(i18n("%"));

        bristleLengthSlider->setRange(0, 100, 0);
        bristleLengthSlider->setSuffix(i18n("%"));

        bristleInkAmountSlider->setRange(0, 100, 0);
        bristleInkAmountSlider->setSuffix(i18n("%"));

        inkDepletionSlider->setRange(0, 100, 0);
        inkDepletionSlider->setSuffix(i18n("%"));
    }
};


}


struct KisHairyInkOptionWidget::Private
{
    Private(lager::cursor<KisHairyInkOptionData> optionData)
        : model(optionData)
    {
    }

    KisHairyInkOptionModel model;
};

class ConnectCurveWidgetHelper : public QObject
{
    Q_OBJECT
public:

    ConnectCurveWidgetHelper(KisCurveWidget *parent)
        : QObject(parent),
          m_curveWidget(parent)
    {
        connect(parent, &KisCurveWidget::curveChanged,
                this, &ConnectCurveWidgetHelper::slotWidgetChanged);
    }
public Q_SLOTS:
    void slotWidgetChanged() {
        Q_EMIT sigWidgetChanged(m_curveWidget->curve());
    }

    void slotPropertyChanged(KisCubicCurve curve) {
        m_curveWidget->setCurve(curve);
    }

Q_SIGNALS:
    // this signal was added only in Qt 5.15
    void sigWidgetChanged(KisCubicCurve curve);

private:
    KisCurveWidget *m_curveWidget;
};

#include <QMetaProperty>
void connectControlCurve(KisCurveWidget *widget, QObject *source, const char *property)
{
    const QMetaObject* meta = source->metaObject();
    QMetaProperty prop = meta->property(meta->indexOfProperty(property));

    KIS_SAFE_ASSERT_RECOVER_RETURN(prop.hasNotifySignal());

    QMetaMethod signal = prop.notifySignal();

    KIS_SAFE_ASSERT_RECOVER_RETURN(signal.parameterCount() >= 1);
    KIS_SAFE_ASSERT_RECOVER_RETURN(signal.parameterType(0) == QMetaType::type("KisCubicCurve"));

    ConnectCurveWidgetHelper *helper = new ConnectCurveWidgetHelper(widget);

    const QMetaObject* dstMeta = helper->metaObject();

    QMetaMethod updateSlot = dstMeta->method(
                dstMeta->indexOfSlot("slotPropertyChanged(KisCubicCurve)"));
    QObject::connect(source, signal, helper, updateSlot);

    helper->slotPropertyChanged(prop.read(source).value<KisCubicCurve>());

    if (prop.isWritable()) {
        QObject::connect(helper, &ConnectCurveWidgetHelper::sigWidgetChanged,
                         source, [prop, source] (KisCubicCurve value) { prop.write(source, QVariant::fromValue(value)); });
    }
}

KisHairyInkOptionWidget::KisHairyInkOptionWidget(lager::cursor<KisHairyInkOptionData> optionData)
    : KisPaintOpOption(i18n("Ink depletion"), KisPaintOpOption::COLOR, optionData[&KisHairyInkOptionData::inkDepletionEnabled])
    , m_d(new Private(optionData))
{
    KisInkOptionsWidget *widget = new KisInkOptionsWidget();
    setObjectName("KisHairyInkOption");

    using namespace KisWidgetConnectionUtils;

    connectControl(widget->inkAmountSpinBox,  &m_d->model, "inkAmount");
    connectControl(widget->saturationCBox, &m_d->model, "useSaturation");
    connectControl(widget->opacityCBox, &m_d->model, "useOpacity");
    connectControl(widget->useWeightCHBox, &m_d->model, "useWeights");
    connectControl(widget->pressureSlider, &m_d->model, "pressureWeight");
    connectControl(widget->bristleLengthSlider, &m_d->model, "bristleLengthWeight");
    connectControl(widget->bristleInkAmountSlider, &m_d->model, "bristleInkAmountWeight");
    connectControl(widget->inkDepletionSlider, &m_d->model, "inkDepletionWeight");
    connectControlCurve(widget->inkCurve, &m_d->model, "inkDepletionCurve");
    connectControl(widget->soakInkCBox, &m_d->model, "useSoakInk");

    m_d->model.optionData.bind(std::bind(&KisHairyInkOptionWidget::emitSettingChanged, this));

    setConfigurationPage(widget);
}

KisHairyInkOptionWidget::~KisHairyInkOptionWidget()
{
}

void KisHairyInkOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisHairyInkOptionData data = *m_d->model.optionData;
    data.write(setting.data());
}

void KisHairyInkOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisHairyInkOptionData data = *m_d->model.optionData;
    data.read(setting.data());
    m_d->model.optionData.set(data);
}


#include "KisHairyInkOptionWidget.moc"
