/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __EMPTY_NODES_TEST_H
#define __EMPTY_NODES_TEST_H

#include "kis_types.h"

#include "KoColorSpace.h"

#include "kis_image.h"
#include "kis_paint_layer.h"
#include "kis_clone_layer.h"
#include "kis_group_layer.h"
#include "kis_transparency_mask.h"


namespace TestUtil
{

class EmptyNodesTest
{
protected:
    void initBase() {
        m_image = new KisImage(0, 512, 512, 0, "test");
        m_layer1 = new KisPaintLayer(m_image, "layer1", OPACITY_OPAQUE_U8);
        m_layer2 = new KisGroupLayer(m_image, "layer2", OPACITY_OPAQUE_U8);
        m_layer3 = new KisCloneLayer(m_layer1, m_image, "layer3", OPACITY_OPAQUE_U8);
        m_layer4 = new KisGroupLayer(m_image, "layer4", OPACITY_OPAQUE_U8);
        m_mask1 = new KisTransparencyMask();
    }

    void cleanupBase() {
        m_layer1 = 0;
        m_layer2 = 0;
        m_layer3 = 0;
        m_layer4 = 0;
        m_mask1 = 0;
        m_image = 0;
    }

    void constructImage() {
        m_image->addNode(m_layer1);
        m_image->addNode(m_layer2);
        m_image->addNode(m_layer3);
        m_image->addNode(m_layer4);
        m_image->addNode(m_mask1, m_layer3);
    }

protected:
    KisImageSP m_image;
    KisLayerSP m_layer1;
    KisLayerSP m_layer2;
    KisLayerSP m_layer3;
    KisLayerSP m_layer4;
    KisMaskSP m_mask1;
};

}

#endif /* __EMPTY_NODES_TEST_H */
