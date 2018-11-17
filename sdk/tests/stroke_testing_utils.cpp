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
#include <KoCompositeOpRegistry.h>
#include <brushengine/kis_paintop_preset.h>
#include <resources/KoPattern.h>
#include "kis_canvas_resource_provider.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include <KisViewManager.h>

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

KoCanvasResourceProvider* utils::createResourceManager(KisImageWSP image,
                                                KisNodeSP node,
                                                const QString &presetFileName)
{
    KoCanvasResourceProvider *manager = new KoCanvasResourceProvider();
    KisViewManager::initializeResourceManager(manager);

    QVariant i;

    i.setValue(KoColor(Qt::black, image->colorSpace()));
    manager->setResource(KoCanvasResourceProvider::ForegroundColor, i);

    i.setValue(KoColor(Qt::white, image->colorSpace()));
    manager->setResource(KoCanvasResourceProvider::BackgroundColor, i);

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

    if (!presetFileName.isEmpty()) {
        QString fullFileName = TestUtil::fetchDataFileLazy(presetFileName);
        preset = KisPaintOpPresetSP(new KisPaintOpPreset(fullFileName));
        bool presetValid = preset->load();
        Q_ASSERT(presetValid); Q_UNUSED(presetValid);

        i.setValue(preset);
        manager->setResource(KisCanvasResourceProvider::CurrentPaintOpPreset, i);
    }

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

    return manager;
}

utils::StrokeTester::StrokeTester(const QString &name, const QSize &imageSize, const QString &presetFilename)
    : m_name(name),
      m_imageSize(imageSize),
      m_presetFilename(presetFilename),
      m_numIterations(1),
      m_baseFuzziness(1)
{
}

utils::StrokeTester::~StrokeTester()
{
}

void utils::StrokeTester::setNumIterations(int value)
{
    m_numIterations = value;
}

void utils::StrokeTester::setBaseFuzziness(int value)
{
    m_baseFuzziness = value;
}

void utils::StrokeTester::testSimpleStroke()
{
    testOneStroke(false, true, false, true);
}

int utils::StrokeTester::lastStrokeTime() const
{
    return m_strokeTime;
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
    doStroke(false, false, false, false);
}

void utils::StrokeTester::testSimpleStrokeNoVerification()
{
    doStroke(false, false, true, false);
}

void utils::StrokeTester::testOneStroke(bool cancelled,
                                        bool indirectPainting,
                                        bool externalLayer,
                                        bool testUpdates)
{
    // TODO: indirectPainting option is not used anymore! The real value is
    //       taken from the preset!

    QString testName = formatTestName(m_name,
                                      cancelled,
                                      indirectPainting,
                                      externalLayer);

    dbgKrita << "Testcase:" << testName
             << "(compare against " << (testUpdates ? "projection" : "layer") << ")";

    QImage resultImage;
    resultImage = doStroke(cancelled, externalLayer, testUpdates);

    QImage refImage;
    refImage.load(referenceFile(testName));

    QPoint temp;
    if(!TestUtil::compareQImages(temp, refImage, resultImage, m_baseFuzziness, m_baseFuzziness)) {
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
                                     bool externalLayer,
                                     bool testUpdates,
                                     bool needQImage)
{
    KisImageSP image = utils::createImage(0, m_imageSize);
    KoCanvasResourceProvider *manager = utils::createResourceManager(image, 0, m_presetFilename);
    KisNodeSP currentNode;

    for (int i = 0; i < m_numIterations; i++) {
        modifyResourceManager(manager, image, i);

        KisResourcesSnapshotSP resources =
            new KisResourcesSnapshot(image,
                                     image->rootLayer()->firstChild(),
                                     manager);

        if(externalLayer) {
            KisNodeSP externalNode = new KisPaintLayer(0, "extlyr", OPACITY_OPAQUE_U8, image->colorSpace());
            resources->setCurrentNode(externalNode);
            Q_ASSERT(resources->currentNode() == externalNode);
        }

        initImage(image, resources->currentNode(), i);

        QElapsedTimer strokeTime;
        strokeTime.start();

        KisStrokeStrategy *stroke = createStroke(resources, image);
        m_strokeId = image->startStroke(stroke);
        addPaintingJobs(image, resources, i);

        if(!cancelled) {
            image->endStroke(m_strokeId);
        }
        else {
            image->cancelStroke(m_strokeId);
        }

        image->waitForDone();

        m_strokeTime = strokeTime.elapsed();
        currentNode = resources->currentNode();
    }

    beforeCheckingResult(image, currentNode);

    QImage resultImage;
    if(needQImage) {
        KisPaintDeviceSP device = testUpdates ?
            image->projection() :
            currentNode->paintDevice();

        resultImage = device->convertToQImage(0, 0, 0, image->width(), image->height());
    }

    image = 0;
    delete manager;
    return resultImage;
}

void utils::StrokeTester::modifyResourceManager(KoCanvasResourceProvider *manager,
                                                KisImageWSP image, int iteration)
{
    Q_UNUSED(iteration);
    modifyResourceManager(manager, image);
}

void utils::StrokeTester::modifyResourceManager(KoCanvasResourceProvider *manager,
                                                KisImageWSP image)
{
    Q_UNUSED(manager);
    Q_UNUSED(image);
}

void utils::StrokeTester::initImage(KisImageWSP image, KisNodeSP activeNode, int iteration)
{
    Q_UNUSED(iteration);
    initImage(image, activeNode);
}

void utils::StrokeTester::initImage(KisImageWSP image, KisNodeSP activeNode)
{
    Q_UNUSED(image);
    Q_UNUSED(activeNode);
}

void utils::StrokeTester::addPaintingJobs(KisImageWSP image,
                                          KisResourcesSnapshotSP resources,
                                          int iteration)
{
    Q_UNUSED(iteration);
    addPaintingJobs(image, resources);
}

void utils::StrokeTester::addPaintingJobs(KisImageWSP image,
                                          KisResourcesSnapshotSP resources)
{
    Q_UNUSED(image);
    Q_UNUSED(resources);
}

void utils::StrokeTester::beforeCheckingResult(KisImageWSP image, KisNodeSP activeNode)
{
    Q_UNUSED(image);
    Q_UNUSED(activeNode);
}
