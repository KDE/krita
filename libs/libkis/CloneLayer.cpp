/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "CloneLayer.h"
#include <kis_clone_layer.h>
#include <kis_image.h>
#include <kis_layer.h>

CloneLayer::CloneLayer(KisImageSP image, QString name, KisLayerSP source, QObject *parent) :
    Node(image, new KisCloneLayer(source, image, name, OPACITY_OPAQUE_U8), parent)
{

}

CloneLayer::CloneLayer(KisCloneLayerSP layer, QObject *parent):
    Node(layer->image(), layer, parent)
{

}

CloneLayer::~CloneLayer()
{

}

QString CloneLayer::type() const
{
    return "clonelayer";
}
