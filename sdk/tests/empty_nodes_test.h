/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __EMPTY_NODES_TEST_H
#define __EMPTY_NODES_TEST_H

#include "kis_types.h"

#include "KoColorSpace.h"
#include "KoColor.h"

#include "kis_image.h"
#include "kis_painter.h"
#include "kis_paint_layer.h"
#include "kis_clone_layer.h"
#include "kis_group_layer.h"
#include "kis_transparency_mask.h"
#include "kis_selection_mask.h"
#include "kis_random_source.h"
#include "kis_undo_stores.h"

namespace TestUtil
{

class EmptyNodesTest
{
protected:
    void initPaintDevice(KisNodeSP node, int seed) {
        Q_ASSERT(node->paintDevice());
        KisRandomSource source(seed);
        QColor color(source.generate(10,255), source.generate(10,255), source.generate(10, 255));
        QRect rc(source.generate(128,255),
                 source.generate(128,255),
                 source.generate(128,255),
                 source.generate(128,255));

        KisPaintDeviceSP dev = node->paintDevice()->createCompositionSourceDevice();
        dev->fill(rc, KoColor(color, dev->colorSpace()));

        KisPainter gc(node->paintDevice());
        gc.bitBlt(rc.topLeft(), dev, rc);
    }


    void initBase() {
        m_image = new KisImage(new KisSurrogateUndoStore(), 512, 512, KoColorSpaceRegistry::instance()->rgb8(), "test");
        m_layer1 = new KisPaintLayer(m_image, "layer1", OPACITY_OPAQUE_U8);
        m_layer2 = new KisGroupLayer(m_image, "layer2", OPACITY_OPAQUE_U8);
        m_layer3 = new KisCloneLayer(m_layer1, m_image, "layer3", OPACITY_OPAQUE_U8);
        m_layer4 = new KisGroupLayer(m_image, "layer4", OPACITY_OPAQUE_U8);
        m_mask1 = new KisTransparencyMask(m_image, "mask1");
        m_sel1 = new KisSelectionMask(m_image, "sel1");
        m_sel2 = new KisSelectionMask(m_image, "sel2");
        m_sel3 = new KisSelectionMask(m_image, "sel3");
    }

    void cleanupBase() {
        m_layer1 = 0;
        m_layer2 = 0;
        m_layer3 = 0;
        m_layer4 = 0;
        m_mask1 = 0;
        m_sel1 = 0;
        m_sel2 = 0;
        m_sel3 = 0;
        m_image = 0;
    }

    /**
     * root
     *   sel1
     *   layer1
     *   layer2
     *   sel2
     *   layer3
     *     mask1
     *     sel3
     *   layer4
     */

    void constructImage() {
        m_image->addNode(m_layer1);
        m_image->addNode(m_layer2);
        m_image->addNode(m_layer3);
        m_image->addNode(m_layer4);
        m_image->addNode(m_mask1, m_layer3);
        m_mask1->initSelection(m_layer3);
        initPaintDevice(m_layer1, 10);
        initPaintDevice(m_mask1, 500);
        m_image->initialRefreshGraph();
    }

    void addSelectionMasks() {
        m_image->addNode(m_sel1, m_image->root(), (int)0);
        m_image->addNode(m_sel2, m_image->root(), (int)2);
        m_image->addNode(m_sel3, m_layer3);
        m_sel1->initSelection(m_layer3);
        m_sel2->initSelection(m_layer3);
        m_sel3->initSelection(m_layer3);
        initPaintDevice(m_sel1, 100);
        initPaintDevice(m_sel2, 200);
        const quint8 defPixel(quint8(750));
        m_sel3->paintDevice()->setDefaultPixel(KoColor(&defPixel, m_sel3->paintDevice()->colorSpace()));
    }

protected:
    KisImageSP m_image;
    KisLayerSP m_layer1;
    KisLayerSP m_layer2;
    KisLayerSP m_layer3;
    KisLayerSP m_layer4;
    KisMaskSP m_mask1;
    KisMaskSP m_sel1;
    KisMaskSP m_sel2;
    KisMaskSP m_sel3;
};

}

#endif /* __EMPTY_NODES_TEST_H */
