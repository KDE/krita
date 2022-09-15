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
#include <brushengine/kis_paint_information.h>
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

        gc.paintEllipse(1.0, 1.0, 0.0, {2.500000, 2.500000});
        gc.paintEllipse(1.0, 1.0, 0.0, {5.500000, 2.500000});
        gc.paintEllipse(1.0, 1.0, 0.001, {8.500000, 2.500000});
        gc.paintEllipse(1.0, 1.0, 3.447332, {11.500000, 2.500000});
        gc.paintEllipse(1.0, 1.0, 2.720089, {14.500000, 2.500000});
        gc.paintEllipse(1.0, 1.0, 4.708133, {17.500000, 2.500000});
        gc.paintEllipse(1.0, 2.0, 0.0, {21.500000, 3.000000});
        gc.paintEllipse(2.0, 1.0, 0.0, {25.000000, 3.500000});
        gc.paintEllipse(1.0, 2.0, 0.001, {29.500000, 3.000000});
        gc.paintEllipse(1.0, 2.0, 4.599109, {33.500000, 3.000000});
        gc.paintEllipse(1.0, 2.0, 3.477489, {37.500000, 3.000000});
        gc.paintEllipse(1.0, 2.0, 3.965196, {41.500000, 3.000000});
        gc.paintEllipse(2.0, 2.0, 0.0, {45.000000, 3.000000});
        gc.paintEllipse(2.0, 2.0, 0.0, {49.000000, 3.000000});
        gc.paintEllipse(2.0, 2.0, 0.001, {53.000000, 3.000000});
        gc.paintEllipse(2.0, 2.0, 1.536669, {57.000000, 3.000000});
        gc.paintEllipse(2.0, 2.0, 0.843838, {61.000000, 3.000000});
        gc.paintEllipse(2.0, 2.0, 0.679193, {65.000000, 3.000000});
        gc.paintEllipse(3.0, 4.0, 0.0, {70.500000, 4.000000});
        gc.paintEllipse(4.0, 3.0, 0.0, {76.000000, 4.500000});
        gc.paintEllipse(3.0, 4.0, 0.001, {82.500000, 4.000000});
        gc.paintEllipse(3.0, 4.0, 4.937069, {88.500000, 4.000000});
        gc.paintEllipse(3.0, 4.0, 2.123195, {94.500000, 4.000000});
        gc.paintEllipse(3.0, 4.0, 2.730237, {100.500000, 4.000000});
        gc.paintEllipse(4.0, 5.0, 0.0, {106.000000, 4.500000});
        gc.paintEllipse(5.0, 4.0, 0.0, {113.500000, 4.000000});
        gc.paintEllipse(4.0, 5.0, 0.001, {120.000000, 4.500000});
        gc.paintEllipse(4.0, 5.0, 1.812736, {127.000000, 4.500000});
        gc.paintEllipse(4.0, 5.0, 1.543446, {134.000000, 4.500000});
        gc.paintEllipse(4.0, 5.0, 3.835928, {141.000000, 4.500000});
        gc.paintEllipse(5.0, 5.0, 0.0, {148.500000, 4.500000});
        gc.paintEllipse(5.0, 5.0, 0.0, {155.500000, 4.500000});
        gc.paintEllipse(5.0, 5.0, 0.001, {162.500000, 4.500000});
        gc.paintEllipse(5.0, 5.0, 4.865655, {169.500000, 4.500000});
        gc.paintEllipse(5.0, 5.0, 1.121816, {176.500000, 4.500000});
        gc.paintEllipse(5.0, 5.0, 3.689620, {183.500000, 4.500000});
        gc.paintEllipse(3.0, 3.0, 0.0, {190.500000, 4.500000});
        gc.paintEllipse(3.0, 3.0, 0.0, {195.500000, 4.500000});
        gc.paintEllipse(3.0, 3.0, 0.001, {200.500000, 4.500000});
        gc.paintEllipse(3.0, 3.0, 4.578076, {205.500000, 4.500000});
        gc.paintEllipse(3.0, 3.0, 3.880855, {210.500000, 4.500000});
        gc.paintEllipse(3.0, 3.0, 1.469428, {215.500000, 4.500000});
        gc.paintEllipse(4.0, 4.0, 0.0, {220.000000, 4.000000});
        gc.paintEllipse(4.0, 4.0, 0.0, {226.000000, 4.000000});
        gc.paintEllipse(4.0, 4.0, 0.001, {232.000000, 4.000000});
        gc.paintEllipse(4.0, 4.0, 4.376009, {238.000000, 4.000000});
        gc.paintEllipse(4.0, 4.0, 3.304281, {244.000000, 4.000000});
        gc.paintEllipse(4.0, 4.0, 3.212126, {250.000000, 4.000000});
        gc.paintEllipse(6.0, 2.0, 0.0, {257.000000, 5.000000});
        gc.paintEllipse(2.0, 6.0, 0.0, {265.000000, 5.000000});
        gc.paintEllipse(6.0, 2.0, 0.001, {273.000000, 5.000000});
        gc.paintEllipse(6.0, 2.0, 1.840461, {281.000000, 5.000000});
        gc.paintEllipse(6.0, 2.0, 4.966129, {289.000000, 5.000000});
        gc.paintEllipse(6.0, 2.0, 3.084118, {297.000000, 5.000000});
        gc.paintEllipse(7.0, 7.0, 0.0, {306.500000, 6.500000});
        gc.paintEllipse(7.0, 7.0, 0.0, {315.500000, 6.500000});
        gc.paintEllipse(7.0, 7.0, 0.001, {324.500000, 6.500000});
        gc.paintEllipse(7.0, 7.0, 0.491244, {333.500000, 6.500000});
        gc.paintEllipse(7.0, 7.0, 2.549983, {342.500000, 6.500000});
        gc.paintEllipse(7.0, 7.0, 0.281522, {351.500000, 6.500000});
        gc.paintEllipse(8.0, 7.0, 0.0, {360.000000, 6.500000});
        gc.paintEllipse(7.0, 8.0, 0.0, {370.500000, 6.000000});
        gc.paintEllipse(8.0, 7.0, 0.001, {380.000000, 6.500000});
        gc.paintEllipse(8.0, 7.0, 3.477640, {390.000000, 6.500000});
        gc.paintEllipse(8.0, 7.0, 4.408846, {400.000000, 6.500000});
        gc.paintEllipse(8.0, 7.0, 2.289880, {410.000000, 6.500000});
        gc.paintEllipse(1.0, 10.0, 0.0, {421.500000, 7.000000});
        gc.paintEllipse(10.0, 1.0, 0.0, {433.000000, 7.500000});
        gc.paintEllipse(1.0, 10.0, 0.001, {445.500000, 7.000000});
        gc.paintEllipse(1.0, 10.0, 1.103278, {457.500000, 7.000000});
        gc.paintEllipse(1.0, 10.0, 2.534889, {469.500000, 7.000000});
        gc.paintEllipse(1.0, 10.0, 2.576652, {481.500000, 7.000000});
        gc.paintEllipse(2.0, 10.0, 0.0, {493.000000, 7.000000});
        gc.paintEllipse(10.0, 2.0, 0.0, {505.000000, 7.000000});
        gc.paintEllipse(2.0, 10.0, 0.001, {517.000000, 7.000000});
        gc.paintEllipse(2.0, 10.0, 0.322600, {529.000000, 7.000000});
        gc.paintEllipse(2.0, 10.0, 4.415827, {541.000000, 7.000000});
        gc.paintEllipse(2.0, 10.0, 3.546348, {553.000000, 7.000000});
        gc.paintEllipse(3.0, 10.0, 0.0, {565.500000, 7.000000});
        gc.paintEllipse(10.0, 3.0, 0.0, {577.000000, 7.500000});
        gc.paintEllipse(3.0, 10.0, 0.001, {589.500000, 7.000000});
        gc.paintEllipse(3.0, 10.0, 3.702482, {601.500000, 7.000000});
        gc.paintEllipse(3.0, 10.0, 1.093167, {613.500000, 7.000000});
        gc.paintEllipse(3.0, 10.0, 4.595159, {625.500000, 7.000000});
        gc.paintEllipse(4.0, 10.0, 0.0, {637.000000, 7.000000});
        gc.paintEllipse(10.0, 4.0, 0.0, {649.000000, 7.000000});
        gc.paintEllipse(4.0, 10.0, 0.001, {661.000000, 7.000000});
        gc.paintEllipse(4.0, 10.0, 0.227650, {673.000000, 7.000000});
        gc.paintEllipse(4.0, 10.0, 2.752410, {685.000000, 7.000000});
        gc.paintEllipse(4.0, 10.0, 0.097503, {697.000000, 7.000000});
        gc.paintEllipse(3.0, 70.0, 0.0, {739.500000, 37.000000});
        gc.paintEllipse(70.0, 3.0, 0.0, {811.000000, 37.500000});
        gc.paintEllipse(3.0, 70.0, 0.001, {883.500000, 37.000000});
        gc.paintEllipse(3.0, 70.0, 4.841069, {955.500000, 37.000000});
        gc.paintEllipse(3.0, 70.0, 2.882992, {35.500000, 109.000000});
        gc.paintEllipse(3.0, 70.0, 1.564884, {107.500000, 109.000000});
        gc.paintEllipse(57.0, 46.0, 0.0, {172.500000, 102.000000});
        gc.paintEllipse(46.0, 57.0, 0.0, {231.000000, 102.500000});
        gc.paintEllipse(57.0, 46.0, 0.001, {290.500000, 102.000000});
        gc.paintEllipse(57.0, 46.0, 1.865616, {349.500000, 102.000000});
        gc.paintEllipse(57.0, 46.0, 1.645545, {408.500000, 102.000000});
        gc.paintEllipse(57.0, 46.0, 2.495434, {467.500000, 102.000000});
        gc.paintEllipse(26.0, 68.0, 0.0, {532.000000, 108.000000});
        gc.paintEllipse(68.0, 26.0, 0.0, {602.000000, 108.000000});
        gc.paintEllipse(26.0, 68.0, 0.001, {672.000000, 108.000000});
        gc.paintEllipse(26.0, 68.0, 2.300878, {742.000000, 108.000000});
        gc.paintEllipse(26.0, 68.0, 0.896425, {812.000000, 108.000000});
        gc.paintEllipse(26.0, 68.0, 1.497499, {882.000000, 108.000000});
        gc.paintEllipse(52.0, 91.0, 0.0, {46.000000, 192.500000});
        gc.paintEllipse(91.0, 52.0, 0.0, {139.500000, 192.000000});
        gc.paintEllipse(52.0, 91.0, 0.001, {232.000000, 192.500000});
        gc.paintEllipse(52.0, 91.0, 0.483947, {325.000000, 192.500000});
        gc.paintEllipse(52.0, 91.0, 4.318118, {418.000000, 192.500000});
        gc.paintEllipse(52.0, 91.0, 3.013741, {511.000000, 192.500000});
        gc.paintEllipse(13.0, 74.0, 0.0, {595.500000, 183.000000});
        gc.paintEllipse(74.0, 13.0, 0.0, {671.000000, 183.500000});
        gc.paintEllipse(13.0, 74.0, 0.001, {747.500000, 183.000000});
        gc.paintEllipse(13.0, 74.0, 2.692361, {823.500000, 183.000000});
        gc.paintEllipse(13.0, 74.0, 2.600321, {899.500000, 183.000000});
        gc.paintEllipse(13.0, 74.0, 3.130989, {37.500000, 276.000000});

        // FIXME: I tried to use a brush with spacing here, but won't draw... So using some random brush here.
        QScopedPointer<KoCanvasResourceProvider> Brush2Manager(
                utils::createResourceManager(image, 0, "test_smudge_20px_dul_nsa_new.0001.kpp"));

        Brush2Manager->setResource(KoCanvasResource::ForegroundColor, KoColor(Qt::darkGreen, image->colorSpace()));

        KisResourcesSnapshotSP Brush2resources =
                new KisResourcesSnapshot(image, targetNode, Brush2Manager.data());

        Brush2resources->setupPainter(&gc);

        gc.paintEllipse(236.0, 228.0, 6.011207, {194.000000, 357.000000});
        gc.paintEllipse(236.0, 228.0, 2.430340, {472.000000, 357.000000});
        gc.paintEllipse(270.0, 234.0, 4.957483, {767.000000, 374.000000});
        gc.paintEllipse(270.0, 234.0, 7.588924, {135.000000, 686.000000});
        gc.paintEllipse(164.0, 295.0, 3.351935, {460.000000, 699.500000});
        gc.paintEllipse(164.0, 295.0, 0.357193, {797.000000, 699.500000});
        gc.paintEllipse(183.0, 255.0, 2.087918, {128.500000, 1016.500000});
        gc.paintEllipse(183.0, 255.0, 3.908099, {425.500000, 1016.500000});
        gc.paintEllipse(114.0, 137.0, 5.177951, {662.000000, 956.500000});
        gc.paintEllipse(114.0, 137.0, 2.653228, {841.000000, 956.500000});
        gc.paintEllipse(123.0, 240.0, 9.426152, {120.500000, 1305.000000});
        gc.paintEllipse(123.0, 240.0, 6.487866, {402.500000, 1305.000000});
        gc.paintEllipse(122.0, 178.0, 7.776356, {653.000000, 1274.000000});
        gc.paintEllipse(122.0, 178.0, 9.863346, {89.000000, 1556.000000});
        gc.paintEllipse(146.0, 137.0, 2.545302, {293.000000, 1540.500000});
        gc.paintEllipse(146.0, 137.0, 9.990388, {481.000000, 1540.500000});
        gc.paintEllipse(195.0, 198.0, 9.384000, {695.500000, 1566.000000});
        gc.paintEllipse(195.0, 198.0, 8.611850, {99.500000, 1806.000000});
        gc.paintEllipse(205.0, 255.0, 7.340687, {368.500000, 1835.500000});
        gc.paintEllipse(205.0, 255.0, 1.145769, {665.500000, 1835.500000});
        gc.paintEllipse(57.0, 46.0, 8.865715, {862.500000, 1735.000000});
        gc.paintEllipse(57.0, 46.0, 3.082175, {28.500000, 2032.000000});
        gc.paintEllipse(52.0, 91.0, 5.834873, {145.000000, 2050.500000});
        gc.paintEllipse(52.0, 91.0, 9.462468, {278.000000, 2050.500000});

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

        gc.paintEllipse(50.0, 14000.0, M_PI_4, {5000.000000, 5000.000000});

        manager->setResource(KoCanvasResource::ForegroundColor, KoColor(Qt::darkYellow, image->colorSpace()));

        gc.paintEllipse(9974.0, 9988.0, M_PI_4 + 0.23167, {5000.000000, 5000.000000});

        QImage I = gc.device().data()->convertToQImage(0);

        TestUtil::checkQImage(I, "", "test", "ellipses_precision");
    }
};

void KisEllipseTest::testDrawing(){
    KisEllipseTestInner t;
    t.testDrawing();
}

void KisEllipseTest::testPrecision() {
    KisEllipseTestInner t;
    t.testPrecision();
}

KISTEST_MAIN(KisEllipseTest)





