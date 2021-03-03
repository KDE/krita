/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "filter_stroke_test.h"

#include <simpletest.h>
#include "stroke_testing_utils.h"
#include "strokes/kis_filter_stroke_strategy.h"
#include "kis_resources_snapshot.h"
#include "kis_image.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_registry.h"
#include "filter/kis_filter_configuration.h"
#include <KisGlobalResourcesInterface.h>

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
        QImage src(QString(FILES_DATA_DIR) + '/' + "carrot.png");
        activeNode->original()->convertFromQImage(src, 0);

        image->refreshGraph();
    }

    KisStrokeStrategy* createStroke(KisResourcesSnapshotSP resources,
                                    KisImageWSP image) override {
        Q_UNUSED(image);

        KisFilterSP filter = KisFilterRegistry::instance()->value(m_filterName);
        Q_ASSERT(filter);
        KisFilterConfigurationSP filterConfig = filter->defaultConfiguration(KisGlobalResourcesInterface::instance());
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

SIMPLE_TEST_MAIN(FilterStrokeTest)
