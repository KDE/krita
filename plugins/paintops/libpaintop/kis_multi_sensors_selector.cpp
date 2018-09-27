/*
 *  Copyright (c) 2011 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
    connect(d->form.sensorsList, SIGNAL(activated(QModelIndex)), SLOT(sensorActivated(QModelIndex)));
    connect(d->form.sensorsList, SIGNAL(clicked(QModelIndex)), SLOT(sensorActivated(QModelIndex)));
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
