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

#include <qtest_kde.h>
#include <KoCompositeOpRegistry.h>
#include "stroke_testing_utils.h"
#include "strokes/freehand_stroke.h"
#include "kis_resources_snapshot.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_paint_information.h"


class FreehandStrokeTester : public utils::StrokeTester
{
public:
    FreehandStrokeTester(const QString &presetFilename, bool useLod = false)
        : StrokeTester(useLod ? "freehand-lod" : "freehand", QSize(500, 500), presetFilename),
          m_useLod(useLod)
    {
    }

protected:
    void initImage(KisImageWSP image, KisNodeSP activeNode) {
        Q_UNUSED(activeNode);

        if (m_useLod) {
            image->setDesiredLevelOfDetail(1);
        }
    }

    void beforeCheckingResult(KisImageWSP image, KisNodeSP activeNode) {
        Q_UNUSED(activeNode);

        if (m_useLod) {
            image->testingSetLevelOfDetailsEnabled(true);
        }
    }

    KisStrokeStrategy* createStroke(bool indirectPainting,
                                    KisResourcesSnapshotSP resources,
                                    KisPainter *painter,
                                    KisImageWSP image) {
        Q_UNUSED(image);

        m_painterInfo =
            new FreehandStrokeStrategy::PainterInfo(painter,
                                                    new KisDistanceInformation());

        QScopedPointer<FreehandStrokeStrategy> stroke(
            new FreehandStrokeStrategy(indirectPainting, COMPOSITE_ALPHA_DARKEN, resources, m_painterInfo, kundo2_noi18n("Freehand Stroke")));

        return m_useLod ? stroke->createLodClone(1) : stroke.take();
    }

    void addPaintingJobs(KisImageWSP image, KisResourcesSnapshotSP resources, KisPainter *painter) {

        Q_ASSERT(painter == m_painterInfo->painter);
        Q_UNUSED(painter);

        KisPaintInformation pi1;
        KisPaintInformation pi2;

        pi1 = KisPaintInformation(QPointF(200, 200));
        pi2 = KisPaintInformation(QPointF(300, 300));

        QScopedPointer<KisStrokeJobData> data(
            new FreehandStrokeStrategy::Data(resources->currentNode(),
                                             m_painterInfo, pi1, pi2));

        image->addJob(strokeId(), m_useLod ? data->createLodClone(1) : data.take());
    }

private:
    FreehandStrokeStrategy::PainterInfo *m_painterInfo;
    bool m_useLod;
};

void FreehandStrokeTest::testAutobrushStroke()
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

void FreehandStrokeTest::testAutobrushStrokeLod()
{
    FreehandStrokeTester tester("autobrush_300px.kpp", true);
    tester.test();
}

QTEST_KDEMAIN(FreehandStrokeTest, GUI)
#include "freehand_stroke_test.moc"
