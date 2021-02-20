/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_all_filter_test.h"
#include <QTest>
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "kis_selection.h"
#include "kis_processing_information.h"
#include "filter/kis_filter.h"
#include "kis_pixel_selection.h"
#include "kis_transaction.h"
#include <KoColorSpaceRegistry.h>
#include <sdk/tests/qimage_test_util.h>
#include <sdk/tests/testing_timed_default_bounds.h>
#include <KisGlobalResourcesInterface.h>

bool testFilterSrcNotIsDev(KisFilterSP f)
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    QImage qimage(QString(FILES_DATA_DIR) + '/' + "carrot.png");
    QImage result(QString(FILES_DATA_DIR) + '/' + "carrot_" + f->id() + ".png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->setDefaultBounds(new TestUtil::TestingTimedDefaultBounds(qimage.rect()));

    KisPaintDeviceSP dstdev = new KisPaintDevice(cs);
    dstdev->setDefaultBounds(new TestUtil::TestingTimedDefaultBounds(qimage.rect()));

    dev->convertFromQImage(qimage, 0, 0, 0);

    // Get the predefined configuration from a file
    KisFilterConfigurationSP  kfc = f->defaultConfiguration(KisGlobalResourcesInterface::instance());

    QFile file(QString(FILES_DATA_DIR) + '/' + f->id() + ".cfg");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        //qDebug() << "creating new file for " << f->id();
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << kfc->toXML();
    } else {
        QString s;
        QTextStream in(&file);
        in.setCodec("UTF-8");
        s = in.readAll();
        //qDebug() << "Read for " << f->id() << "\n" << s;
        kfc->fromXML(s);
    }
    dbgKrita << f->id();// << "\n" << kfc->toXML() << "\n";

    kfc->createLocalResourcesSnapshot(KisGlobalResourcesInterface::instance());
    f->process(dev, dstdev, 0, QRect(QPoint(0,0), qimage.size()), kfc);

    QPoint errpoint;

    QImage actualResult = dstdev->convertToQImage(0, 0, 0, qimage.width(), qimage.height());

    if (!TestUtil::compareQImages(errpoint, result, actualResult, 1, 1)) {
        qDebug() << "Failed compare result images for: " << f->id();
        qDebug() << errpoint;
        actualResult.save(QString("carrot_%1.png").arg(f->id()));
        result.save(QString("carrot_%1_expected.png").arg(f->id()));
        return false;
    }
    return true;
}

bool testFilter(KisFilterSP f)
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    QImage qimage(QString(FILES_DATA_DIR) + '/' + "carrot.png");
    QString resultFileName = QString(FILES_DATA_DIR) + '/' + "carrot_" + f->id() + ".png";
    QImage result(resultFileName);

    //if (!f->id().contains("hsv")) return true;

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->setDefaultBounds(new TestUtil::TestingTimedDefaultBounds(qimage.rect()));
    dev->convertFromQImage(qimage, 0, 0, 0);
    KisTransaction * cmd = new KisTransaction(kundo2_noi18n(f->name()), dev);

    // Get the predefined configuration from a file
    KisFilterConfigurationSP  kfc = f->defaultConfiguration(KisGlobalResourcesInterface::instance());

    QFile file(QString(FILES_DATA_DIR) + '/' + f->id() + ".cfg");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        //qDebug() << "creating new file for " << f->id();
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << kfc->toXML();
    } else {
        QString s;
        QTextStream in(&file);
        in.setCodec("UTF-8");
        s = in.readAll();
        //qDebug() << "Read for " << f->id() << "\n" << s;
        const bool validConfig = kfc->fromXML(s);


        if (!validConfig) {
            qDebug() << QString("Couldn't parse XML settings for filter %1").arg(f->id()).toLatin1();
            return false;
        }
    }
    dbgKrita << f->id();// << "\n" << kfc->toXML() << "\n";
    kfc->createLocalResourcesSnapshot(KisGlobalResourcesInterface::instance());

    f->process(dev, QRect(QPoint(0,0), qimage.size()), kfc);

    QPoint errpoint;

    delete cmd;

    QImage actualResult = dev->convertToQImage(0, 0, 0, qimage.width(), qimage.height());

    if (!TestUtil::compareQImages(errpoint, result, actualResult, 1, 1)) {
        qDebug() << "Failed compare result images for: " << f->id();
        qDebug() << errpoint;
        actualResult.save(QString("carrot_%1.png").arg(f->id()));
        result.save(QString("carrot_%1_expected.png").arg(f->id()));
        return false;
    }
    return true;
}


bool testFilterWithSelections(KisFilterSP f)
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    QImage qimage(QString(FILES_DATA_DIR) + '/' + "carrot.png");
    QImage result(QString(FILES_DATA_DIR) + '/' + "carrot_" + f->id() + ".png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->setDefaultBounds(new TestUtil::TestingTimedDefaultBounds(qimage.rect()));
    dev->convertFromQImage(qimage, 0, 0, 0);

    // Get the predefined configuration from a file
    KisFilterConfigurationSP  kfc = f->defaultConfiguration(KisGlobalResourcesInterface::instance());

    QFile file(QString(FILES_DATA_DIR) + '/' + f->id() + ".cfg");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        //qDebug() << "creating new file for " << f->id();
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << kfc->toXML();
    } else {
        QString s;
        QTextStream in(&file);
        in.setCodec("UTF-8");
        s = in.readAll();
        //qDebug() << "Read for " << f->id() << "\n" << s;
        kfc->fromXML(s);
    }
    dbgKrita << f->id();// << "\n"; << kfc->toXML() << "\n";

    KisSelectionSP sel1 = new KisSelection(new KisSelectionDefaultBounds(dev));
    sel1->pixelSelection()->select(qimage.rect());

    kfc->createLocalResourcesSnapshot(KisGlobalResourcesInterface::instance());
    f->process(dev, dev, sel1, QRect(QPoint(0,0), qimage.size()), kfc);

    QPoint errpoint;

    QImage actualResult = dev->convertToQImage(0, 0, 0, qimage.width(), qimage.height());

    if (!TestUtil::compareQImages(errpoint, result, actualResult, 1, 1)) {
        qDebug() << "Failed compare result images for: " << f->id();
        qDebug() << errpoint;
        actualResult.save(QString("carrot_%1.png").arg(f->id()));
        result.save(QString("carrot_%1_expected.png").arg(f->id()));
        return false;
    }

    return true;
}

void KisAllFilterTest::testAllFilters()
{
    QStringList excludeFilters;
    excludeFilters << "colortransfer";
    excludeFilters << "gradientmap";
    excludeFilters << "phongbumpmap";
    excludeFilters << "raindrops";

    // halftone has some bezier curve painting drifts, so
    // let's just exclude it
    excludeFilters << "halftone";

    QStringList failures;
    QStringList successes;

    QList<QString> filterList = KisFilterRegistry::instance()->keys();
    std::sort(filterList.begin(), filterList.end());
    for (QList<QString>::Iterator it = filterList.begin(); it != filterList.end(); ++it) {
        if (excludeFilters.contains(*it)) continue;

        if (testFilter(KisFilterRegistry::instance()->value(*it)))
            successes << *it;
        else
            failures << *it;
    }
    dbgKrita << "Success: " << successes;
    if (failures.size() > 0) {
        QFAIL(QString("Failed filters:\n\t %1").arg(failures.join("\n\t")).toLatin1());
    }
}

void KisAllFilterTest::testAllFiltersSrcNotIsDev()
{
    QStringList excludeFilters;
    excludeFilters << "colortransfer";
    excludeFilters << "gradientmap";
    excludeFilters << "phongbumpmap";
    excludeFilters << "raindrops";

    // halftone has some bezier curve painting drifts, so
    // let's just exclude it
    excludeFilters << "halftone";

    QStringList failures;
    QStringList successes;

    QList<QString> filterList = KisFilterRegistry::instance()->keys();
    std::sort(filterList.begin(), filterList.end());
    for (QList<QString>::Iterator it = filterList.begin(); it != filterList.end(); ++it) {
        if (excludeFilters.contains(*it)) continue;

        if (testFilterSrcNotIsDev(KisFilterRegistry::instance()->value(*it)))
            successes << *it;
        else
            failures << *it;
    }
    dbgKrita << "Src!=Dev Success: " << successes;
    if (failures.size() > 0) {
        QFAIL(QString("Src!=Dev Failed filters:\n\t %1").arg(failures.join("\n\t")).toLatin1());
    }

}

void KisAllFilterTest::testAllFiltersWithSelections()
{
    QStringList excludeFilters;
    excludeFilters << "colortransfer";
    excludeFilters << "gradientmap";
    excludeFilters << "phongbumpmap";
    excludeFilters << "raindrops";

    // halftone has some bezier curve painting drifts, so
    // let's just exclude it
    excludeFilters << "halftone";

    QStringList failures;
    QStringList successes;

    QList<QString> filterList = KisFilterRegistry::instance()->keys();
    std::sort(filterList.begin(), filterList.end());
    for (QList<QString>::Iterator it = filterList.begin(); it != filterList.end(); ++it) {
        if (excludeFilters.contains(*it)) continue;

        if (testFilterWithSelections(KisFilterRegistry::instance()->value(*it)))
            successes << *it;
        else
            failures << *it;
    }
    dbgKrita << "Success: " << successes;
    if (failures.size() > 0) {
        QFAIL(QString("Failed filters with selections:\n\t %1").arg(failures.join("\n\t")).toLatin1());
    }
}



#include <sdk/tests/testimage.h>
KISTEST_MAIN(KisAllFilterTest)
