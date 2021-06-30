/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "FreehandStrokeBenchmark.h"

#include <simpletest.h>
#include <sdk/tests/testui.h>
#include <KoCompositeOpRegistry.h>
#include <KoColor.h>
#include "stroke_testing_utils.h"
#include "strokes/freehand_stroke.h"
#include "strokes/KisFreehandStrokeInfo.h"
#include "KisAsyncronousStrokeUpdateHelper.h"
#include "kis_resources_snapshot.h"
#include "kis_image.h"
#include <brushengine/kis_paint_information.h>

#include "testutil.h"
#include "KisResourceModel.h"
#include "KisGlobalResourcesInterface.h"


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

        image->addJob(strokeId(), new KisAsyncronousStrokeUpdateHelper::UpdateData(true));
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

void benchmarkBrushUnthreaded(const QString &presetName)
{
    FreehandStrokeBenchmarkTester tester(presetName);


    Q_FOREACH(int i, QVector<int>({1, QThread::idealThreadCount()})) {
        tester.setCpuCoresLimit(i);
        tester.benchmark();

        qDebug() << qPrintable(QString("Cores: %1 Time: %2 (ms)").arg(i).arg(tester.lastStrokeTime()));
    }
}

#include <KoResourcePaths.h>

void FreehandStrokeBenchmark::initTestCase()
{
    {
        QString fullFileName = TestUtil::fetchDataFileLazy("3_texture.png");
        KIS_ASSERT(!fullFileName.isEmpty());
        KIS_ASSERT(QFileInfo(fullFileName).exists());

        KisResourceModel model(ResourceType::Brushes);
        model.importResourceFile(fullFileName);
    }

    {
        QString fullFileName = TestUtil::fetchDataFileLazy("DA_RGBA bluegreen_small1.png");
        KIS_ASSERT(!fullFileName.isEmpty());
        KIS_ASSERT(QFileInfo(fullFileName).exists());

        KisResourceModel model(ResourceType::Brushes);
        model.importResourceFile(fullFileName);
    }
}

void FreehandStrokeBenchmark::testDefaultTip()
{
    benchmarkBrush("testing_1000px_auto_deafult.kpp");
}

void FreehandStrokeBenchmark::testSoftTip()
{
    benchmarkBrushUnthreaded("testing_1000px_auto_soft.kpp");
}

void FreehandStrokeBenchmark::testGaussianTip()
{
    benchmarkBrushUnthreaded("testing_1000px_auto_gaussian.kpp");
}

void FreehandStrokeBenchmark::testRectangularTip()
{
    benchmarkBrushUnthreaded("testing_1000px_auto_rectangular.kpp");
}

void FreehandStrokeBenchmark::testRectGaussianTip()
{
    benchmarkBrushUnthreaded("testing_1000px_auto_gaussian_rect.kpp");
}

void FreehandStrokeBenchmark::testRectSoftTip()
{
    benchmarkBrushUnthreaded("testing_1000px_auto_soft_rect.kpp");
}

void FreehandStrokeBenchmark::testStampTip()
{
    benchmarkBrushUnthreaded("testing_1000px_stamp_450_rotated.kpp");
}

void FreehandStrokeBenchmark::testColorsmudgeDefaultTip_dull_old_sa()
{
    benchmarkBrushUnthreaded("testing_200px_colorsmudge_default_dulling_old_sa.kpp");
}

void FreehandStrokeBenchmark::testColorsmudgeDefaultTip_dull_old_nsa()
{
    benchmarkBrushUnthreaded("testing_200px_colorsmudge_defaut_dulling_old_nsa.kpp");
}

void FreehandStrokeBenchmark::testColorsmudgeDefaultTip_dull_new_sa()
{
    benchmarkBrushUnthreaded("testing_200px_colorsmudge_defaut_dulling_new_sa.kpp");
}

void FreehandStrokeBenchmark::testColorsmudgeDefaultTip_dull_new_nsa()
{
    benchmarkBrushUnthreaded("testing_200px_colorsmudge_defaut_dulling_new_nsa.kpp");
}

void FreehandStrokeBenchmark::testColorsmudgeDefaultTip_smear_old_sa()
{
    benchmarkBrushUnthreaded("testing_200px_colorsmudge_defaut_smearing_old_sa.kpp");
}

void FreehandStrokeBenchmark::testColorsmudgeDefaultTip_smear_old_nsa()
{
    benchmarkBrushUnthreaded("testing_200px_colorsmudge_defaut_smearing_old_nsa.kpp");
}

void FreehandStrokeBenchmark::testColorsmudgeDefaultTip_smear_new_sa()
{
    benchmarkBrushUnthreaded("testing_200px_colorsmudge_defaut_smearing_new_sa.kpp");
}

void FreehandStrokeBenchmark::testColorsmudgeDefaultTip_smear_new_nsa()
{
    benchmarkBrushUnthreaded("testing_200px_colorsmudge_defaut_smearing_new_nsa.kpp");
}

void FreehandStrokeBenchmark::testColorsmudgeLightness_smear_new_nsa_maskmode()
{
    benchmarkBrushUnthreaded("testing_200px_colorsmudge_lightness_smearing_new_nsa_maskmode.kpp");
}

void FreehandStrokeBenchmark::testColorsmudgeLightness_smear_new_nsa_nopt()
{
    benchmarkBrushUnthreaded("testing_200px_colorsmudge_lightness_smearing_new_nsa_nopt.kpp");
}

void FreehandStrokeBenchmark::testColorsmudgeLightness_smear_new_nsa_ptoverlay()
{
    benchmarkBrushUnthreaded("testing_200px_colorsmudge_lightness_smearing_new_nsa_ptoverlay.kpp");
}

void FreehandStrokeBenchmark::testColorsmudgeLightness_smear_new_nsa_ptoverwrite()
{
    benchmarkBrushUnthreaded("testing_200px_colorsmudge_lightness_smearing_new_nsa_ptoverwrite.kpp");
}

KISTEST_MAIN(FreehandStrokeBenchmark)
