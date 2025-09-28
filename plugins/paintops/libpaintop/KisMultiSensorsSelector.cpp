/*
 *  SPDX-FileCopyrightText: 2011 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisMultiSensorsSelector.h"

#include "KisCurveOptionData.h"
#include "ui_wdgmultisensorsselector.h"
#include "KisMultiSensorsModel.h"
#include <KisDynamicSensorFactoryRegistry.h>


struct KisMultiSensorsSelector::Private {
    lager::cursor<KisCurveOptionDataCommon> optionData;

    Ui_WdgMultiSensorsSelector form;
    KisMultiSensorsModel* model;
    QWidget* currentConfigWidget;
    QHBoxLayout* layout;
};

auto sensorsLens = lager::lenses::getset(
    [](const KisCurveOptionDataCommon &data) -> KisMultiSensorsModel::MultiSensorData {
        std::vector<const KisSensorData*> srcSensors = data.sensors();

        KisMultiSensorsModel::MultiSensorData sensors;
        sensors.reserve(srcSensors.size());

        Q_FOREACH(const KisSensorData* srcSensor, srcSensors) {
            sensors.emplace_back(srcSensor->id, srcSensor->isActive);
        }
        return sensors;
    },
    [](KisCurveOptionDataCommon data, KisMultiSensorsModel::MultiSensorData sensors) -> KisCurveOptionDataCommon {
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



KisMultiSensorsSelector::KisMultiSensorsSelector(QWidget* parent)
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

KisMultiSensorsSelector::~KisMultiSensorsSelector()
{
    delete d;
}

void KisMultiSensorsSelector::setOptionDataCursor(lager::cursor<KisCurveOptionDataCommon> optionData)
{
    d->optionData = optionData;
    d->model = new KisMultiSensorsModel(optionData.zoom(sensorsLens), this);
    d->form.sensorsList->setModel(d->model);

    // TODO: verify that at least one sensor is always active!
}

void KisMultiSensorsSelector::setCurrent(const QString &id)
{
    const QModelIndex index = d->model->sensorIndex(id);
    KIS_SAFE_ASSERT_RECOVER_RETURN(index.isValid());

    d->form.sensorsList->setCurrentIndex(index);

    sensorActivated(index);
    Q_EMIT highlightedSensorChanged(id);
}

void KisMultiSensorsSelector::setCurrent(const QModelIndex& index)
{
    setCurrent(d->model->getSensorId(index));
}

QString KisMultiSensorsSelector::currentHighlighted()
{
    return d->model->getSensorId(d->form.sensorsList->currentIndex());
}

void KisMultiSensorsSelector::sensorActivated(const QModelIndex& index)
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

bool KisMultiSensorsSelector::eventFilter(QObject *obj, QEvent *event)
{
    /**
     * This filter makes it possible to select a sensor by click+drag gesture.
     * Such gesture is very easy to do with a tablet stylus. Basically, we
     * activate the lastly-dragged-over sensor on mause/tablet-release
     */

    if (event->type() == (QEvent::MouseButtonRelease) || event->type() == QEvent::TabletRelease) {
        QModelIndex index = d->form.sensorsList->currentIndex();
        setCurrent(index);
        event->accept();
    }

    return QObject::eventFilter(obj, event);
}
