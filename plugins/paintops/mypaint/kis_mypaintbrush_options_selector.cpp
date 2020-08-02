/*
 * Copyright (c) 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "kis_mypaintbrush_options_selector.h"

#include "ui_wdgmypaintbrushoptionsselector.h"
#include "kis_mypaintbrush_options_model.h"
#include "kis_mypaint_curve_option.h"
#include <QHBoxLayout>

class Ui_WdgMyPaintBrushOptionsSelector;
struct KisMyPaintBrushOptionsSelector::Private {
    Ui_WdgMyPaintBrushOptionsSelector form;
    KisMyPaintBrushOptionsModel* model;
    QWidget* currentConfigWidget;
    QHBoxLayout* layout;
};

KisMyPaintBrushOptionsSelector::KisMyPaintBrushOptionsSelector(QWidget* parent)
    : QWidget(parent)
    , d(new Private)
{
    d->currentConfigWidget = 0;
    d->form.setupUi(this);
    d->model = new KisMyPaintBrushOptionsModel(this);
    connect(d->model, SIGNAL(sensorChanged(KisDynamicOptionSP)), SIGNAL(sensorChanged(KisDynamicOptionSP)));
    connect(d->model, SIGNAL(parametersChanged()), SIGNAL(parametersChanged()));
    //connect(d->form.sensorsList, SIGNAL(activated(QModelIndex)), SLOT(sensorActivated(QModelIndex)));
    //connect(d->form.sensorsList, SIGNAL(clicked(QModelIndex)), SLOT(sensorActivated(QModelIndex)));

    connect(d->form.sensorsList, SIGNAL(activated(QModelIndex)), SLOT(setCurrent(QModelIndex)));
    connect(d->form.sensorsList, SIGNAL(clicked(QModelIndex)), SLOT(setCurrent(QModelIndex)));
    d->form.sensorsList->setModel(d->model);
    d->layout = new QHBoxLayout(d->form.widgetConfiguration);
}

KisMyPaintBrushOptionsSelector::~KisMyPaintBrushOptionsSelector()
{
    delete d;
}

void KisMyPaintBrushOptionsSelector::setCurveOption(KisMyPaintCurveOption *curveOption)
{
    d->model->setCurveOption(curveOption);

    if(!(curveOption->activeSensors().size() > 0))
        return ;
    KisDynamicOptionSP s = curveOption->activeSensors().first();
    if (!s) {
        s = curveOption->sensors().first();
    }
    setCurrent(s);
}

void KisMyPaintBrushOptionsSelector::setCurrent(KisDynamicOptionSP _sensor)
{
    d->form.sensorsList->setCurrentIndex(d->model->sensorIndex(_sensor)); // make sure the first element is selected

    // HACK ALERT: make sure the signal is delivered to us. Without this line it isn't.
    sensorActivated(d->model->sensorIndex(_sensor));

    KisDynamicOptionSP sensor = currentHighlighted();
    if (!sensor) {
        sensor = d->model->getSensor(d->model->index(0, 0));
    }
    emit(highlightedSensorChanged(sensor));
}

void KisMyPaintBrushOptionsSelector::setCurrent(const QModelIndex& index)
{
    d->form.sensorsList->setCurrentIndex(index); // make sure the first element is selected

    // HACK ALERT: make sure the signal is delivered to us. Without this line it isn't.
    sensorActivated(index);

    KisDynamicOptionSP sensor = currentHighlighted();
    if (!sensor) {
        sensor = d->model->getSensor(d->model->index(0, 0));
    }
    emit(highlightedSensorChanged(sensor));
}

KisDynamicOptionSP KisMyPaintBrushOptionsSelector::currentHighlighted()
{
    return d->model->getSensor(d->form.sensorsList->currentIndex());
}

void KisMyPaintBrushOptionsSelector::sensorActivated(const QModelIndex& index)
{
    delete d->currentConfigWidget;
    KisDynamicOptionSP sensor = d->model->getSensor(index);
    if (sensor) {
        d->currentConfigWidget = sensor->createConfigurationWidget(d->form.widgetConfiguration, this);
        if (d->currentConfigWidget) {
            d->layout->addWidget(d->currentConfigWidget);
        }
        emit(highlightedSensorChanged(sensor));
    }
}

void KisMyPaintBrushOptionsSelector::setCurrentCurve(const KisCubicCurve& curve, bool useSameCurve)
{
    d->model->setCurrentCurve(d->form.sensorsList->currentIndex(), curve, useSameCurve);
}

void KisMyPaintBrushOptionsSelector::reload()
{
    d->model->resetCurveOption();
}
