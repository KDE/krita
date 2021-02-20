/*
 *  SPDX-FileCopyrightText: 2011 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "kis_multi_sensors_selector.h"

#include "ui_wdgmultisensorsselector.h"
#include "kis_multi_sensors_model_p.h"
#include "kis_dynamic_sensor.h"
#include "kis_curve_option.h"

struct KisMultiSensorsSelector::Private {
    Ui_WdgMultiSensorsSelector form;
    KisMultiSensorsModel* model;
    QWidget* currentConfigWidget;
    QHBoxLayout* layout;
};

KisMultiSensorsSelector::KisMultiSensorsSelector(QWidget* parent)
    : QWidget(parent)
    , d(new Private)
{
    d->currentConfigWidget = 0;
    d->form.setupUi(this);
    d->model = new KisMultiSensorsModel(this);
    connect(d->model, SIGNAL(sensorChanged(KisDynamicSensorSP)), SIGNAL(sensorChanged(KisDynamicSensorSP)));
    connect(d->model, SIGNAL(parametersChanged()), SIGNAL(parametersChanged()));

    connect(d->form.sensorsList, SIGNAL(activated(QModelIndex)), SLOT(setCurrent(QModelIndex)));
    connect(d->form.sensorsList, SIGNAL(clicked(QModelIndex)), SLOT(setCurrent(QModelIndex)));
    d->form.sensorsList->setModel(d->model);
    d->layout = new QHBoxLayout(d->form.widgetConfiguration);
}

KisMultiSensorsSelector::~KisMultiSensorsSelector()
{
    delete d;
}

void KisMultiSensorsSelector::setCurveOption(KisCurveOption *curveOption)
{
    d->model->setCurveOption(curveOption);

    if(!(curveOption->activeSensors().size() > 0))
        return ;
    KisDynamicSensorSP s = curveOption->activeSensors().first();
    if (!s) {
        s = curveOption->sensors().first();
    }
    setCurrent(s);
}

void KisMultiSensorsSelector::setCurrent(KisDynamicSensorSP _sensor)
{
    d->form.sensorsList->setCurrentIndex(d->model->sensorIndex(_sensor)); // make sure the first element is selected

    // HACK ALERT: make sure the signal is delivered to us. Without this line it isn't.
    sensorActivated(d->model->sensorIndex(_sensor));

    KisDynamicSensorSP sensor = currentHighlighted();
    if (!sensor) {
        sensor = d->model->getSensor(d->model->index(0, 0));
    }
    emit(highlightedSensorChanged(sensor));
}

void KisMultiSensorsSelector::setCurrent(const QModelIndex& index)
{
    d->form.sensorsList->setCurrentIndex(index); // make sure the first element is selected

    // HACK ALERT: make sure the signal is delivered to us. Without this line it isn't.
    sensorActivated(index);

    KisDynamicSensorSP sensor = currentHighlighted();
    if (!sensor) {
        sensor = d->model->getSensor(d->model->index(0, 0));
    }
    emit(highlightedSensorChanged(sensor));
}

KisDynamicSensorSP KisMultiSensorsSelector::currentHighlighted()
{
    return d->model->getSensor(d->form.sensorsList->currentIndex());
}

void KisMultiSensorsSelector::sensorActivated(const QModelIndex& index)
{
    delete d->currentConfigWidget;
    KisDynamicSensorSP sensor = d->model->getSensor(index);
    if (sensor) {
        d->currentConfigWidget = sensor->createConfigurationWidget(d->form.widgetConfiguration, this);
        if (d->currentConfigWidget) {
            d->layout->addWidget(d->currentConfigWidget);
        }
        emit(highlightedSensorChanged(sensor));
    }
}

void KisMultiSensorsSelector::setCurrentCurve(const KisCubicCurve& curve, bool useSameCurve)
{
    d->model->setCurrentCurve(d->form.sensorsList->currentIndex(), curve, useSameCurve);
}

void KisMultiSensorsSelector::reload()
{
    d->model->resetCurveOption();
}
