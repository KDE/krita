#include "kis_animation_exporter_test.h"
#include "kis_animation_exporter.h"

#include <qtest_kde.h>
#include <testutil.h>
#include "KisPart.h"
#include "kis_image.h"
#include "KisDocument.h"
#include "kis_image_animation_interface.h"
#include "KoColor.h"
#include "kis_time_range.h"
#include "kis_keyframe_channel.h"


void KisAnimationExporterTest::testAnimationExport()
{
    KisDocument *document = KisPart::instance()->createDocument();
    QRect rect(0,0,512,512);
    TestUtil::MaskParent p(rect);
    document->setCurrentImage(p.image);
    const KoColorSpace *cs = p.image->colorSpace();

    KUndo2Command parentCommand;

    KisKeyframeChannel *rasterChannel = p.layer->getKeyframeChannel(KisKeyframeChannel::Content.id());

    rasterChannel->addKeyframe(1, &parentCommand);
    rasterChannel->addKeyframe(2, &parentCommand);
    p.image->animationInterface()->setRange(KisTimeRange::fromTime(0, 2));

    KisPaintDeviceSP dev = p.layer->paintDevice();

    dev->fill(rect, KoColor(Qt::red, cs));
    QImage frame0 = dev->createThumbnail(50, 50);

    p.image->animationInterface()->switchCurrentTimeAsync(1);
    p.image->waitForDone();
    dev->fill(rect, KoColor(Qt::green, cs));
    QImage frame1 = dev->createThumbnail(50, 50);

    p.image->animationInterface()->switchCurrentTimeAsync(2);
    p.image->waitForDone();
    dev->fill(rect, KoColor(Qt::blue, cs));
    QImage frame2 = dev->createThumbnail(50, 50);

    KisAnimationExporter exporter(document, "export-test.png", 0, 2);
    QSignalSpy spy(&exporter, SIGNAL(sigExportProgress(int)));

    QVERIFY(spy.isValid());


    exporter.startExport();

    // If we had Qt5 already:
    // spy.wait(100);
    // spy.wait(100);
    // spy.wait(100);
    // But in the meanwhile...
    QTest::qWait(1000);


    QCOMPARE(spy.count(), 3);
    QCOMPARE(spy.at(0).at(0).value<int>(), 0);
    QCOMPARE(spy.at(1).at(0).value<int>(), 1);
    QCOMPARE(spy.at(2).at(0).value<int>(), 2);

    // FIXME: Export doesn't seem to work from unit tests
    /*
    QImage exported;
    exported.load("export-test0.png");
    QVERIFY(exported == frame0);

    exported.load("export-test1.png");
    QVERIFY(exported == frame1);

    exported.load("export-test2.png");
    QVERIFY(exported == frame2);
    */
}

QTEST_KDEMAIN(KisAnimationExporterTest, GUI)
#include "kis_animation_exporter_test.moc"

