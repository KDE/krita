/*
 *  Copyright (c) 2011 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 or later of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_multi_sensors_selector.h"

#include "ui_wdgmultisensorsselector.h"
#include "kis_multi_sensors_model_p.h"
#include "kis_dynamic_sensor.h"

struct KisMultiSensorsSelector::Private
{
    Ui_WdgMultiSensorsSelector form;
    KisMultiSensorsModel* model;
    QWidget* currentConfigWidget;
    QHBoxLayout* layout;
};

KisMultiSensorsSelector::KisMultiSensorsSelector(QWidget* parent) : QWidget(parent), d(new Private)
{
    d->currentConfigWidget = 0;
    d->form.setupUi(this);
    d->model = new KisMultiSensorsModel(this);
    connect(d->model, SIGNAL(sensorChanged(KisDynamicSensor*)), SIGNAL(sensorChanged(KisDynamicSensor*)));
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

void KisMultiSensorsSelector::setCurrent(KisDynamicSensor* _sensor)
{
    d->model->setCurrentSensor(_sensor);
    d->form.sensorsList->setCurrentIndex(d->model->sensorIndex(_sensor)); // make sure the first element is selected
    KisDynamicSensor* sensor = currentHighlighted();
    if (!sensor)
    {
        sensor = d->model->getSensor(d->model->index(0, 0));
    }
    emit(highlightedSensorChanged(sensor));
}

KisDynamicSensor* KisMultiSensorsSelector::currentHighlighted()
{
    return d->model->getSensor(d->form.sensorsList->currentIndex());
}

KisDynamicSensor* KisMultiSensorsSelector::current()
{
    return 0;
}

void KisMultiSensorsSelector::sensorActivated(const QModelIndex& index)
{
    delete d->currentConfigWidget;
    KisDynamicSensor* sensor = d->model->getSensor(index);
    if (sensor) {
        d->currentConfigWidget = sensor->createConfigurationWidget(d->form.widgetConfiguration, this);
        if(d->currentConfigWidget)
        {
            d->layout->addWidget(d->currentConfigWidget);
        }
        emit(highlightedSensorChanged(sensor));
    }
}

void KisMultiSensorsSelector::setCurrentCurve(const KisCubicCurve& curve, bool useSameCurve)
{
    d->model->setCurrentCurve(d->form.sensorsList->currentIndex(), curve, useSameCurve);
}
