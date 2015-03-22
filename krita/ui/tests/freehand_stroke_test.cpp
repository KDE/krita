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
#include <KoColor.h>
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
          m_useLod(useLod),
          m_flipLineDirection(false)
    {
    }

    void setFlipLineDirection(bool value) {
        m_flipLineDirection = value;
        setNumIterations(2);
    }

    void setPaintColor(const QColor &color) {
        m_paintColor.reset(new QColor(color));
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

    void modifyResourceManager(KoCanvasResourceManager *manager,
                               KisImageWSP image)
    {
        modifyResourceManager(manager, image, 0);
    }


    void modifyResourceManager(KoCanvasResourceManager *manager,
                               KisImageWSP image,
                               int iteration) {

        if (m_paintColor && iteration > 0) {
            QVariant i;
            i.setValue(KoColor(*m_paintColor, image->colorSpace()));
            manager->setResource(KoCanvasResourceManager::ForegroundColor, i);
        }
    }

    KisStrokeStrategy* createStroke(bool indirectPainting,
                                    KisResourcesSnapshotSP resources,
                                    KisImageWSP image) {
        Q_UNUSED(image);

        FreehandStrokeStrategy::PainterInfo *painterInfo =
            new FreehandStrokeStrategy::PainterInfo();

        QScopedPointer<FreehandStrokeStrategy> stroke(
            new FreehandStrokeStrategy(indirectPainting, COMPOSITE_ALPHA_DARKEN, resources, painterInfo, kundo2_noi18n("Freehand Stroke")));

        return m_useLod ? stroke->createLodClone(1) : stroke.take();
    }

    virtual void addPaintingJobs(KisImageWSP image,
                                 KisResourcesSnapshotSP resources,
                                 KisPainter *painter)
    {
        addPaintingJobs(image, resources, painter, 0);
    }

    void addPaintingJobs(KisImageWSP image, KisResourcesSnapshotSP resources, KisPainter *painter, int iteration) {

        Q_ASSERT(painter == m_painterInfo->painter);
        Q_UNUSED(painter);

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
            new FreehandStrokeStrategy::Data(resources->currentNode(),
                                             m_painterInfo, pi1, pi2));

        image->addJob(strokeId(), m_useLod ? data->createLodClone(1) : data.take());
    }

private:
    FreehandStrokeStrategy::PainterInfo *m_painterInfo;
    bool m_useLod;
    bool m_flipLineDirection;
    QScopedPointer<QColor> m_paintColor;
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

void FreehandStrokeTest::testMixDullCompositioning()
{
    FreehandStrokeTester tester("Mix_dull.kpp");
    tester.setFlipLineDirection(true);
    tester.setPaintColor(Qt::red);
    tester.test();
}

QTEST_KDEMAIN(FreehandStrokeTest, GUI)
#include "freehand_stroke_test.moc"
