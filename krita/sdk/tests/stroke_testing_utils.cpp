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

#include "stroke_testing_utils.h"

#include <QtTest>

#include <QDir>
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include "kis_painter.h"
#include "kis_paintop_preset.h"
#include "kis_pattern.h"
#include "kis_canvas_resource_provider.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_paint_layer.h"

#include "testutil.h"


KisImageSP utils::createImage(KisUndoStore *undoStore, const QSize &imageSize) {
    QRect imageRect(0,0,imageSize.width(),imageSize.height());

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(undoStore, imageRect.width(), imageRect.height(), cs, "stroke test");

    KisPaintLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
    KisPaintLayerSP paintLayer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE_U8);
    KisPaintLayerSP paintLayer3 = new KisPaintLayer(image, "paint3", OPACITY_OPAQUE_U8);
    KisPaintLayerSP paintLayer4 = new KisPaintLayer(image, "paint4", OPACITY_OPAQUE_U8);
    KisPaintLayerSP paintLayer5 = new KisPaintLayer(image, "paint5", OPACITY_OPAQUE_U8);

    image->lock();
    image->addNode(paintLayer1);
    image->addNode(paintLayer2);
    image->addNode(paintLayer3);
    image->addNode(paintLayer4);
    image->addNode(paintLayer5);
    image->unlock();
    return image;
}

KoCanvasResourceManager* utils::createResourceManager(KisImageWSP image,
                                                KisNodeSP node,
                                                const QString &presetFileName)
{
    KoCanvasResourceManager *manager = new KoCanvasResourceManager();

    QVariant i;

    i.setValue(KoColor(Qt::black, image->colorSpace()));
    manager->setResource(KoCanvasResourceManager::ForegroundColor, i);

    i.setValue(KoColor(Qt::white, image->colorSpace()));
    manager->setResource(KoCanvasResourceManager::BackgroundColor, i);

    i.setValue(static_cast<void*>(0));
    manager->setResource(KisCanvasResourceProvider::CurrentPattern, i);
    manager->setResource(KisCanvasResourceProvider::CurrentGradient, i);
    manager->setResource(KisCanvasResourceProvider::CurrentGeneratorConfiguration, i);

    if(!node) {
        node = image->root();

        while(node && !dynamic_cast<KisPaintLayer*>(node.data())) {
            node = node->firstChild();
        }

        Q_ASSERT(node && dynamic_cast<KisPaintLayer*>(node.data()));
    }

    i.setValue(node);
    manager->setResource(KisCanvasResourceProvider::CurrentKritaNode, i);

    KisPaintOpPresetSP preset;

    if(!presetFileName.isEmpty()) {
        QString dataPath = QString(FILES_DATA_DIR) + QDir::separator();
        preset = new KisPaintOpPreset(dataPath + presetFileName);
        preset->load();
    }

    i.setValue(preset);
    manager->setResource(KisCanvasResourceProvider::CurrentPaintOpPreset, i);

    i.setValue(COMPOSITE_OVER);
    manager->setResource(KisCanvasResourceProvider::CurrentCompositeOp, i);

    i.setValue(false);
    manager->setResource(KisCanvasResourceProvider::MirrorHorizontal, i);

    i.setValue(false);
    manager->setResource(KisCanvasResourceProvider::MirrorVertical, i);

    i.setValue(1.0);
    manager->setResource(KisCanvasResourceProvider::Opacity, i);

    i.setValue(1.0);
    manager->setResource(KisCanvasResourceProvider::HdrExposure, i);

    i.setValue(QPoint());
    manager->setResource(KisCanvasResourceProvider::MirrorAxisCenter, i);

    return manager;
}


utils::StrokeTester::StrokeTester(const QString &name, const QSize &imageSize, const QString &presetFilename)
    : m_name(name),
      m_imageSize(imageSize),
      m_presetFilename(presetFilename)
{
}

utils::StrokeTester::~StrokeTester()
{
}

void utils::StrokeTester::test()
{
    testOneStroke(false, false, false);
    testOneStroke(false, true, false);
    testOneStroke(true, false, false);
    testOneStroke(true, true, false);

    // The same but with updates (compare against projection)

    testOneStroke(false, false, false, true);
    testOneStroke(false, true, false, true);
    testOneStroke(true, false, false, true);
    testOneStroke(true, true, false, true);

    // The same, but with an external layer

    testOneStroke(false, false, true);
    testOneStroke(false, true, true);
    testOneStroke(true, false, true);
    testOneStroke(true, true, true);
}

void utils::StrokeTester::benchmark()
{
    // not cancelled, indirect painting, internal, no updates, no qimage
    doStroke(false, true, false, false, false);
}

void utils::StrokeTester::testOneStroke(bool cancelled,
                                        bool indirectPainting,
                                        bool externalLayer,
                                        bool testUpdates)
{
    QString testName = formatTestName(m_name,
                                      cancelled,
                                      indirectPainting,
                                      externalLayer);

    qDebug() << "Testcase:" << testName
             << "(comare against " << (testUpdates ? "projection" : "layer") << ")";

    QImage resultImage;
    resultImage = doStroke(cancelled, indirectPainting, externalLayer, testUpdates);

    QImage refImage;
    refImage.load(referenceFile(testName));

    QPoint temp;
    if(!TestUtil::compareQImages(temp, refImage, resultImage, 1, 1)) {
        refImage.save(dumpReferenceFile(testName));
        resultImage.save(resultFile(testName));

        QFAIL("Images do not coincide");
    }
}

QString utils::StrokeTester::formatTestName(const QString &baseName,
                                            bool cancelled,
                                            bool indirectPainting,
                                            bool externalLayer)
{
    QString result = baseName;
    result += "_" + m_presetFilename;
    result += indirectPainting ? "_indirect" : "_incremental";
    result += cancelled ? "_cancelled" : "_finished";
    result += externalLayer ? "_external" : "_internal";
    return result;
}

QString utils::StrokeTester::referenceFile(const QString &testName)
{
    QString path =
        QString(FILES_DATA_DIR) + QDir::separator() +
        m_name + QDir::separator();

    path += testName;
    path += ".png";
    return path;
}

QString utils::StrokeTester::dumpReferenceFile(const QString &testName)
{
    QString path = QString(FILES_OUTPUT_DIR) + QDir::separator();
    path += testName;
    path += "_expected";
    path += ".png";
    return path;
}

QString utils::StrokeTester::resultFile(const QString &testName)
{
    QString path = QString(FILES_OUTPUT_DIR) + QDir::separator();
    path += testName;
    path += ".png";
    return path;
}

QImage utils::StrokeTester::doStroke(bool cancelled,
                                     bool indirectPainting,
                                     bool externalLayer,
                                     bool testUpdates,
                                     bool needQImage)
{
    KisImageSP image = utils::createImage(0, m_imageSize);
    KoCanvasResourceManager *manager = utils::createResourceManager(image, 0, m_presetFilename);

    KisPainter *painter = new KisPainter();
    KisResourcesSnapshotSP resources =
        new KisResourcesSnapshot(image,
                                 image->postExecutionUndoAdapter(),
                                 manager);

    if(externalLayer) {
        KisNodeSP externalNode = new KisPaintLayer(0, "extlyr", OPACITY_OPAQUE_U8, image->colorSpace());
        resources->setCurrentNode(externalNode);
        Q_ASSERT(resources->currentNode() == externalNode);
    }

    initImage(image, resources->currentNode());

    KisStrokeStrategy *stroke = createStroke(indirectPainting, resources, painter, image);
    m_strokeId = image->startStroke(stroke);
    addPaintingJobs(image, resources, painter);

    if(!cancelled) {
        image->endStroke(m_strokeId);
    }
    else {
        image->cancelStroke(m_strokeId);
    }

    image->waitForDone();

    QImage resultImage;
    if(needQImage) {
        KisPaintDeviceSP device = testUpdates ?
            image->projection() :
            resources->currentNode()->paintDevice();

        resultImage = device->convertToQImage(0, 0, 0, image->width(), image->height());
    }

    image = 0;
    delete manager;
    return resultImage;
}

void utils::StrokeTester::initImage(KisImageWSP image, KisNodeSP activeNode)
{
    Q_UNUSED(image);
    Q_UNUSED(activeNode);
}
