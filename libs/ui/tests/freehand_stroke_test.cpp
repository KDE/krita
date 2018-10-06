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

#include "freehand_stroke_test.h"

#include <QTest>
#include <KoCompositeOpRegistry.h>
#include <KoColor.h>
#include "stroke_testing_utils.h"
#include "strokes/freehand_stroke.h"
#include "strokes/KisFreehandStrokeInfo.h"
#include "kis_resources_snapshot.h"
#include "kis_image.h"
#include "kis_painter.h"
#include <brushengine/kis_paint_information.h>

#include "kistest.h"

class FreehandStrokeTester : public utils::StrokeTester
{
public:
    FreehandStrokeTester(const QString &presetFilename, bool useLod = false)
        : StrokeTester(useLod ? "freehand-lod" : "freehand", QSize(500, 500), presetFilename),
          m_useLod(useLod),
          m_flipLineDirection(false)
    {
        setBaseFuzziness(3);
    }

    void setFlipLineDirection(bool value) {
        m_flipLineDirection = value;
        setNumIterations(2);
    }

    void setPaintColor(const QColor &color) {
        m_paintColor.reset(new QColor(color));
    }

protected:
    using utils::StrokeTester::initImage;
    void initImage(KisImageWSP image, KisNodeSP activeNode) override {
        Q_UNUSED(activeNode);

        if (m_useLod) {
            image->setDesiredLevelOfDetail(1);
        }
    }

    void beforeCheckingResult(KisImageWSP image, KisNodeSP activeNode) override {
        Q_UNUSED(image)
        Q_UNUSED(activeNode);

        if (m_useLod) {
            //image->testingSetLevelOfDetailsEnabled(true);
        }
    }

    void modifyResourceManager(KoCanvasResourceProvider *manager,
                               KisImageWSP image) override
    {
        modifyResourceManager(manager, image, 0);
    }


    void modifyResourceManager(KoCanvasResourceProvider *manager,
                               KisImageWSP image,
                               int iteration) override {

        if (m_paintColor && iteration > 0) {
            QVariant i;
            i.setValue(KoColor(*m_paintColor, image->colorSpace()));
            manager->setResource(KoCanvasResourceProvider::ForegroundColor, i);
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
        Q_UNUSED(resources);

        KisPaintInformation pi1;
        KisPaintInformation pi2;

        if (!iteration) {
            pi1 = KisPaintInformation(QPointF(200, 200));
            pi2 = KisPaintInformation(QPointF(300, 300));
        } else {
            pi1 = KisPaintInformation(QPointF(200, 300));
            pi2 = KisPaintInformation(QPointF(300, 200));
        }

        QScopedPointer<KisStrokeJobData> data(
            new FreehandStrokeStrategy::Data(0, pi1, pi2));

        image->addJob(strokeId(), data.take());
        image->addJob(strokeId(), new FreehandStrokeStrategy::UpdateData(true));
    }

private:
    bool m_useLod;
    bool m_flipLineDirection;
    QScopedPointer<QColor> m_paintColor;
};

void FreehandStrokeTest::testAutoBrushStroke()
{
    FreehandStrokeTester tester("autobrush_300px.kpp");
    tester.test();
}

void FreehandStrokeTest::testHatchingStroke()
{
    FreehandStrokeTester tester("hatching_30px.kpp");
    tester.test();
}

void FreehandStrokeTest::testColorSmudgeStroke()
{
    FreehandStrokeTester tester("colorsmudge_predefined.kpp");
    tester.test();
}

void FreehandStrokeTest::testAutoTextured17()
{
    FreehandStrokeTester tester("auto_textured_17.kpp");
    tester.test();
}

void FreehandStrokeTest::testAutoTextured38()
{
    FreehandStrokeTester tester("auto_textured_38.kpp");
    tester.test();
}

void FreehandStrokeTest::testMixDullCompositioning()
{
    FreehandStrokeTester tester("Mix_dull.kpp");
    tester.setFlipLineDirection(true);
    tester.setPaintColor(Qt::red);
    tester.test();
}

void FreehandStrokeTest::testAutoBrushStrokeLod()
{
    FreehandStrokeTester tester("Basic_tip_default.kpp", true);
    tester.testSimpleStroke();
}

void FreehandStrokeTest::testPredefinedBrushStrokeLod()
{
    qsrand(QTime::currentTime().msec());

    FreehandStrokeTester tester("testing_predefined_lod_spc13.kpp", true);
    //FreehandStrokeTester tester("testing_predefined_lod.kpp", true);
    tester.testSimpleStroke();
}

KISTEST_MAIN(FreehandStrokeTest)
