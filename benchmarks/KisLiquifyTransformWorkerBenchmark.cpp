/*
 *  SPDX-FileCopyrightText: 2025 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisLiquifyTransformWorkerBenchmark.h"
#include <simpletest.h>

#include <QRandomGenerator>

#include <KoColor.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include <testutil.h>
#include <kis_liquify_transform_worker.h>
#include <kis_algebra_2d.h>


#include <quazip.h>
#include <quazipfile.h>


KisPaintDeviceSP getBlackImageDev(int size = 5000)
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(size, size, QImage::Format_RGBA8888);
    image.fill(Qt::black);

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    //qCritical() << ppVar(dev->exactBounds());
    return dev;
}



QList<QPointF> preparePointsData(const QPointF& A, const QPointF& B, const QPointF& C, const QPointF& D, int oneWayBasesCount)
{
    QList<QPointF> bases;
    for (int i = 0; i < oneWayBasesCount; i++) {
        bases << A + (B - A)*i/oneWayBasesCount;
    }

    for (int i = 0; i < oneWayBasesCount; i++) {
        bases << B + (C - B)*i/oneWayBasesCount;
    }

    for (int i = 0; i < oneWayBasesCount; i++) {
        bases << C + (D - C)*i/oneWayBasesCount;
    }
    return bases;
}

void prepareRandData(QRandomGenerator &randGen, QList<qreal> &sigmas, QList<qreal> &flows, QList<qreal> &scales, int randValuesCount)
{

    for (int i = 0; i < randValuesCount; i++) {
        sigmas << randGen.bounded(5.0);
    }

    for (int i = 0; i < randValuesCount + 1; i++) {
        flows << randGen.bounded(1.0);
    }

    for (int i = 0; i < randValuesCount + 2; i++) {
        scales << randGen.bounded(4.0) - 2.0;
    }
}

void testPoints(KisLiquifyTransformWorkerBenchmark::Operation operation, bool useWashMode, bool onlySmallAreaOfChange = false) {
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();
    const int pixelPrecision = 8;
    const int size = 5000;

    KisPaintDeviceSP dev = getBlackImageDev(size);

    KisLiquifyTransformWorker worker(dev->exactBounds(),
                                     updater,
                                     pixelPrecision);

    // A --- D
    // |     |
    // C --- B
    qreal smaller = size/5.0;
    qreal bigger = 4*size/5.0;
    if (onlySmallAreaOfChange) {
        bigger = 1.5*size/5.0;
    }

    QPointF A = QPointF(smaller, smaller);
    QPointF B = QPointF(bigger, bigger);
    QPointF C = QPointF(smaller, bigger);
    QPointF D = QPointF(bigger, smaller);


    int oneWayBasesCount = 300;
    QList<QPointF> bases = preparePointsData(A, B, C, D, oneWayBasesCount);

    QRandomGenerator randGen(1000);
    QList<qreal> sigmas; // that means "sizes"...
    QList<qreal> flows; // the "amount" or strength
    QList<qreal> scales; // the scale values for scales


    int randValuesCount = 100;

    prepareRandData(randGen, sigmas, flows, scales, randValuesCount);


    if (operation == KisLiquifyTransformWorkerBenchmark::Translate) {
        QBENCHMARK {

            for (int i = 0; i < bases.count(); i++) {
                worker.translatePoints(bases[i], QPointF(50, 0), sigmas[i%sigmas.length()], useWashMode, flows[i%flows.length()]);
            }
        }
    } else if (operation == KisLiquifyTransformWorkerBenchmark::Rotate) {
        QBENCHMARK {

            for (int i = 0; i < bases.count(); i++) {
                worker.rotatePoints(bases[i], M_PI / 4, sigmas[i%sigmas.length()], useWashMode, flows[i%flows.length()]);
            }
        }
    } else if (operation == KisLiquifyTransformWorkerBenchmark::Scale) {
        QBENCHMARK {

            for (int i = 0; i < bases.count(); i++) {
                worker.scalePoints(bases[i], scales[i%scales.length()], sigmas[i%sigmas.length()], useWashMode, flows[i%flows.length()]);
            }
        }
    } else if (operation == KisLiquifyTransformWorkerBenchmark::Undo) {

        for (int i = 0; i < bases.count(); i++) {
            worker.translatePoints(bases[i], QPointF(50, 0), sigmas[i%sigmas.length()], useWashMode, flows[i%flows.length()]);
        }

        for (int i = 0; i < bases.count(); i++) {
            worker.rotatePoints(bases[i], M_PI / 4, sigmas[i%sigmas.length()], useWashMode, flows[i%flows.length()]);
        }

        for (int i = 0; i < bases.count(); i++) {
            worker.scalePoints(bases[i], scales[i%scales.length()], sigmas[i%sigmas.length()], useWashMode, flows[i%flows.length()]);
        }


        QList<QPointF> undoBases = preparePointsData((A + D)/2, (C+B)/2, (B+D)/2, (A+C)/2, oneWayBasesCount);


        QBENCHMARK {

            for (int i = 0; i < undoBases.count(); i++) {
                worker.undoPoints(undoBases[i], flows[i%flows.length()], sigmas[i%sigmas.length()]);
            }
        }
    } else if (operation == KisLiquifyTransformWorkerBenchmark::RunOnQImage) {
        for (int i = 0; i < bases.count(); i++) {
            worker.translatePoints(bases[i], QPointF(50, 0), sigmas[i%sigmas.length()], useWashMode, flows[i%flows.length()]);
        }

        for (int i = 0; i < bases.count(); i++) {
            worker.rotatePoints(bases[i], M_PI / 4, sigmas[i%sigmas.length()], useWashMode, flows[i%flows.length()]);
        }

        for (int i = 0; i < bases.count(); i++) {
            worker.scalePoints(bases[i], scales[i%scales.length()], sigmas[i%sigmas.length()], useWashMode, flows[i%flows.length()]);
        }

        QImage image(size, size, QImage::Format_ARGB32);
        image.fill(Qt::black);
        QPointF newOffset;

        QBENCHMARK {
            worker.runOnQImage(image, QPointF(), QTransform(), &newOffset);
        }

    } else if (operation == KisLiquifyTransformWorkerBenchmark::RunOnDev) {
        for (int i = 0; i < bases.count(); i++) {
            worker.translatePoints(bases[i], QPointF(50, 0), sigmas[i%sigmas.length()], useWashMode, flows[i%flows.length()]);
        }

        for (int i = 0; i < bases.count(); i++) {
            worker.rotatePoints(bases[i], M_PI / 4, sigmas[i%sigmas.length()], useWashMode, flows[i%flows.length()]);
        }

        for (int i = 0; i < bases.count(); i++) {
            worker.scalePoints(bases[i], scales[i%scales.length()], sigmas[i%sigmas.length()], useWashMode, flows[i%flows.length()]);
        }

        KisPaintDeviceSP newDev = getBlackImageDev();

        QBENCHMARK {
            worker.run(dev, newDev);
        }
    }
    else if (operation == KisLiquifyTransformWorkerBenchmark::BenchmarkCopyConstructor) {
        int cases = 1000;
        //qCritical() << "cases: " << cases << "number of points: " << worker.originalPoints().length();

        KisLiquifyTransformWorker previous(worker);
        QSize prevSize;
        QBENCHMARK {
            for (int i = 0; i < cases; i++) {
                //previous = KisLiquifyTransformWorker(previous);
                KisLiquifyTransformWorker here(previous);
                prevSize = here.gridSize();
            }
        }
    }
    else if (operation == KisLiquifyTransformWorkerBenchmark::MovePointsOutsideOfTheGrid) {

        QRect bounds = dev->exactBounds();
        QPointF addition = bounds.topLeft() - QPointF(bounds.width(), bounds.height());


        QBENCHMARK {
            for (int i = 0; i < bases.count(); i++) {
                worker.translatePoints(addition/2 + bases[i], QPointF(50, 0), sigmas[i%sigmas.length()], useWashMode, flows[i%flows.length()]);
            }

            KisPaintDeviceSP newDev = getBlackImageDev();

            QBENCHMARK {
                worker.run(dev, newDev);
            }
        }

    } //

}


void KisLiquifyTransformWorkerBenchmark::testInitialization()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();
    const int pixelPrecision = 8;

    KisPaintDeviceSP dev = getBlackImageDev();

    QBENCHMARK {
    KisLiquifyTransformWorker worker(dev->exactBounds(),
                                     updater,
                                     pixelPrecision);
    }
}

void KisLiquifyTransformWorkerBenchmark::testPointsTranslateBuildUp()
{
    testPoints(Translate, false);
}

void KisLiquifyTransformWorkerBenchmark::testPointsTranslateWash()
{
    testPoints(Translate, true);
}

void KisLiquifyTransformWorkerBenchmark::testPointsScaleBuildUp()
{
    testPoints(Scale, false);
}

void KisLiquifyTransformWorkerBenchmark::testPointsScaleWash()
{
    testPoints(Scale, true);
}

void KisLiquifyTransformWorkerBenchmark::testPointsRotateBuildUp()
{
    testPoints(Rotate, false);
}

void KisLiquifyTransformWorkerBenchmark::testPointsRotateWash()
{
    testPoints(Rotate, true);
}

void KisLiquifyTransformWorkerBenchmark::testPointsUndoBuildUp()
{
    testPoints(Undo, false);
}

void KisLiquifyTransformWorkerBenchmark::testPointsUndoWash()
{
    testPoints(Undo, true);
}

void KisLiquifyTransformWorkerBenchmark::testRunOnDev()
{
    testPoints(RunOnDev, true);
}

void KisLiquifyTransformWorkerBenchmark::testRunOnQImage()
{
    testPoints(RunOnQImage, true);
}

void KisLiquifyTransformWorkerBenchmark::testSmallChangeOnDev()
{
    testPoints(RunOnDev, true, true);
}

void KisLiquifyTransformWorkerBenchmark::testSmallChangeOnQImage()
{
    testPoints(RunOnQImage, true, true);
}


KisLiquifyTransformWorker* getWorkerFromIODeviceXml(QIODevice* device)
{
    QDomDocument doc;

    doc.setContent(device);

    QDomElement rootElement = doc.documentElement();
    QDomElement data = rootElement.firstChildElement("data");
    return KisLiquifyTransformWorker::fromXML(data);
}


KisLiquifyTransformWorker* getWorkerFromXml(QString filename)
{
    filename = TestUtil::fetchDataFileLazy(filename);


    if (filename.endsWith("zip")) {
        QuaZip zipArchive(filename);
        bool result = zipArchive.open(QuaZip::mdUnzip);
        if (!result) return nullptr;
        QStringList filenames = zipArchive.getFileNameList();
        if (filenames.length() < 1) {
            return nullptr;
        }
        // it should be the first and only file
        QString correctFilename = filenames[0];
        if (!zipArchive.setCurrentFile(correctFilename)) {
            qWarning() << "Could not set current file" << zipArchive.getZipError() << correctFilename;
            return nullptr;
        }

        QuaZipFile zipFile = QuaZipFile(&zipArchive);
        if (!zipFile.open(QIODevice::ReadOnly)) {
            qWarning() << "It could not open the file" << zipArchive.getZipError();
            return nullptr;
        }
        return getWorkerFromIODeviceXml(&zipFile);

    } else {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            ENTER_FUNCTION() << "Could not find the file" << filename;
            return nullptr;
        }

        return getWorkerFromIODeviceXml(&file);

    }

}


void KisLiquifyTransformWorkerBenchmark::testRenderingMask_data()
{

    QTest::addColumn<QString>("filename");
    QTest::addColumn<bool>("onQImage");

    QTest::addRow("liquify_mask_typical_config, true") << "liquify_mask_typical_config.zip" << true;
    QTest::addRow("liquify_mask_typical_config, false") << "liquify_mask_typical_config.zip" << false;

    QTest::addRow("liquify_mask_simpler_config, true") << "liquify_mask_simpler_config.zip" << true;
    QTest::addRow("liquify_mask_simpler_config, false") << "liquify_mask_simpler_config.zip" << false;

    QTest::addRow("liquify_mask_unusual_config, true") << "liquify_mask_unusual_config.zip" << true;
    QTest::addRow("liquify_mask_unusual_config, false") << "liquify_mask_unusual_config.zip" << false;


}

void KisLiquifyTransformWorkerBenchmark::testRenderingMask()
{

    QFETCH(QString, filename);
    QFETCH(bool, onQImage);

    QScopedPointer<KisLiquifyTransformWorker> liquifyWorker;

    liquifyWorker.reset(getWorkerFromXml(filename));

    int size = 2000;
    KisPaintDeviceSP newDev = getBlackImageDev(size);

    if (onQImage) {

        QImage image = newDev->convertToQImage(0);
        QImage dst = image.copy();

        QPointF p;

        QBENCHMARK {
            QImage dst = liquifyWorker->runOnQImage(image, p, QTransform(), &p);
        }

    } else {

        QBENCHMARK {
            liquifyWorker->run(newDev, newDev);
        }
    }

}


void KisLiquifyTransformWorkerBenchmark::testCopyConstructor()
{
    testPoints(BenchmarkCopyConstructor, true, false);

}

void KisLiquifyTransformWorkerBenchmark::testMovePointsOutsideOfTheGrid()
{
    testPoints(MovePointsOutsideOfTheGrid, true, false);
}

void KisLiquifyTransformWorkerBenchmark::checkMath()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();
    const int pixelPrecision = 8;
    const int size = 100;

    KisPaintDeviceSP dev = getBlackImageDev(size);

    KisLiquifyTransformWorker worker(dev->exactBounds(),
                                     updater,
                                     pixelPrecision);

    // A --- D
    // |     |
    // C --- B
    qreal smaller = size/5.0;
    qreal bigger = 4*size/5.0;

    QPointF A = QPointF(smaller, smaller);
    QPointF B = QPointF(bigger, bigger);
    QPointF C = QPointF(smaller, bigger);
    QPointF D = QPointF(bigger, smaller);


    int oneWayBasesCount = 300;
    QList<QPointF> bases = preparePointsData(A, B, C, D, oneWayBasesCount);

    QRandomGenerator randGen(1000);
    QList<qreal> sigmas; // that means "sizes"...
    QList<qreal> flows; // the "amount" or strength
    QList<qreal> scales; // the scale values for scales


    int randValuesCount = 100;

    prepareRandData(randGen, sigmas, flows, scales, randValuesCount);
}

SIMPLE_TEST_MAIN(KisLiquifyTransformWorkerBenchmark)
