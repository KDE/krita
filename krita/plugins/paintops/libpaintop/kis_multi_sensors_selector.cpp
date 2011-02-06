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

struct KisMultiSensorsSelector::Private
{
  Ui_WdgMultiSensorsSelector form;
  KisMultiSensorsModel* model;
  QWidget* currentConfigWidget;
  QHBoxLayout* layout;
};

KisMultiSensorsSelector::KisMultiSensorsSelector(QWidget* parent) : d(new Private)
{
    d->currentConfigWidget = 0;
    d->form.setupUi(this);
    d->model = new KisMultiSensorsModel(this);
    connect(d->model, SIGNAL(sensorChanged(KisDynamicSensor*)), SIGNAL(sensorChanged(KisDynamicSensor*)));
    connect(d->model, SIGNAL(parametersChanged()), SIGNAL(parametersChanged()));
    connect(d->form.sensorsList, SIGNAL(activated(QModelIndex)), SLOT(sensorActivated(QModelIndex)));
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
}

KisDynamicSensor* KisMultiSensorsSelector::current()
{
    return 0;
}

void KisMultiSensorsSelector::sensorActivated(const QModelIndex& index)
{
    delete d->currentConfigWidget;
    d->currentConfigWidget = d->model->createConfigurationWidget(index, d->form.widgetConfiguration, this);
    if(d->currentConfigWidget)
    {
        d->layout->addWidget(d->currentConfigWidget);
    }
}
