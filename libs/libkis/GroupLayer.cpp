/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "GroupLayer.h"
#include <kis_group_layer.h>
#include <kis_image.h>

GroupLayer::GroupLayer(KisImageSP image, QString name, QObject *parent) :
    Node(image, new KisGroupLayer(image, name, OPACITY_OPAQUE_U8), parent)
{

}

GroupLayer::GroupLayer(KisGroupLayerSP layer, QObject *parent):
    Node(layer->image(), layer, parent)
{

}

GroupLayer::~GroupLayer()
{

}

void GroupLayer::setPassThroughMode(bool passthrough)
{
    KisGroupLayer *group = dynamic_cast<KisGroupLayer*>(this->node().data());
    group->setPassThroughMode(passthrough);
}

bool GroupLayer::passThroughMode() const
{
    const KisGroupLayer *group = qobject_cast<const KisGroupLayer*>(this->node());
    return group->passThroughMode();
}

QString GroupLayer::type() const
{
    return "grouplayer";
}
