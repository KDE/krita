/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_crash_filter_test.h"
#include <KoColorProfile.h>
#include <QTest>
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "kis_selection.h"
#include "kis_processing_information.h"
#include "filter/kis_filter.h"
#include "kis_pixel_selection.h"
#include <KoColorSpaceRegistry.h>
#include "kis_transaction.h"
#include <sdk/tests/testing_timed_default_bounds.h>


bool KisCrashFilterTest::applyFilter(const KoColorSpace * cs,  KisFilterSP f)
{

    QImage qimage(QString(FILES_DATA_DIR) + QDir::separator() + "carrot.png");

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->setDefaultBounds(new TestUtil::TestingTimedDefaultBounds(qimage.rect()));
    dev->convertFromQImage(qimage, 0, 0, 0);

    // Get the predefined configuration from a file
    KisFilterConfigurationSP  kfc = f->defaultConfiguration();

    QFile file(QString(FILES_DATA_DIR) + QDir::separator() + f->id() + ".cfg");
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
        // XXX: Let's not check the painterly colorspaces right now
        if (colorSpace->id().startsWith("KS", Qt::CaseInsensitive)) {
            continue;
        }

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

#include <sdk/tests/kistest.h>
KISTEST_MAIN(KisCrashFilterTest)
