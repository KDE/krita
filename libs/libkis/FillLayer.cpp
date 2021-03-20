/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "FillLayer.h"
#include <kis_generator_layer.h>
#include <kis_image.h>
#include <kis_filter_configuration.h>
#include <kis_generator_registry.h>
#include <InfoObject.h>
#include <kis_selection.h>
#include <KisGlobalResourcesInterface.h>
#include <kis_assert.h>

FillLayer::FillLayer(KisImageSP image, QString name, KisFilterConfigurationSP filterConfig, Selection &selection, QObject *parent) :
    Node(image, new KisGeneratorLayer(image, name, filterConfig->cloneWithResourcesSnapshot(), selection.selection()), parent)
{

}

FillLayer::FillLayer(KisGeneratorLayerSP layer, QObject *parent):
    Node(layer->image(), layer, parent)
{

}

FillLayer::~FillLayer()
{

}

QString FillLayer::generatorName()
{
    const KisGeneratorLayer *layer = qobject_cast<const KisGeneratorLayer*>(this->node());
    return layer->filter()->name();
}

InfoObject * FillLayer::filterConfig()
{
    const KisGeneratorLayer *layer = qobject_cast<const KisGeneratorLayer*>(this->node());
    return new InfoObject(layer->filter());
}

QString FillLayer::type() const
{
    return "filllayer";
}

bool FillLayer::setGenerator(const QString &generatorName, InfoObject *config)
{
    KisGeneratorLayer *layer = dynamic_cast<KisGeneratorLayer*>(this->node().data());
    KIS_ASSERT_RECOVER_RETURN_VALUE(layer, false);

    //getting the default configuration here avoids trouble with versioning.
    KisGeneratorSP generator = KisGeneratorRegistry::instance()->value(generatorName);
    if (generator) {
        KisFilterConfigurationSP cfg = generator->factoryConfiguration(KisGlobalResourcesInterface::instance());
        Q_FOREACH(const QString property, config->properties().keys()) {
            cfg->setProperty(property, config->property(property));
        }
        layer->setFilter(cfg->cloneWithResourcesSnapshot());

        if (layer->hasPendingTimedUpdates()) {
            layer->forceUpdateTimedNode();
        }

        image()->waitForDone();
        return true;
    }
    return false;
}
