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
#include "kis_icon_utils.h"
#include <QComboBox>
#include <QListView>
#include <QCheckBox>


struct KisMultiSensorsSelector::Private {
    Ui_WdgMultiSensorsSelector form;
    KisMultiSensorsModel* model;
    QWidget* currentConfigWidget;
    QHBoxLayout* layout;
    QListView *sensorsList;
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


    d->sensorsList = new QListView(this);
    d->sensorsList->setSpacing(2);


    connect(d->sensorsList, SIGNAL(activated(QModelIndex)), SLOT(sensorActivated(QModelIndex)));
    connect(d->sensorsList, SIGNAL(entered(QModelIndex)), SLOT(sensorActivated(QModelIndex)));

    d->sensorsList->setModel(d->model);
    d->form.penSensorCombobox->setModel(d->model);
    d->form.penSensorCombobox->setView(d->sensorsList);


    connect(d->form.currentSensorEnabledCheckbox, SIGNAL(clicked(bool)), this, SLOT(slotSensorEnableChange(bool)));


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
   d->sensorsList->setCurrentIndex(d->model->sensorIndex(_sensor)); // make sure the first element is selected

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
    return d->model->getSensor(d->sensorsList->currentIndex());
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

        d->form.penSensorCombobox->setCurrentIndex(index.row());

        d->form.currentSensorEnabledCheckbox->setChecked(sensor->isActive());


        // how many sensors are active?
        int sensorsActive = 0;
        for( int i =0; i <= d->model->rowCount()-1 ; i++) {
            QModelIndex row = d->model->index(i, 0);
            KisDynamicSensorSP loopSensor = d->model->getSensor(row);
            if ( loopSensor->isActive()) {
                sensorsActive++;
            }
        }

        // if the sensor selected is the only one, don't allow the person
        // to disable it. You need at least one sensor active for it to be valid
        if (d->form.currentSensorEnabledCheckbox->isChecked() && sensorsActive == 1 ) {
            d->form.currentSensorEnabledCheckbox->setEnabled(false);
        } else {
            d->form.currentSensorEnabledCheckbox->setEnabled(true);
        }


        emit(highlightedSensorChanged(sensor));
    }
}

void KisMultiSensorsSelector::setCurrentCurve(const KisCubicCurve& curve, bool useSameCurve)
{
    d->model->setCurrentCurve(d->sensorsList->currentIndex(), curve, useSameCurve);
}

void KisMultiSensorsSelector::reload()
{
    d->model->resetCurveOption();
}

void KisMultiSensorsSelector::slotSensorEnableChange(bool enabled) {
    // get current index
    KisDynamicSensorSP sensor = currentHighlighted();

    // change the data model index so the enabled state is changed
    sensor->setActive(enabled);
}
