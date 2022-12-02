/*
 *  SPDX-FileCopyrightText: 2011 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisMultiSensorsSelector2.h"

#include "KisCurveOptionData.h"
#include "kis_dynamic_sensor.h"
#include "ui_wdgmultisensorsselector.h"
#include "KisMultiSensorsModel2.h"
#include <KisDynamicSensorFactoryRegistry.h>


struct KisMultiSensorsSelector2::Private {
    lager::cursor<KisCurveOptionDataCommon> optionData;

    Ui_WdgMultiSensorsSelector form;
    KisMultiSensorsModel2* model;
    QWidget* currentConfigWidget;
    QHBoxLayout* layout;
};

auto sensorsLens = lager::lenses::getset(
    [](const KisCurveOptionDataCommon &data) -> KisMultiSensorsModel2::MultiSensorData {
        std::vector<const KisSensorData*> srcSensors = data.sensors();

        KisMultiSensorsModel2::MultiSensorData sensors;
        sensors.reserve(srcSensors.size());

        Q_FOREACH(const KisSensorData* srcSensor, srcSensors) {
            sensors.emplace_back(srcSensor->id, srcSensor->isActive);
        }
        return sensors;
    },
    [](KisCurveOptionDataCommon data, KisMultiSensorsModel2::MultiSensorData sensors) -> KisCurveOptionDataCommon {
        std::vector<KisSensorData*> parentSensors = data.sensors();

        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(parentSensors.size() == sensors.size(), data);

        auto parentIt = parentSensors.begin();
        auto it = sensors.begin();
        for (; parentIt != parentSensors.end(); ++parentIt, ++it) {

            KIS_SAFE_ASSERT_RECOVER((*parentIt)->id == it->first) {
                continue;
            }
            (*parentIt)->isActive = it->second;
        }

        return data;
    });



KisMultiSensorsSelector2::KisMultiSensorsSelector2(QWidget* parent)
    : QWidget(parent)
    , d(new Private)
{
    d->currentConfigWidget = 0;
    d->form.setupUi(this);

    connect(d->form.sensorsList, SIGNAL(activated(QModelIndex)), SLOT(setCurrent(QModelIndex)));
    connect(d->form.sensorsList, SIGNAL(clicked(QModelIndex)), SLOT(setCurrent(QModelIndex)));

    d->layout = new QHBoxLayout(d->form.widgetConfiguration);

    // allow the list viewport to be responsive to input release events
    d->form.sensorsList->viewport()->installEventFilter(this);
}

KisMultiSensorsSelector2::~KisMultiSensorsSelector2()
{
    delete d;
}

void KisMultiSensorsSelector2::setOptionDataCursor(lager::cursor<KisCurveOptionDataCommon> optionData)
{
    d->optionData = optionData;
    d->model = new KisMultiSensorsModel2(optionData.zoom(sensorsLens), this);
    d->form.sensorsList->setModel(d->model);

    // TODO: verify that at least one sensor is always active!
}

void KisMultiSensorsSelector2::setCurrent(const QString &id)
{
    const QModelIndex index = d->model->sensorIndex(id);
    KIS_SAFE_ASSERT_RECOVER_RETURN(index.isValid());

    d->form.sensorsList->setCurrentIndex(index);

    sensorActivated(index);
    Q_EMIT highlightedSensorChanged(id);
}

void KisMultiSensorsSelector2::setCurrent(const QModelIndex& index)
{
    setCurrent(d->model->getSensorId(index));
}

QString KisMultiSensorsSelector2::currentHighlighted()
{
    return d->model->getSensorId(d->form.sensorsList->currentIndex());
}

void KisMultiSensorsSelector2::sensorActivated(const QModelIndex& index)
{
    delete d->currentConfigWidget;

    KisDynamicSensorFactory *factory =
        KisDynamicSensorFactoryRegistry::instance()->get(d->model->getSensorId(index));
    KIS_SAFE_ASSERT_RECOVER_RETURN(factory);

    d->currentConfigWidget = factory->createConfigWidget(d->optionData, d->form.widgetConfiguration);
    if (d->currentConfigWidget) {
        d->layout->addWidget(d->currentConfigWidget);
    }
}

bool KisMultiSensorsSelector2::eventFilter(QObject *obj, QEvent *event)
{
    // TODO: check what this function does?

    if (event->type() == (QEvent::MouseButtonRelease) || event->type() == QEvent::TabletRelease) {
        QModelIndex index = d->form.sensorsList->currentIndex();
        setCurrent(index);
        event->accept();
    }

    return QObject::eventFilter(obj, event);
}
