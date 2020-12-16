/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "move_stroke_test.h"

#include <QTest>

#include "stroke_testing_utils.h"
#include "kis_image.h"
#include "kis_node.h"
#include "kis_paint_device.h"
#include "strokes/move_stroke_strategy.h"


class MoveStrokeTester : public utils::StrokeTester
{
public:
    MoveStrokeTester()
        : StrokeTester("move", QSize(512, 512), "")
    {
    }

protected:
    using utils::StrokeTester::initImage;
    void initImage(KisImageWSP image, KisNodeSP activeNode) override {
        Q_UNUSED(image);

        QImage src(QString(FILES_DATA_DIR) + '/' + "carrot.png");
        activeNode->original()->convertFromQImage(src, 0);
    }

    KisStrokeStrategy* createStroke(KisResourcesSnapshotSP resources,
                                    KisImageWSP image) override {

        KisNodeSP node = resources->currentNode();
        return new MoveStrokeStrategy({node}, image.data(), image.data());
    }

    using utils::StrokeTester::addPaintingJobs;
    void addPaintingJobs(KisImageWSP image,
                         KisResourcesSnapshotSP resources) override {

        Q_UNUSED(resources);

        image->
            addJob(strokeId(), new MoveStrokeStrategy::Data(QPoint(100,100)));

        image->
            addJob(strokeId(), new MoveStrokeStrategy::Data(QPoint(50, 50)));

        for (int i = 0; i < 25; i++) {
            image->
                addJob(strokeId(), new MoveStrokeStrategy::Data(QPoint(50+i,50)));
            QTest::qSleep(1);
            image->
                addJob(strokeId(), new MoveStrokeStrategy::Data(QPoint(50+i,50+i)));

            QTest::qSleep(1);
        }
    }

private:

};


void MoveStrokeTest::testMoveStroke()
{
    MoveStrokeTester tester;
    tester.test();
}

QTEST_GUILESS_MAIN(MoveStrokeTest)
