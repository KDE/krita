#include "kis_mypaintop_test.h"
#include <kis_my_paintop_settings.h>
#include "qimage_based_test.h"
#include <kis_my_paintop.h>
#include <stroke_testing_utils.h>
#include <kis_paint_information.h>
#include <kis_distance_information.h>
#include <kis_mypaint_surface.h>
#include <kis_node_facade.h>
#include <QImageReader>
#include <kis_group_layer.h>
#include <kis_paintop_preset.h>
#include <kis_random_accessor_ng.h>

class KisMyPaintOpSettings;
KisMyPaintOpTest::KisMyPaintOpTest(): TestUtil::QImageBasedTest("")
{
}

void KisMyPaintOpTest::testDab() {

    KisSurrogateUndoStore *undoStore = new KisSurrogateUndoStore();
    KisImageSP image = createTrivialImage(undoStore);
    image->initialRefreshGraph();

    KisNodeSP paintNode = findNode(image->root(), "paint1");
    KisPainter gc(paintNode->paintDevice());

    KisPaintDeviceSP dst = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb16());
    KisPainter painter(paintNode->paintDevice());

    mypaint_brush_new();

    KisMyPaintSurface *surface = new KisMyPaintSurface(&painter, paintNode);

    surface->draw_dab(surface->surface(), 250, 250, 100, 0, 0, 1, 1, 0.8, 1, 1, 90, 0, 0);

    QImage img = paintNode->paintDevice()->convertToQImage(0, image->bounds().x(), image->bounds().y(), image->bounds().width(), image->bounds().height());
    QImage source(QString(FILES_DATA_DIR) + QDir::separator() + "draw_dab.png");

    QVERIFY(img == source);
}

void KisMyPaintOpTest::testGetColor() {

    KisSurrogateUndoStore *undoStore = new KisSurrogateUndoStore();
    KisImageSP image = createTrivialImage(undoStore);
    image->initialRefreshGraph();

    KisNodeSP paintNode = findNode(image->root(), "paint1");
    KisPainter gc(paintNode->paintDevice());

    KisPaintDeviceSP dst = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb16());
    KisPainter painter(paintNode->paintDevice());

    mypaint_brush_new();

    KisMyPaintSurface *surface = new KisMyPaintSurface(&painter, paintNode);

    surface->draw_dab(surface->surface(), 250, 250, 100, 0, 0, 1, 1, 0.8, 1, 1, 90, 0, 0);

    QImage img = paintNode->paintDevice()->convertToQImage(0, image->bounds().x(), image->bounds().y(), image->bounds().width(), image->bounds().height());
    QImage source(QString(FILES_DATA_DIR) + QDir::separator() + "draw_dab.png");

    QVERIFY(img == source);

    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 0.0f;

    surface->get_color(surface->surface(), 250, 250, 100, &r, &g, &b, &a);

    QVERIFY(qFuzzyCompare((float)qRound(r), 0.0L));
    QVERIFY(qFuzzyCompare((float)qRound(g), 0.0L));
    QVERIFY(qFuzzyCompare((float)qRound(b), 1.0L));
    QVERIFY(qFuzzyCompare((float)qRound(a), 1.0L));
}

QTEST_MAIN(KisMyPaintOpTest)
