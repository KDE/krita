/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _UTIL_H_
#define _UTIL_H_

#include <simpletest.h>
#include <QBitArray>

#include <KisDocument.h>
#include <KoDocumentInfo.h>
#include <KoColorSpaceRegistry.h>
#include <KoShapeContainer.h>
#include <KoColorSpace.h>
#include <KoPathShape.h>

#include <kis_count_visitor.h>
#include "kis_types.h"
#include "filter/kis_filter_registry.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter.h"
#include "KisPart.h"
#include "kis_image.h"
#include "kis_pixel_selection.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_clone_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_shape_layer.h"
#include "kis_filter_mask.h"
#include "kis_transparency_mask.h"
#include "kis_selection_mask.h"
#include "kis_selection.h"
#include "kis_fill_painter.h"
#include "kis_shape_selection.h"
#include "kis_default_bounds.h"
#include "kis_transform_mask_params_interface.h"
#include <KisGlobalResourcesInterface.h>


KisSelectionSP createPixelSelection(KisPaintDeviceSP paintDevice)
{
    KisSelectionSP pixelSelection = new KisSelection(new KisSelectionDefaultBounds(paintDevice));

    KisFillPainter gc(pixelSelection->pixelSelection());
    gc.fillRect(10, 10, 200, 200, KoColor(gc.device()->colorSpace()));
    gc.fillRect(150, 150, 200, 200, KoColor(QColor(100, 100, 100, 100), gc.device()->colorSpace()));
    gc.end();

    return pixelSelection;
}

KisSelectionSP createVectorSelection(KisPaintDeviceSP paintDevice, KisImageSP image, KoShapeControllerBase *shapeController)
{
    KisSelectionSP selection = new KisSelection(new KisSelectionDefaultBounds(paintDevice));
    KoPathShape* path = new KoPathShape();
    path->setShapeId(KoPathShapeId);
    path->moveTo(QPointF(10, 10));
    path->lineTo(QPointF(10, 10) + QPointF(100, 0));
    path->lineTo(QPointF(100, 100));
    path->lineTo(QPointF(10, 10) + QPointF(0, 100));
    path->close();
    path->normalize();
    KisShapeSelection* shapeSelection = new KisShapeSelection(shapeController, image, selection);
    shapeSelection->addShape(path);
    selection->convertToVectorSelectionNoUndo(shapeSelection);

    return selection;
}

QTransform createTestingTransform() {
    return QTransform(1,2,3,4,5,6,7,8,9);
}

KisDocument* createCompleteDocument()
{
    KisImageSP image = new KisImage(0, 1024, 1024, KoColorSpaceRegistry::instance()->rgb8(), "test for roundtrip");

    KisDocument *doc = qobject_cast<KisDocument*>(KisPart::instance()->createDocument());

    doc->setCurrentImage(image);
    doc->documentInfo()->setAboutInfo("title", image->objectName());

    KisGroupLayerSP group1 = new KisGroupLayer(image, "group1", 50);

    KisGroupLayerSP group2 = new KisGroupLayer(image, "group2", 100);

    KisPaintLayerSP paintLayer1 = new KisPaintLayer(image, "paintlayer1", OPACITY_OPAQUE_U8);
    paintLayer1->setUserLocked(true);
    QBitArray channelFlags(4);
    channelFlags[0] = true;
    channelFlags[2] = true;
    paintLayer1->setChannelFlags(channelFlags);

    {
        KisFillPainter gc(paintLayer1->paintDevice());
        gc.fillRect(10, 10, 200, 200, KoColor(Qt::red, paintLayer1->paintDevice()->colorSpace()));
        gc.end();
    }

    KisPaintLayerSP paintLayer2 = new KisPaintLayer(image, "paintlayer2", OPACITY_TRANSPARENT_U8, KoColorSpaceRegistry::instance()->lab16());
    paintLayer2->setVisible(false);
    {
        KisFillPainter gc(paintLayer2->paintDevice());
        gc.fillRect(0, 0, 900, 1024, KoColor(QColor(10, 20, 30), paintLayer2->paintDevice()->colorSpace()));
        gc.end();
    }


    KisCloneLayerSP cloneLayer1 = new KisCloneLayer(group1, image, "clonelayer1", 150);
    cloneLayer1->setX(100);
    cloneLayer1->setY(100);

    KisSelectionSP pixelSelection = createPixelSelection(paintLayer1->paintDevice());
    KisFilterConfigurationSP kfc = KisFilterRegistry::instance()->get("pixelize")->defaultConfiguration(KisGlobalResourcesInterface::instance());
    Q_ASSERT(kfc);
    KisAdjustmentLayerSP adjustmentLayer1 = new KisAdjustmentLayer(image, "adjustmentLayer1", kfc->cloneWithResourcesSnapshot(), pixelSelection);

    KisSelectionSP vectorSelection = createVectorSelection(paintLayer2->paintDevice(), image, doc->shapeController());
    KisAdjustmentLayerSP adjustmentLayer2 = new KisAdjustmentLayer(image, "adjustmentLayer2", kfc->cloneWithResourcesSnapshot(), vectorSelection);

    image->addNode(paintLayer1);
    image->addNode(group1);
    image->addNode(paintLayer2, group1);
    image->addNode(group2);
    image->addNode(cloneLayer1, group2);
    image->addNode(adjustmentLayer1, group2);

//    KoShapeContainer * parentContainer =
//        dynamic_cast<KoShapeContainer*>(doc->shapeForNode(group1));

    KoPathShape* path = new KoPathShape();
    path->setShapeId(KoPathShapeId);
    path->moveTo(QPointF(10, 10));
    path->lineTo(QPointF(10, 10) + QPointF(100, 0));
    path->lineTo(QPointF(100, 100));
    path->lineTo(QPointF(10, 10) + QPointF(0, 100));
    path->close();
    path->normalize();
    KisShapeLayerSP shapeLayer = new KisShapeLayer(doc->shapeController(), image, "shapeLayer1", 75);
    shapeLayer->addShape(path);
    image->addNode(shapeLayer, group1);
    image->addNode(adjustmentLayer2, group1);

    KisFilterMaskSP filterMask1 = new KisFilterMask(image, "filterMask1");

    kfc = KisFilterRegistry::instance()->get("pixelize")->defaultConfiguration(KisGlobalResourcesInterface::instance());
    filterMask1->setFilter(kfc->cloneWithResourcesSnapshot());
    kfc = 0; // kfc cannot be shared!

    filterMask1->setSelection(createPixelSelection(paintLayer1->paintDevice()));
    image->addNode(filterMask1, paintLayer1);

    KisFilterMaskSP filterMask2 = new KisFilterMask(image, "filterMask2");

    kfc = KisFilterRegistry::instance()->get("pixelize")->defaultConfiguration(KisGlobalResourcesInterface::instance());
    filterMask2->setFilter(kfc->cloneWithResourcesSnapshot());
    kfc = 0; // kfc cannot be shared!

    filterMask2->setSelection(createVectorSelection(paintLayer2->paintDevice(), image, doc->shapeController()));
    image->addNode(filterMask2, paintLayer2);

    KisTransparencyMaskSP transparencyMask1 = new KisTransparencyMask(image, "transparencyMask1");
    transparencyMask1->setSelection(createPixelSelection(paintLayer1->paintDevice()));
    image->addNode(transparencyMask1, group1);

    KisTransparencyMaskSP transparencyMask2 = new KisTransparencyMask(image, "transparencyMask2");
    transparencyMask2->setSelection(createPixelSelection(paintLayer1->paintDevice()));
    image->addNode(transparencyMask2, group2);

    KisSelectionMaskSP selectionMask1 = new KisSelectionMask(image, "selectionMask1");
    image->addNode(selectionMask1, paintLayer1);
    selectionMask1->setSelection(createPixelSelection(paintLayer1->paintDevice()));

    KisSelectionMaskSP selectionMask2 = new KisSelectionMask(image, "selectionMask2");
    selectionMask2->setSelection(createPixelSelection(paintLayer2->paintDevice()));
    image->addNode(selectionMask2, paintLayer2);

    KisTransformMaskSP transformMask = new KisTransformMask(image, "testTransformMask");
    transformMask->setTransformParams(KisTransformMaskParamsInterfaceSP(
                                          new KisDumbTransformMaskParams(createTestingTransform())));

    image->addNode(transformMask, paintLayer2);

    return doc;
}

KisDocument *createEmptyDocument()
{
    KisImageSP image = new KisImage(0, 1024, 1024, KoColorSpaceRegistry::instance()->rgb8(), "test for roundtrip");

    KisDocument *doc = qobject_cast<KisDocument*>(KisPart::instance()->createDocument());

    doc->setCurrentImage(image);
    doc->documentInfo()->setAboutInfo("title", image->objectName());

    return doc;
}


#endif
