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
#include "TransparencyMask.h"
#include <kis_transparency_mask.h>
#include <kis_image.h>

TransparencyMask::TransparencyMask(KisImageSP image, QString name, QObject *parent) :
    Node(image, new KisTransparencyMask(), parent)
{
    this->node()->setName(name);
}

TransparencyMask::TransparencyMask(KisImageSP image, KisTransparencyMaskSP mask, QObject *parent):
    Node(image, mask, parent)
{

}

TransparencyMask::~TransparencyMask()
{

}

QString TransparencyMask::type() const
{
    return "transparencymask";
}
