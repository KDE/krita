/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_crash_filter_test.h"
#include <KoColorProfile.h>
#include <simpletest.h>
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "kis_selection.h"
#include "kis_processing_information.h"
#include "filter/kis_filter.h"
#include "kis_pixel_selection.h"
#include <KoColorSpaceRegistry.h>
#include "kis_transaction.h"
#include <sdk/tests/testing_timed_default_bounds.h>
#include <KisGlobalResourcesInterface.h>


bool KisCrashFilterTest::applyFilter(const KoColorSpace * cs,  KisFilterSP f)
{

    QImage qimage(QString(FILES_DATA_DIR) + '/' + "carrot.png");

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->setDefaultBounds(new TestUtil::TestingTimedDefaultBounds(qimage.rect()));
    dev->convertFromQImage(qimage, 0, 0, 0);

    // Get the predefined configuration from a file
    KisFilterConfigurationSP  kfc = f->defaultConfiguration(KisGlobalResourcesInterface::instance());

    QFile file(QString(FILES_DATA_DIR) + '/' + f->id() + ".cfg");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        dbgKrita << "creating new file for " << f->id();
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << kfc->toXML();
    } else {
        QString s;
        QTextStream in(&file);
        in.setCodec("UTF-8");
        s = in.readAll();
        kfc->fromXML(s);
    }
    dbgKrita << f->id() << ", " << cs->id() << ", " << cs->profile()->name();// << kfc->toXML() << "\n";

    {
        kfc->createLocalResourcesSnapshot(KisGlobalResourcesInterface::instance());
        KisTransaction t(kundo2_noi18n(f->name()), dev);
        f->process(dev, QRect(QPoint(0,0), qimage.size()), kfc);
    }

    return true;

}

bool KisCrashFilterTest::testFilter(KisFilterSP f)
{
    QList<const KoColorSpace*> colorSpaces = KoColorSpaceRegistry::instance()->allColorSpaces(KoColorSpaceRegistry::AllColorSpaces, KoColorSpaceRegistry::OnlyDefaultProfile);
    bool ok = false;
    Q_FOREACH (const KoColorSpace* colorSpace, colorSpaces) {

        // Alpha color spaces are never processed directly. They are
        // first converted into GrayA color space
        if (colorSpace->id().startsWith("ALPHA", Qt::CaseInsensitive)) {
            continue;
        }

        ok = applyFilter(colorSpace, f);
    }

    return ok;
}

void KisCrashFilterTest::testCrashFilters()
{
    QStringList excludeFilters;
    excludeFilters << "colortransfer";
    excludeFilters << "gradientmap";
    excludeFilters << "phongbumpmap";
    excludeFilters << "perchannel";
    excludeFilters << "height to normal";


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

#include <sdk/tests/testimage.h>
KISTEST_MAIN(KisCrashFilterTest)
