/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "kis_animation_utils_test.h"

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_image_animation_interface.h"
#include "kis_paint_layer.h"
#include "kis_keyframe_channel.h"
#include <testutil.h>
#include "KisAnimUtils.h"

#include <tuple>


bool verifyFrames(TestUtil::MaskParent &p,
                  const QVector<KisNodeSP> &nodes,
                  const QVector<std::tuple<int, QRect, QRect>> &offsets)
{
    KisImageAnimationInterface *i = p.image->animationInterface();

    Q_FOREACH (const auto &offset, offsets) {
        int time = 0;
        QRect rc1;
        QRect rc2;
        std::tie(time, rc1, rc2) = offset;

        i->switchCurrentTimeAsync(time);
        p.image->waitForDone();

        KIS_SAFE_ASSERT_RECOVER_NOOP(nodes[0]->paintDevice()->defaultBounds()->currentTime() == time);
        KIS_SAFE_ASSERT_RECOVER_NOOP(nodes[1]->paintDevice()->defaultBounds()->currentTime() == time);

        KisKeyframeChannel *channel1 = nodes[0]->getKeyframeChannel("content");
        KisKeyframeChannel *channel2 = nodes[1]->getKeyframeChannel("content");


        if (!rc1.isValid() && !channel1->keyframeAt(time)) {
            // noop
        } else if (nodes[0]->paintDevice()->exactBounds() != rc1) {
            qWarning() << "Compared values are not the same:";
            qWarning() << "    " << ppVar(nodes[0]->paintDevice()->exactBounds());
            qWarning() << "    " << ppVar(rc1);
            qWarning() << "    " << ppVar(time);
            return false;
        }

        if (!rc2.isValid() && !channel2->keyframeAt(time)) {
            // noop
        } else if (nodes[1]->paintDevice()->exactBounds() != rc2) {
            qWarning() << "Compared values are not the same:";
            qWarning() << "    "  << ppVar(nodes[1]->paintDevice()->exactBounds());
            qWarning() << "    "  << ppVar(rc2);
            qWarning() << "    " << ppVar(time);
            return false;
        }
    }

    return true;
}


void KisAnimationUtilsTest::test()
{
    QRect refRect(QRect(0,0,512,512));
    TestUtil::MaskParent p(refRect);

    const KoColor fillColor(Qt::red, p.image->colorSpace());

    KisPaintLayerSP layer1 = p.layer;
    KisPaintLayerSP layer2 = new KisPaintLayer(p.image, "paint2", OPACITY_OPAQUE_U8);
    p.image->addNode(layer2);

    QVector<KisNodeSP> nodes({layer1, layer2});

    KisPaintDeviceSP dev1 = layer1->paintDevice();
    KisPaintDeviceSP dev2 = layer2->paintDevice();

    KisKeyframeChannel *channel1 = layer1->getKeyframeChannel(KisKeyframeChannel::Raster.id(), true);
    KisKeyframeChannel *channel2 = layer2->getKeyframeChannel(KisKeyframeChannel::Raster.id(), true);

    channel1->addKeyframe(0);
    channel2->addKeyframe(0);
    channel1->addKeyframe(10);
    channel2->addKeyframe(10);
    channel1->addKeyframe(20);
    channel2->addKeyframe(20);

    KisImageAnimationInterface *i = p.image->animationInterface();

    i->switchCurrentTimeAsync(0);
    p.image->waitForDone();

    dev1->fill(QRect(0, 0, 10, 10), fillColor);
    dev2->fill(QRect(0, 10, 10, 10), fillColor);

    i->switchCurrentTimeAsync(10);
    p.image->waitForDone();

    dev1->fill(QRect(10, 0, 10, 10), fillColor);
    dev2->fill(QRect(10, 10, 10, 10), fillColor);

    i->switchCurrentTimeAsync(20);
    p.image->waitForDone();

    dev1->fill(QRect(20, 0, 10, 10), fillColor);
    dev2->fill(QRect(20, 10, 10, 10), fillColor);


    QVector<std::tuple<int, QRect, QRect>> initialReferenceRects;
    initialReferenceRects << std::make_tuple( 0, QRect( 0, 0, 10, 10), QRect( 0, 10, 10, 10));
    initialReferenceRects << std::make_tuple(10, QRect(10, 0, 10, 10), QRect(10, 10, 10, 10));
    initialReferenceRects << std::make_tuple(20, QRect(20, 0, 10, 10), QRect(20, 10, 10, 10));

    QVERIFY(verifyFrames(p, nodes, initialReferenceRects));

    using namespace KisAnimUtils;

    FrameMovePairList frameMoves;

    //
    // Cycling single-layer move
    //
    frameMoves << std::make_pair(FrameItem(layer1, "content", 0), FrameItem(layer1, "content", 10));
    frameMoves << std::make_pair(FrameItem(layer1, "content", 10), FrameItem(layer1, "content", 20));
    frameMoves << std::make_pair(FrameItem(layer1, "content", 20), FrameItem(layer1, "content", 0));

    QScopedPointer<KUndo2Command> cmd(createMoveKeyframesCommand(frameMoves, false, false));
    cmd->redo();

    QVector<std::tuple<int, QRect, QRect>> referenceRects;
    referenceRects << std::make_tuple( 0, QRect(20, 0, 10, 10), QRect( 0, 10, 10, 10));
    referenceRects << std::make_tuple(10, QRect( 0, 0, 10, 10), QRect(10, 10, 10, 10));
    referenceRects << std::make_tuple(20, QRect(10, 0, 10, 10), QRect(20, 10, 10, 10));
    QVERIFY(verifyFrames(p, nodes, referenceRects));

    cmd->undo();

    QVERIFY(verifyFrames(p, nodes, initialReferenceRects));

    frameMoves.clear();
    referenceRects.clear();
    cmd.reset();

    //
    // Just a complex non-cycling move
    //

    frameMoves << std::make_pair(FrameItem(layer1, "content", 0), FrameItem(layer2, "content", 10));
    frameMoves << std::make_pair(FrameItem(layer2, "content", 10), FrameItem(layer1, "content", 10));
    frameMoves << std::make_pair(FrameItem(layer1, "content", 20), FrameItem(layer2, "content", 20));

    cmd.reset(createMoveKeyframesCommand(frameMoves, false, false));
    cmd->redo();

    referenceRects << std::make_tuple( 0, QRect()              , QRect( 0, 10, 10, 10));
    referenceRects << std::make_tuple(10, QRect(10, 10, 10, 10), QRect( 0,  0, 10, 10));
    referenceRects << std::make_tuple(20, QRect()              , QRect(20,  0, 10, 10));
    QVERIFY(verifyFrames(p, nodes, referenceRects));

    cmd->undo();

    QVERIFY(verifyFrames(p, nodes, initialReferenceRects));

    frameMoves.clear();
    referenceRects.clear();
    cmd.reset();

    //
    // Cross-node swap of the frames
    //
    frameMoves << std::make_pair(FrameItem(layer1, "content", 0), FrameItem(layer2, "content", 0));
    frameMoves << std::make_pair(FrameItem(layer1, "content", 10), FrameItem(layer2, "content", 10));
    frameMoves << std::make_pair(FrameItem(layer1, "content", 20), FrameItem(layer2, "content", 20));
    frameMoves << std::make_pair(FrameItem(layer2, "content", 0), FrameItem(layer1, "content", 0));
    frameMoves << std::make_pair(FrameItem(layer2, "content", 10), FrameItem(layer1, "content", 10));
    frameMoves << std::make_pair(FrameItem(layer2, "content", 20), FrameItem(layer1, "content", 20));

    cmd.reset(createMoveKeyframesCommand(frameMoves, false, false));
    cmd->redo();

    referenceRects << std::make_tuple( 0, QRect( 0, 10, 10, 10), QRect( 0, 0, 10, 10));
    referenceRects << std::make_tuple(10, QRect(10, 10, 10, 10), QRect(10, 0, 10, 10));
    referenceRects << std::make_tuple(20, QRect(20, 10, 10, 10), QRect(20, 0, 10, 10));
    QVERIFY(verifyFrames(p, nodes, referenceRects));

    cmd->undo();

    QVERIFY(verifyFrames(p, nodes, initialReferenceRects));

    frameMoves.clear();
    referenceRects.clear();
    cmd.reset();

    //
    // Cross-node move and swap
    //
    frameMoves << std::make_pair(FrameItem(layer1, "content", 0), FrameItem(layer2, "content", 0));
    frameMoves << std::make_pair(FrameItem(layer1, "content", 10), FrameItem(layer2, "content", 10));
    frameMoves << std::make_pair(FrameItem(layer1, "content", 20), FrameItem(layer2, "content", 20));

    frameMoves << std::make_pair(FrameItem(layer2, "content", 0), FrameItem(layer1, "content", 0));
    frameMoves << std::make_pair(FrameItem(layer2, "content", 10), FrameItem(layer1, "content",  9));
    frameMoves << std::make_pair(FrameItem(layer2, "content", 20), FrameItem(layer1, "content", 20));

    cmd.reset(createMoveKeyframesCommand(frameMoves, false, false));
    cmd->redo();

    referenceRects << std::make_tuple( 0, QRect( 0, 10, 10, 10), QRect( 0, 0, 10, 10));
    referenceRects << std::make_tuple( 9, QRect(10, 10, 10, 10), QRect()             );
    referenceRects << std::make_tuple(10, QRect()              , QRect(10, 0, 10, 10));
    referenceRects << std::make_tuple(20, QRect(20, 10, 10, 10), QRect(20, 0, 10, 10));
    QVERIFY(verifyFrames(p, nodes, referenceRects));

    cmd->undo();

    QVERIFY(verifyFrames(p, nodes, initialReferenceRects));

    frameMoves.clear();
    referenceRects.clear();
    cmd.reset();



}

SIMPLE_TEST_MAIN(KisAnimationUtilsTest)
