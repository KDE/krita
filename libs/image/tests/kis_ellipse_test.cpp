#include "kis_ellipse_test.h"

#include <simpletest.h>

#include "kistest.h"
#include "testimage.h"
#include "testbrush.h"
#include "testflake.h"
#include "testpigment.h"
#include "testresources.h"
#include "testui.h"
#include "testutil.h"

#include "kis_algebra_2d.h"
#include "kis_types.h"
#include "KoCanvasResourceProvider.h"
#include "qimage_based_test.h"
#include <stroke_testing_utils.h>
#include <KoCanvasResourcesIds.h>
#include <brushengine/kis_paintop_preset.h>
#include <brushengine/kis_paintop_settings.h>

class KisEllipseTestInner : public TestUtil::QImageBasedTest {
public:

    KisEllipseTestInner() : QImageBasedTest("ellipse") {

    }

    void testDrawing() {

        KisSurrogateUndoStore *undoStore = new KisSurrogateUndoStore();
        KisImageSP image = createTrivialImage(undoStore);
        image->initialRefreshGraph();
        image->resizeImage(QRect(0, 0, 1000, 1000));
        image->waitForDone();

        KisNodeSP paint1 = findNode(image->root(), "paint1");

        QVERIFY(paint1->extent().isEmpty());

        KisNodeSP targetNode = paint1;

        KisPainter gc(targetNode->paintDevice());

        QScopedPointer<KoCanvasResourceProvider> manager(
                utils::createResourceManager(image, 0, "Pixel_Art_Copy.kpp"));

        manager->setResource(KoCanvasResource::ForegroundColor, KoColor(Qt::green, image->colorSpace()));

        KisResourcesSnapshotSP resources =
                new KisResourcesSnapshot(image, targetNode, manager.data());

        resources->setupPainter(&gc);

        gc.paintEllipse(1.0, 1.0, 0.0, {2.5, 2.5});
        gc.paintEllipse(1.0, 1.0, 0.0, {5.5, 2.5});
        gc.paintEllipse(1.0, 1.0, 0.001, {8.5, 2.5});
        gc.paintEllipse(1.0, 1.0, 3.447332, {11.5, 2.5});
        gc.paintEllipse(1.0, 1.0, 2.720089, {14.5, 2.5});
        gc.paintEllipse(1.0, 1.0, 4.708133, {17.5, 2.5});
        gc.paintEllipse(1.0, 2.0, 0.0, {21.5, 3.0});
        gc.paintEllipse(2.0, 1.0, 0.0, {25.0, 3.5});
        gc.paintEllipse(1.0, 2.0, 0.001, {29.5, 3.0});
        gc.paintEllipse(1.0, 2.0, 4.599109, {33.5, 3.0});
        gc.paintEllipse(1.0, 2.0, 3.477489, {37.5, 3.0});
        gc.paintEllipse(1.0, 2.0, 3.965196, {41.5, 3.0});
        gc.paintEllipse(2.0, 2.0, 0.0, {45.0, 3.0});
        gc.paintEllipse(2.0, 2.0, 0.0, {49.0, 3.0});
        gc.paintEllipse(2.0, 2.0, 0.001, {53.0, 3.0});
        gc.paintEllipse(2.0, 2.0, 1.536669, {57.0, 3.0});
        gc.paintEllipse(2.0, 2.0, 0.843838, {61.0, 3.0});
        gc.paintEllipse(2.0, 2.0, 0.679193, {65.0, 3.0});
        gc.paintEllipse(3.0, 4.0, 0.0, {70.5, 4.0});
        gc.paintEllipse(4.0, 3.0, 0.0, {76.0, 4.5});
        gc.paintEllipse(3.0, 4.0, 0.001, {82.5, 4.0});
        gc.paintEllipse(3.0, 4.0, 4.937069, {88.5, 4.0});
        gc.paintEllipse(3.0, 4.0, 2.123195, {94.5, 4.0});
        gc.paintEllipse(3.0, 4.0, 2.730237, {100.5, 4.0});
        gc.paintEllipse(4.0, 5.0, 0.0, {106.0, 4.5});
        gc.paintEllipse(5.0, 4.0, 0.0, {113.5, 4.0});
        gc.paintEllipse(4.0, 5.0, 0.001, {120.0, 4.5});
        gc.paintEllipse(4.0, 5.0, 1.812736, {127.0, 4.5});
        gc.paintEllipse(4.0, 5.0, 1.543446, {134.0, 4.5});
        gc.paintEllipse(4.0, 5.0, 3.835928, {141.0, 4.5});
        gc.paintEllipse(5.0, 5.0, 0.0, {148.5, 4.5});
        gc.paintEllipse(5.0, 5.0, 0.0, {155.5, 4.5});
        gc.paintEllipse(5.0, 5.0, 0.001, {162.5, 4.5});
        gc.paintEllipse(5.0, 5.0, 4.865655, {169.5, 4.5});
        gc.paintEllipse(5.0, 5.0, 1.121816, {176.5, 4.5});
        gc.paintEllipse(5.0, 5.0, 3.689620, {183.5, 4.5});
        gc.paintEllipse(3.0, 3.0, 0.0, {190.5, 4.5});
        gc.paintEllipse(3.0, 3.0, 0.0, {195.5, 4.5});
        gc.paintEllipse(3.0, 3.0, 0.001, {200.5, 4.5});
        gc.paintEllipse(3.0, 3.0, 4.578076, {205.5, 4.5});
        gc.paintEllipse(3.0, 3.0, 3.880855, {210.5, 4.5});
        gc.paintEllipse(3.0, 3.0, 1.469428, {215.5, 4.5});
        gc.paintEllipse(4.0, 4.0, 0.0, {220.0, 4.0});
        gc.paintEllipse(4.0, 4.0, 0.0, {226.0, 4.0});
        gc.paintEllipse(4.0, 4.0, 0.001, {232.0, 4.0});
        gc.paintEllipse(4.0, 4.0, 4.376009, {238.0, 4.0});
        gc.paintEllipse(4.0, 4.0, 3.304281, {244.0, 4.0});
        gc.paintEllipse(4.0, 4.0, 3.212126, {250.0, 4.0});
        gc.paintEllipse(6.0, 2.0, 0.0, {257.0, 5.0});
        gc.paintEllipse(2.0, 6.0, 0.0, {265.0, 5.0});
        gc.paintEllipse(6.0, 2.0, 0.001, {273.0, 5.0});
        gc.paintEllipse(6.0, 2.0, 1.840461, {281.0, 5.0});
        gc.paintEllipse(6.0, 2.0, 4.966129, {289.0, 5.0});
        gc.paintEllipse(6.0, 2.0, 3.084118, {297.0, 5.0});
        gc.paintEllipse(7.0, 7.0, 0.0, {306.5, 6.5});
        gc.paintEllipse(7.0, 7.0, 0.0, {315.5, 6.5});
        gc.paintEllipse(7.0, 7.0, 0.001, {324.5, 6.5});
        gc.paintEllipse(7.0, 7.0, 0.491244, {333.5, 6.5});
        gc.paintEllipse(7.0, 7.0, 2.549983, {342.5, 6.5});
        gc.paintEllipse(7.0, 7.0, 0.281522, {351.5, 6.5});
        gc.paintEllipse(8.0, 7.0, 0.0, {360.0, 6.5});
        gc.paintEllipse(7.0, 8.0, 0.0, {370.5, 6.0});
        gc.paintEllipse(8.0, 7.0, 0.001, {380.0, 6.5});
        gc.paintEllipse(8.0, 7.0, 3.477640, {390.0, 6.5});
        gc.paintEllipse(8.0, 7.0, 4.408846, {400.0, 6.5});
        gc.paintEllipse(8.0, 7.0, 2.289880, {410.0, 6.5});
        gc.paintEllipse(1.0, 10.0, 0.0, {421.5, 7.0});
        gc.paintEllipse(10.0, 1.0, 0.0, {433.0, 7.5});
        gc.paintEllipse(1.0, 10.0, 0.001, {445.5, 7.0});
        gc.paintEllipse(1.0, 10.0, 1.103278, {457.5, 7.0});
        gc.paintEllipse(1.0, 10.0, 2.534889, {469.5, 7.0});
        gc.paintEllipse(1.0, 10.0, 2.576652, {481.5, 7.0});
        gc.paintEllipse(2.0, 10.0, 0.0, {493.0, 7.0});
        gc.paintEllipse(10.0, 2.0, 0.0, {505.0, 7.0});
        gc.paintEllipse(2.0, 10.0, 0.001, {517.0, 7.0});
        gc.paintEllipse(2.0, 10.0, 0.322600, {529.0, 7.0});
        gc.paintEllipse(2.0, 10.0, 4.415827, {541.0, 7.0});
        gc.paintEllipse(2.0, 10.0, 3.546348, {553.0, 7.0});
        gc.paintEllipse(3.0, 10.0, 0.0, {565.5, 7.0});
        gc.paintEllipse(10.0, 3.0, 0.0, {577.0, 7.5});
        gc.paintEllipse(3.0, 10.0, 0.001, {589.5, 7.0});
        gc.paintEllipse(3.0, 10.0, 3.702482, {601.5, 7.0});
        gc.paintEllipse(3.0, 10.0, 1.093167, {613.5, 7.0});
        gc.paintEllipse(3.0, 10.0, 4.595159, {625.5, 7.0});
        gc.paintEllipse(4.0, 10.0, 0.0, {637.0, 7.0});
        gc.paintEllipse(10.0, 4.0, 0.0, {649.0, 7.0});
        gc.paintEllipse(4.0, 10.0, 0.001, {661.0, 7.0});
        gc.paintEllipse(4.0, 10.0, 0.227650, {673.0, 7.0});
        gc.paintEllipse(4.0, 10.0, 2.752410, {685.0, 7.0});
        gc.paintEllipse(4.0, 10.0, 0.097503, {697.0, 7.0});
        gc.paintEllipse(3.0, 70.0, 0.0, {739.5, 37.0});
        gc.paintEllipse(70.0, 3.0, 0.0, {811.0, 37.5});
        gc.paintEllipse(3.0, 70.0, 0.001, {883.5, 37.0});
        gc.paintEllipse(3.0, 70.0, 4.841069, {955.5, 37.0});
        gc.paintEllipse(3.0, 70.0, 2.882992, {35.5, 109.0});
        gc.paintEllipse(3.0, 70.0, 1.564884, {107.5, 109.0});
        gc.paintEllipse(57.0, 46.0, 0.0, {172.5, 102.0});
        gc.paintEllipse(46.0, 57.0, 0.0, {231.0, 102.5});
        gc.paintEllipse(57.0, 46.0, 0.001, {290.5, 102.0});
        gc.paintEllipse(57.0, 46.0, 1.865616, {349.5, 102.0});
        gc.paintEllipse(57.0, 46.0, 1.645545, {408.5, 102.0});
        gc.paintEllipse(57.0, 46.0, 2.495434, {467.5, 102.0});
        gc.paintEllipse(26.0, 68.0, 0.0, {532.0, 108.0});
        gc.paintEllipse(68.0, 26.0, 0.0, {602.0, 108.0});
        gc.paintEllipse(26.0, 68.0, 0.001, {672.0, 108.0});
        gc.paintEllipse(26.0, 68.0, 2.300878, {742.0, 108.0});
        gc.paintEllipse(26.0, 68.0, 0.896425, {812.0, 108.0});
        gc.paintEllipse(26.0, 68.0, 1.497499, {882.0, 108.0});
        gc.paintEllipse(52.0, 91.0, 0.0, {46.0, 192.5});
        gc.paintEllipse(91.0, 52.0, 0.0, {139.5, 192.0});
        gc.paintEllipse(52.0, 91.0, 0.001, {232.0, 192.5});
        gc.paintEllipse(52.0, 91.0, 0.483947, {325.0, 192.5});
        gc.paintEllipse(52.0, 91.0, 4.318118, {418.0, 192.5});
        gc.paintEllipse(52.0, 91.0, 3.013741, {511.0, 192.5});
        gc.paintEllipse(13.0, 74.0, 0.0, {595.5, 183.0});
        gc.paintEllipse(74.0, 13.0, 0.0, {671.0, 183.5});
        gc.paintEllipse(13.0, 74.0, 0.001, {747.5, 183.0});
        gc.paintEllipse(13.0, 74.0, 2.692361, {823.5, 183.0});
        gc.paintEllipse(13.0, 74.0, 2.600321, {899.5, 183.0});
        gc.paintEllipse(13.0, 74.0, 3.130989, {37.5, 276.0});

        // FIXME: I tried to use a brush with spacing here, but won't draw... So using some random brush here.
        QScopedPointer<KoCanvasResourceProvider> Brush2Manager(
                utils::createResourceManager(image, 0, "test_smudge_20px_dul_nsa_new.0001.kpp"));

        Brush2Manager->setResource(KoCanvasResource::ForegroundColor, KoColor(Qt::darkGreen, image->colorSpace()));
        Brush2Manager->setResource(KoCanvasResource::BackgroundColor, KoColor(Qt::red, image->colorSpace()));

        KisResourcesSnapshotSP Brush2resources =
                new KisResourcesSnapshot(image, targetNode, Brush2Manager.data());

        Brush2resources->setupPainter(&gc);

        gc.setFillStyle(KisPainter::FillStyleBackgroundColor);

        gc.paintEllipse(236.0, 228.0, 6.011207, {194.0, 357.0});
        gc.paintEllipse(236.0, 228.0, 2.430340, {472.0, 357.0});
        gc.paintEllipse(270.0, 234.0, 4.957483, {767.0, 374.0});
        gc.paintEllipse(270.0, 234.0, 7.588924, {135.0, 686.0});
        gc.paintEllipse(164.0, 295.0, 3.351935, {460.0, 699.5});
        gc.paintEllipse(164.0, 295.0, 0.357193, {797.0, 699.5});
        gc.paintEllipse(183.0, 255.0, 2.087918, {128.5, 1016.5});
        gc.paintEllipse(183.0, 255.0, 3.908099, {425.5, 1016.5});
        gc.paintEllipse(114.0, 137.0, 5.177951, {662.0, 956.5});
        gc.paintEllipse(114.0, 137.0, 2.653228, {841.0, 956.5});
        gc.setFillStyle(KisPainter::FillStyleNone);
        gc.paintEllipse(123.0, 240.0, 9.426152, {120.5, 1305.0});
        gc.paintEllipse(123.0, 240.0, 6.487866, {402.5, 1305.0});
        gc.paintEllipse(122.0, 178.0, 7.776356, {653.0, 1274.0});
        gc.paintEllipse(122.0, 178.0, 9.863346, {89.0, 1556.0});
        gc.paintEllipse(146.0, 137.0, 2.545302, {293.0, 1540.5});
        gc.paintEllipse(146.0, 137.0, 9.990388, {481.0, 1540.5});
        gc.paintEllipse(195.0, 198.0, 9.384000, {695.5, 1566.0});
        gc.paintEllipse(195.0, 198.0, 8.611850, {99.5, 1806.0});
        gc.paintEllipse(205.0, 255.0, 7.340687, {368.5, 1835.5});
        gc.paintEllipse(205.0, 255.0, 1.145769, {665.5, 1835.5});
        gc.paintEllipse(57.0, 46.0, 8.865715, {862.5, 1735.0});
        gc.paintEllipse(57.0, 46.0, 3.082175, {28.5, 2032.0});
        gc.paintEllipse(52.0, 91.0, 5.834873, {145.0, 2050.5});
        gc.paintEllipse(52.0, 91.0, 9.462468, {278.0, 2050.5});

        QImage I = gc.device().data()->convertToQImage(0);
        TestUtil::checkQImage(I, "", "test", "ellipses");
    }

    void testPrecision() {
        KisSurrogateUndoStore *undoStore = new KisSurrogateUndoStore();
        KisImageSP image = createTrivialImage(undoStore);
        image->initialRefreshGraph();
        image->resizeImage(QRect(0, 0, 1000, 1000));
        image->waitForDone();

        KisNodeSP paint1 = findNode(image->root(), "paint1");

        QVERIFY(paint1->extent().isEmpty());

        KisNodeSP targetNode = paint1;

        KisPainter gc(targetNode->paintDevice());

        QScopedPointer<KoCanvasResourceProvider> manager(
                utils::createResourceManager(image, 0, "Pixel_Art_Copy.kpp"));

        manager->setResource(KoCanvasResource::ForegroundColor, KoColor(Qt::green, image->colorSpace()));

        KisResourcesSnapshotSP resources =
                new KisResourcesSnapshot(image, targetNode, manager.data());

        resources->setupPainter(&gc);

        gc.paintEllipse(50.0, 14000.0, M_PI_4, {5000.0, 5000.0});

        manager->setResource(KoCanvasResource::ForegroundColor, KoColor(Qt::darkYellow, image->colorSpace()));

        gc.paintEllipse(9974.0, 9988.0, M_PI_4 + 0.23167, {5000.0, 5000.0});

        QImage I = gc.device().data()->convertToQImage(0);

        TestUtil::checkQImage(I, "", "test", "ellipses_precision");
    }
};

void KisEllipseTest::testDrawing(){
    KisEllipseTestInner t;
    t.testDrawing();
}

void KisEllipseTest::testPrecision() {
    // TODO: Add some tolerance here, because the reference image is generated with 80-bit floating point numbers
    // TODO: and there's only IEEE 754 on many platforms, so on these platforms the test may fail.
    KisEllipseTestInner t;
    t.testPrecision();
}

KISTEST_MAIN(KisEllipseTest)





