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
