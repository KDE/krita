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

#include <QTest>
#include "stroke_testing_utils.h"
#include "strokes/kis_filter_stroke_strategy.h"
#include "kis_resources_snapshot.h"
#include "kis_image.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_registry.h"
#include "filter/kis_filter_configuration.h"


class FilterStrokeTester : public utils::StrokeTester
{
public:
    FilterStrokeTester(const QString &filterName)
        : StrokeTester(QString("filter_") + filterName, QSize(500, 500), ""),
          m_filterName(filterName)
    {
        setBaseFuzziness(5);
    }

protected:
    using utils::StrokeTester::initImage;
    using utils::StrokeTester::addPaintingJobs;

    void initImage(KisImageWSP image, KisNodeSP activeNode) override {
        QImage src(QString(FILES_DATA_DIR) + QDir::separator() + "carrot.png");
        activeNode->original()->convertFromQImage(src, 0);

        image->refreshGraph();
    }

    KisStrokeStrategy* createStroke(KisResourcesSnapshotSP resources,
                                    KisImageWSP image) override {
        Q_UNUSED(image);

        KisFilterSP filter = KisFilterRegistry::instance()->value(m_filterName);
        Q_ASSERT(filter);
        KisFilterConfigurationSP filterConfig = filter->defaultConfiguration();
        Q_ASSERT(filterConfig);

        return new KisFilterStrokeStrategy(filter, KisFilterConfigurationSP(filterConfig), resources);
    }

    void addPaintingJobs(KisImageWSP image, KisResourcesSnapshotSP resources) override {

        Q_UNUSED(resources);

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

QTEST_MAIN(FilterStrokeTest)
