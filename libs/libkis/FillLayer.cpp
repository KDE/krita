/*
 *  Copyright (c) 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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
#include "FillLayer.h"
#include <kis_generator_layer.h>
#include <kis_image.h>
#include <kis_filter_configuration.h>
#include <kis_generator_registry.h>
#include <InfoObject.h>
#include <kis_selection.h>

FillLayer::FillLayer(KisImageSP image, QString name, KisFilterConfigurationSP filterConfig, Selection &selection, QObject *parent) :
    Node(image, new KisGeneratorLayer(image, name, filterConfig, selection.selection()), parent)
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
    //getting the default configuration here avoids trouble with versioning.
    KisGeneratorSP generator = KisGeneratorRegistry::instance()->value(generatorName);
    if (generator) {
        KisFilterConfigurationSP cfg = generator->factoryConfiguration();
        Q_FOREACH(const QString property, config->properties().keys()) {
            cfg->setProperty(property, config->property(property));
        }
        layer->setFilter(cfg);
        return true;
    }
    return false;
}
