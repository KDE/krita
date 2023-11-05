/*
 *  SPDX-FileCopyrightText: 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "test_animated_transform_parameters.h"

#include "kis_transform_mask.h"
#include <testutil.h>
#include "kistest.h"
#include "tool_transform_args.h"
#include "commands_new/KisSimpleModifyTransformMaskCommand.h"
#include "commands_new/KisLazyCreateTransformMaskKeyframesCommand.h"
#include "kis_image_animation_interface.h"
#include "kis_transform_mask_params_interface.h"
#include "KisAnimatedTransformMaskParamsHolder.h"
#include "kis_keyframe_channel.h"

#include <KoToolRegistry.h>

void KisAnimatedTransformParametersTest::initTestCase()
{
    KoToolRegistry::instance();
}

QSharedPointer<KisTransformMaskAdapter> adapterFromParams(KisTransformMaskParamsInterfaceSP params)
{
    return params.dynamicCast<KisTransformMaskAdapter>();
}

ToolTransformArgs argsFromParams(KisTransformMaskParamsInterfaceSP params) {
    return *adapterFromParams(params)->transformArgs();
}

void KisAnimatedTransformParametersTest::testTransformKeyframing()
{
    TestUtil::MaskParent p;
    KisTransformMaskSP mask = new KisTransformMask(p.image, "mask");
    p.image->addNode(mask, p.layer);

    // Make mask animated
    QList<KoID> ids = {
        KisKeyframeChannel::PositionX,
        KisKeyframeChannel::PositionY,
        KisKeyframeChannel::ScaleX,
        KisKeyframeChannel::ScaleY,
        KisKeyframeChannel::ShearX,
        KisKeyframeChannel::ShearY,
        KisKeyframeChannel::RotationX,
        KisKeyframeChannel::RotationY,
        KisKeyframeChannel::RotationZ
    };

    Q_FOREACH( const KoID& koid, ids ) {
        mask->getKeyframeChannel(koid.id(), true);
        QVERIFY(mask->getKeyframeChannel(koid.id(), false));
    }

    QVERIFY(!adapterFromParams(mask->transformParams())->isInitialized());

    ToolTransformArgs args;

    KUndo2Command firstFrameCommand;
    KUndo2Command secondFrameCommand;

    {
        p.image->animationInterface()->switchCurrentTimeAsync(0);
        p.image->waitForDone();

        args.setMode(ToolTransformArgs::FREE_TRANSFORM);
        args.setScaleX(0.75);

        new KisLazyCreateTransformMaskKeyframesCommand(mask, &firstFrameCommand);
        new KisSimpleModifyTransformMaskCommand(mask, toQShared(new KisTransformMaskAdapter(args)), {}, &firstFrameCommand);
        firstFrameCommand.redo();

        QVERIFY(adapterFromParams(mask->transformParams())->isInitialized());
        QCOMPARE(argsFromParams(mask->transformParams()).scaleX(), 0.75);
    }

    {
        p.image->animationInterface()->switchCurrentTimeAsync(10);
        p.image->waitForDone();

        args.setScaleX(0.5);

        new KisLazyCreateTransformMaskKeyframesCommand(mask, &secondFrameCommand);
        new KisSimpleModifyTransformMaskCommand(mask, toQShared(new KisTransformMaskAdapter(args)), {}, &secondFrameCommand);
        secondFrameCommand.redo();

        QVERIFY(adapterFromParams(mask->transformParams())->isInitialized());
        QCOMPARE(argsFromParams(mask->transformParams()).scaleX(), 0.5);
    }

    p.image->animationInterface()->switchCurrentTimeAsync(0);
    p.image->waitForDone();
    QVERIFY(p.image->animationInterface()->currentTime() == 0);
    QCOMPARE(argsFromParams(mask->transformParams()).scaleX(), 0.75);

    p.image->animationInterface()->switchCurrentTimeAsync(10);
    p.image->waitForDone();
    QVERIFY(p.image->animationInterface()->currentTime() == 10);
    QCOMPARE(argsFromParams(mask->transformParams()).scaleX(), 0.5);

    secondFrameCommand.undo();
    QVERIFY(p.image->animationInterface()->currentTime() == 10);
    QCOMPARE(argsFromParams(mask->transformParams()).scaleX(), 0.75);

    firstFrameCommand.undo();
    QVERIFY(p.image->animationInterface()->currentTime() == 10);
    QCOMPARE(argsFromParams(mask->transformParams()).scaleX(), 1.0);

    mask->forceUpdateTimedNode();
    p.image->waitForDone();
}


KISTEST_MAIN(KisAnimatedTransformParametersTest)
