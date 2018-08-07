/*
 *  Copyright (c) 2016 Jouni Pentikäinen <joupent@gmail.com>
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

#include "test_animated_transform_parameters.h"

#include "kis_transform_mask.h"
#include "testutil.h"
#include "tool_transform_args.h"
#include "kis_modify_transform_mask_command.h"
#include "kis_image_animation_interface.h"
#include "kis_transform_mask_params_interface.h"
#include "kis_animated_transform_parameters.h"
#include "kis_keyframe_channel.h"

void KisAnimatedTransformParametersTest::testTransformKeyframing()
{
    TestUtil::MaskParent p;
    KisTransformMaskSP mask = new KisTransformMask();
    p.image->addNode(mask, p.layer);


    ToolTransformArgs args;
    mask->setTransformParams(toQShared(new KisTransformMaskAdapter(args)));

    // Make mask animated
    mask->getKeyframeChannel(KisKeyframeChannel::TransformArguments.id(), true);

    args.setMode(ToolTransformArgs::FREE_TRANSFORM);
    args.setScaleX(0.75);
    QScopedPointer<KisModifyTransformMaskCommand> command1(
        new KisModifyTransformMaskCommand(mask, toQShared(new KisTransformMaskAdapter(args))));
    command1->redo();

    p.image->animationInterface()->switchCurrentTimeAsync(10);
    p.image->waitForDone();

    args.setScaleX(0.5);
    QScopedPointer<KisModifyTransformMaskCommand> command2(
        new KisModifyTransformMaskCommand(mask, toQShared(new KisTransformMaskAdapter(args))));
    command2->redo();

    KisAnimatedTransformMaskParameters *params_out = 0;

    params_out = dynamic_cast<KisAnimatedTransformMaskParameters*>(mask->transformParams().data());
    QVERIFY(params_out != 0);
    QCOMPARE(params_out->transformArgs().scaleX(), 0.5);

    p.image->animationInterface()->switchCurrentTimeAsync(0);
    p.image->waitForDone();
    params_out = dynamic_cast<KisAnimatedTransformMaskParameters*>(mask->transformParams().data());
    QVERIFY(params_out != 0);
    QCOMPARE(params_out->transformArgs().scaleX(), 0.75);

}


QTEST_MAIN(KisAnimatedTransformParametersTest)
