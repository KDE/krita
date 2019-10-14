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

#ifndef __QIMAGE_BASED_TEST_H
#define __QIMAGE_BASED_TEST_H

#ifndef USE_DOCUMENT
#define USE_DOCUMENT 1
#endif /* USE_DOCUMENT */

#include "testutil.h"


#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <KoShapeContainer.h>
#include <KoShapeRegistry.h>
#include <KoShapeFactoryBase.h>

#if USE_DOCUMENT
#include "KisDocument.h"
#include "kis_shape_layer.h"
#else
#include "kis_filter_configuration.h"
#endif /* USE_DOCUMENT */

#include "kis_undo_stores.h"
#include "kis_image.h"
#include "kis_selection.h"
#include "kis_paint_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_transparency_mask.h"
#include "kis_clone_layer.h"

#include "filter/kis_filter.h"
#include "filter/kis_filter_registry.h"

#include "commands/kis_selection_commands.h"


namespace TestUtil
{

class QImageBasedTest
{
public:
    QImageBasedTest(const QString &directoryName)
        : m_directoryName(directoryName)
    {
    }

    // you need to declare your own test function
    // See KisProcessingTest for example

protected:

    /**
     * Creates a complex image connected to a surrogate undo store
     */
    KisImageSP createImage(KisSurrogateUndoStore *undoStore) {
        QImage sourceImage(fetchDataFileLazy("hakonepa.png"));

        QRect imageRect = QRect(QPoint(0,0), sourceImage.size());

        QRect transpRect(50,50,300,300);
        QRect blurRect(66,66,300,300);
        QPoint blurShift(34,34);
        QPoint cloneShift(75,75);

        const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
        KisImageSP image = new KisImage(undoStore, imageRect.width(), imageRect.height(), cs, "merge test");

        KisFilterSP filter = KisFilterRegistry::instance()->value("blur");
        Q_ASSERT(filter);
        KisFilterConfigurationSP configuration = filter->defaultConfiguration();
        Q_ASSERT(configuration);

        KisAdjustmentLayerSP blur1 = new KisAdjustmentLayer(image, "blur1", configuration, 0);
        blur1->internalSelection()->clear();
        blur1->internalSelection()->pixelSelection()->select(blurRect);
        blur1->setX(blurShift.x());
        blur1->setY(blurShift.y());

        KisPaintLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
        paintLayer1->paintDevice()->convertFromQImage(sourceImage, 0, 0, 0);

        KisCloneLayerSP cloneLayer1 =
            new KisCloneLayer(paintLayer1, image, "clone1", OPACITY_OPAQUE_U8);
        cloneLayer1->setX(cloneShift.x());
        cloneLayer1->setY(cloneShift.y());

        image->addNode(cloneLayer1);
        image->addNode(blur1);
        image->addNode(paintLayer1);

        KisTransparencyMaskSP transparencyMask1 = new KisTransparencyMask();
        transparencyMask1->setName("tmask1");
        transparencyMask1->testingInitSelection(transpRect, paintLayer1);

        image->addNode(transparencyMask1, paintLayer1);

        return image;
    }

    /**
     * Creates a simple image with one empty layer and connects it to
     * a surrogate undo store
     */
    KisImageSP createTrivialImage(KisSurrogateUndoStore *undoStore) {
        QRect imageRect = QRect(0, 0, 640, 441);

        const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
        KisImageSP image = new KisImage(undoStore, imageRect.width(), imageRect.height(), cs, "merge test");

        KisPaintLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
        image->addNode(paintLayer1);

        return image;
    }

    void addGlobalSelection(KisImageSP image) {
        QRect selectionRect(40,40,300,300);

        KisSelectionSP selection = new KisSelection(new KisDefaultBounds(image));
        KisPixelSelectionSP pixelSelection = selection->pixelSelection();
        pixelSelection->select(selectionRect);

        KUndo2Command *cmd = new KisSetGlobalSelectionCommand(image, selection);
        image->undoAdapter()->addCommand(cmd);
    }

#if USE_DOCUMENT

    void addShapeLayer(KisDocument *doc, KisImageSP image) {

        KisShapeLayerSP shapeLayer = new KisShapeLayer(doc->shapeController(), image.data(), "shape", OPACITY_OPAQUE_U8);
        image->addNode(shapeLayer);

        KoShapeFactoryBase *f1 = KoShapeRegistry::instance()->get("StarShape");
        KoShapeFactoryBase *f2 = KoShapeRegistry::instance()->get("RectangleShape");

        KoShape *shape1 = f1->createDefaultShape();
        KoShape *shape2 = f2->createDefaultShape();

        shape1->setPosition(QPointF(100,100));
        shape2->setPosition(QPointF(200,200));

        shapeLayer->addShape(shape1);
        shapeLayer->addShape(shape2);

        QApplication::processEvents();
    }

#endif /* USE_DOCUMENT*/

    bool checkLayersInitial(KisImageWSP image, int baseFuzzyness = 0) {
        QString prefix = "initial_with_selection";
        QString prefix2 = findNode(image->root(), "shape") ? "_with_shape" : "";
        return checkLayers(image, prefix + prefix2, baseFuzzyness);
    }

    bool checkLayersInitialRootOnly(KisImageWSP image, int baseFuzzyness = 0) {
        QString prefix = "initial_with_selection";
        QString prefix2 = findNode(image->root(), "shape") ? "_with_shape" : "";
        return checkLayers(image, prefix + prefix2, baseFuzzyness, false);
    }

    /**
     * Checks the content of image's layers against the set of
     * QImages stored in @p prefix subfolder
     */
    bool checkLayers(KisImageWSP image, const QString &prefix, int baseFuzzyness = 0, bool recursive = true) {
        QVector<QImage> images;
        QVector<QString> names;

        fillNamesImages(image->root(), image->bounds(), images, names, recursive);

        bool valid = true;

        const int stackSize = images.size();
        for(int i = 0; i < stackSize; i++) {
            if(!checkOneQImage(images[i], prefix, names[i], baseFuzzyness)) {
                valid = false;
            }
        }

        return valid;
    }

    /**
     * Checks the content of one image's layer against the QImage
     * stored in @p prefix subfolder
     */
    bool checkOneLayer(KisImageWSP image, KisNodeSP node,  const QString &prefix, int baseFuzzyness = 0) {
        QVector<QImage> images;
        QVector<QString> names;

        fillNamesImages(node, image->bounds(), images, names);

        return checkOneQImage(images.first(), prefix, names.first(), baseFuzzyness);
    }

    // add default bounds param
    bool checkOneDevice(KisPaintDeviceSP device,
                        const QString &prefix,
                        const QString &name,
                        int baseFuzzyness = 0)
    {
        QImage image = device->convertToQImage(0);
        return checkOneQImage(image, prefix, name, baseFuzzyness);
    }

    KisNodeSP findNode(KisNodeSP root, const QString &name) {
        return TestUtil::findNode(root, name);
    }

private:
    bool checkOneQImage(const QImage &image,
                        const QString &prefix,
                        const QString &name,
                        int baseFuzzyness)
    {
        QString realName = prefix + "_" + name + ".png";
        QString expectedName = prefix + "_" + name + "_expected.png";

        bool valid = true;

        QString fullPath = fetchDataFileLazy(m_directoryName + QDir::separator() +
                                             prefix + QDir::separator() + realName);

        if (fullPath.isEmpty()) {
            // Try without the testname subdirectory
            fullPath = fetchDataFileLazy(prefix + QDir::separator() +
                                         realName);
        }

        if (fullPath.isEmpty()) {
            // Try without the prefix subdirectory
            fullPath = fetchDataFileLazy(m_directoryName + QDir::separator() +
                                         realName);
        }

        QImage ref(fullPath);

        QPoint temp;
        int fuzzy = baseFuzzyness;

        {
            QStringList terms = name.split('_');
            if(terms[0] == "root" ||
               terms[0] == "blur1" ||
               terms[0] == "shape") {

                fuzzy++;
            }
        }

        if(ref != image &&
           !TestUtil::compareQImages(temp, ref, image, fuzzy, fuzzy)) {


            dbgKrita << "--- Wrong image:" << realName;
            valid = false;

            image.save(QString(FILES_OUTPUT_DIR) + QDir::separator() + realName);
            ref.save(QString(FILES_OUTPUT_DIR) + QDir::separator() + expectedName);
        }

        return valid;
    }

    void fillNamesImages(KisNodeSP node, const QRect &rc,
                         QVector<QImage> &images,
                         QVector<QString> &names,
                         bool recursive = true) {

        while (node) {
            if(node->paintDevice()) {
                names.append(node->name() + "_paintDevice");
                images.append(node->paintDevice()->
                              convertToQImage(0, rc.x(), rc.y(),
                                              rc.width(), rc.height()));
            }

            if(node->original() && node->original() != node->paintDevice()) {
                names.append(node->name() + "_original");
                images.append(node->original()->
                              convertToQImage(0, rc.x(), rc.y(),
                                              rc.width(), rc.height()));
            }

            if(node->projection() && node->projection() != node->paintDevice()) {
                names.append(node->name() + "_projection");
                images.append(node->projection()->
                              convertToQImage(0, rc.x(), rc.y(),
                                              rc.width(), rc.height()));
            }

            if (recursive) {
                fillNamesImages(node->firstChild(), rc, images, names);
            }
            node = node->nextSibling();
        }
    }

private:
    QString m_directoryName;

};

}

#endif /* __QIMAGE_BASED_TEST_H */
