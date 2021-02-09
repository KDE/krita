/*
 *  SPDX-FileCopyrightText: 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "test_animated_transform_parameters.h"

#include "kis_transform_mask.h"
#include <testutil.h>
#include "tool_transform_args.h"
#include "kis_modify_transform_mask_command.h"
#include "kis_image_animation_interface.h"
#include "kis_transform_mask_params_interface.h"
#include "kis_animated_transform_parameters.h"
#include "kis_keyframe_channel.h"

void KisAnimatedTransformParametersTest::testTransformKeyframing()
{
    TestUtil::MaskParent p;
    KisTransformMaskSP mask = new KisTransformMask(p.image, "mask");
    p.image->addNode(mask, p.layer);


    ToolTransformArgs args;
    mask->setTransformParams(toQShared(new KisTransformMaskAdapter(args)));

    // Make mask animated
    mask->getKeyframeChannel(KisKeyframeChannel::ScaleX.id(), true);
    QVERIFY(mask->getKeyframeChannel(KisKeyframeChannel::ScaleX.id(), false));

    {
        p.image->animationInterface()->switchCurrentTimeAsync(0);
        p.image->waitForDone();

        args.setMode(ToolTransformArgs::FREE_TRANSFORM);
        args.setScaleX(0.75);

        QScopedPointer<KisModifyTransformMaskCommand> command1(
            new KisModifyTransformMaskCommand(mask, toQShared(new KisTransformMaskAdapter(args))));
        command1->redo();
    }

    {
        p.image->animationInterface()->switchCurrentTimeAsync(10);
        p.image->waitForDone();

        args.setScaleX(0.5);

        QScopedPointer<KisModifyTransformMaskCommand> command2(
            new KisModifyTransformMaskCommand(mask, toQShared(new KisTransformMaskAdapter(args))));
        command2->redo();
    }

    KisAnimatedTransformMaskParameters *params_out = 0;

    params_out = dynamic_cast<KisAnimatedTransformMaskParameters*>(mask->transformParams().data());
    QVERIFY(params_out != 0);
    QCOMPARE(params_out->transformArgs()->scaleX(), 0.5);

    p.image->animationInterface()->switchCurrentTimeAsync(0);
    p.image->waitForDone();

    QVERIFY(p.image->animationInterface()->currentTime() == 0);

    params_out = dynamic_cast<KisAnimatedTransformMaskParameters*>(mask->transformParams().data());
    QVERIFY(params_out != 0);
    QCOMPARE(params_out->transformArgs()->scaleX(), 0.75);
}


QTEST_MAIN(KisAnimatedTransformParametersTest)
