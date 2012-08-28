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
#include "stroke_testing_utils.h"
#include "strokes/freehand_stroke.h"
#include "kis_resources_snapshot.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_paint_information.h"


class FreehandStrokeTester : public utils::StrokeTester
{
public:
    FreehandStrokeTester(const QString &presetFilename)
        : StrokeTester("freehand", QSize(500, 500), presetFilename)
    {
    }

protected:
    KisStrokeStrategy* createStroke(bool indirectPainting,
                                    KisResourcesSnapshotSP resources,
                                    KisPainter *painter,
                                    KisImageWSP image) {
        Q_UNUSED(image);

        m_painterInfo =
            new FreehandStrokeStrategy::PainterInfo(painter,
                                                    new KisDistanceInformation());

        return new FreehandStrokeStrategy(indirectPainting, resources, m_painterInfo, QLatin1String("Freehand Stroke"));
    }

    void addPaintingJobs(KisImageWSP image, KisResourcesSnapshotSP resources,
                         KisPainter *painter) {

        Q_ASSERT(painter == m_painterInfo->painter);

        KisPaintInformation pi1;
        KisPaintInformation pi2;

        pi1 = KisPaintInformation(QPointF(200, 200));
        pi2 = KisPaintInformation(QPointF(300, 300));

        image->addJob(strokeId(),
            new FreehandStrokeStrategy::Data(resources->currentNode(),
                                             m_painterInfo, pi1, pi2));
    }

private:
    FreehandStrokeStrategy::PainterInfo *m_painterInfo;
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

QTEST_KDEMAIN(FreehandStrokeTest, GUI)
#include "freehand_stroke_test.moc"
