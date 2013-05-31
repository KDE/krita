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

#include "filter_stroke_test.h"

#include <qtest_kde.h>
#include "stroke_testing_utils.h"
#include "strokes/kis_filter_stroke_strategy.h"
#include "kis_resources_snapshot.h"
#include "kis_image.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_registry.h"


class FilterStrokeTester : public utils::StrokeTester
{
public:
    FilterStrokeTester(const QString &filterName)
        : StrokeTester(QString("filter_") + filterName, QSize(500, 500), ""),
          m_filterName(filterName)
    {
    }

protected:
    void initImage(KisImageWSP image, KisNodeSP activeNode) {
        QImage src(QString(FILES_DATA_DIR) + QDir::separator() + "lena.png");
        activeNode->original()->convertFromQImage(src, 0);

        image->refreshGraph();
    }

    KisStrokeStrategy* createStroke(bool indirectPainting,
                                    KisResourcesSnapshotSP resources,
                                    KisPainter *painter,
                                    KisImageWSP image) {
        Q_UNUSED(image);
        Q_UNUSED(indirectPainting);
        Q_UNUSED(painter);

        KisFilterSP filter = KisFilterRegistry::instance()->value(m_filterName);
        Q_ASSERT(filter);
        KisFilterConfiguration *filterConfig = filter->defaultConfiguration(0);
        Q_ASSERT(filterConfig);

        return new KisFilterStrokeStrategy(filter, KisSafeFilterConfigurationSP(filterConfig), resources);
    }

    void addPaintingJobs(KisImageWSP image, KisResourcesSnapshotSP resources, KisPainter *painter) {

        Q_UNUSED(resources);
        Q_UNUSED(painter);

        image->addJob(strokeId(),
                      new KisFilterStrokeStrategy::
                      Data(QRect(100,100,100,100), true));

        image->addJob(strokeId(),
                      new KisFilterStrokeStrategy::
                      Data(QRect(200,100,100,100), true));

        image->addJob(strokeId(),
                      new KisFilterStrokeStrategy::
                      Data(QRect(100,200,100,100), true));
    }

private:
    QString m_filterName;
};

void FilterStrokeTest::testBlurFilter()
{
    FilterStrokeTester tester("blur");
    tester.test();
}

QTEST_KDEMAIN(FilterStrokeTest, GUI)
#include "filter_stroke_test.moc"
