/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "FreehandStrokeBenchmark.h"

#include <QTest>
#include <KoCompositeOpRegistry.h>
#include <KoColor.h>
#include "stroke_testing_utils.h"
#include "strokes/freehand_stroke.h"
#include "strokes/KisFreehandStrokeInfo.h"
#include "kis_resources_snapshot.h"
#include "kis_image.h"
#include <brushengine/kis_paint_information.h>

class FreehandStrokeBenchmarkTester : public utils::StrokeTester
{
public:
    FreehandStrokeBenchmarkTester(const QString &presetFilename)
        : StrokeTester("freehand_benchmark", QSize(5000, 5000), presetFilename)
    {
        setBaseFuzziness(3);
    }

    void setCpuCoresLimit(int value) {
        m_cpuCoresLimit = value;
    }

protected:
    using utils::StrokeTester::initImage;
    void initImage(KisImageWSP image, KisNodeSP activeNode) override {
        Q_UNUSED(activeNode);

        if (m_cpuCoresLimit > 0) {
            image->setWorkingThreadsLimit(m_cpuCoresLimit);
        }
    }

    KisStrokeStrategy* createStroke(KisResourcesSnapshotSP resources,
                                    KisImageWSP image) override {
        Q_UNUSED(image);

        KisFreehandStrokeInfo *strokeInfo = new KisFreehandStrokeInfo();

        QScopedPointer<FreehandStrokeStrategy> stroke(
            new FreehandStrokeStrategy(resources, strokeInfo, kundo2_noi18n("Freehand Stroke")));

        return stroke.take();
    }

    void addPaintingJobs(KisImageWSP image,
                                 KisResourcesSnapshotSP resources) override
    {
        addPaintingJobs(image, resources, 0);
    }

    void addPaintingJobs(KisImageWSP image, KisResourcesSnapshotSP resources, int iteration) override {
        Q_UNUSED(iteration);
        Q_UNUSED(resources);

        for (int y = 100; y < 4900; y += 300) {
            KisPaintInformation pi1;
            KisPaintInformation pi2;

            pi1 = KisPaintInformation(QPointF(100, y), 0.5);
            pi2 = KisPaintInformation(QPointF(4900, y + 100), 1.0);

            QScopedPointer<KisStrokeJobData> data(
                new FreehandStrokeStrategy::Data(0, pi1, pi2));

            image->addJob(strokeId(), data.take());
        }

        image->addJob(strokeId(), new FreehandStrokeStrategy::UpdateData(true));
    }

private:
    int m_cpuCoresLimit = -1;
};

void benchmarkBrush(const QString &presetName)
{
    FreehandStrokeBenchmarkTester tester(presetName);

    for (int i = 1; i <= QThread::idealThreadCount(); i++) {
        tester.setCpuCoresLimit(i);
        tester.benchmark();

        qDebug() << qPrintable(QString("Cores: %1 Time: %2 (ms)").arg(i).arg(tester.lastStrokeTime()));
    }
}

#include <KoResourcePaths.h>

void FreehandStrokeBenchmark::initTestCase()
{
    KoResourcePaths::addResourceType("brushes", "data", FILES_DATA_DIR);
}

void FreehandStrokeBenchmark::testDefaultTip()
{
    benchmarkBrush("testing_1000px_auto_deafult.kpp");
}

void FreehandStrokeBenchmark::testSoftTip()
{
    benchmarkBrush("testing_1000px_auto_soft.kpp");
}

void FreehandStrokeBenchmark::testGaussianTip()
{
    benchmarkBrush("testing_1000px_auto_gaussian.kpp");
}

void FreehandStrokeBenchmark::testRectangularTip()
{
    benchmarkBrush("testing_1000px_auto_rectangular.kpp");
}

void FreehandStrokeBenchmark::testRectGaussianTip()
{
    benchmarkBrush("testing_1000px_auto_gaussian_rect.kpp");
}

void FreehandStrokeBenchmark::testRectSoftTip()
{
    benchmarkBrush("testing_1000px_auto_soft_rect.kpp");
}

void FreehandStrokeBenchmark::testStampTip()
{
    benchmarkBrush("testing_1000px_stamp_450_rotated.kpp");
}

void FreehandStrokeBenchmark::testColorsmudgeDefaultTip()
{
    benchmarkBrush("testing_200px_colorsmudge_default.kpp");
}

QTEST_MAIN(FreehandStrokeBenchmark)
