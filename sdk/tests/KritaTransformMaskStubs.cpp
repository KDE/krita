/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include "KritaTransformMaskStubs.h"

#include "kis_pointer_utils.h"
#include "kis_transform_mask_params_factory_registry.h"
#include "KisDumbTransformMaskParams.h"
#include "KisDumbAnimatedTransformMaskParamsHolder.h"

namespace TestUtil {
void registerTransformMaskStubs()
{
    KisTransformMaskParamsFactoryRegistry::instance()->setAnimatedParamsHolderFactory(
        [] (KisDefaultBoundsBaseSP bounds) {
            return toQShared(new KisDumbAnimatedTransformMaskParamsHolder(bounds));
        });

    KisTransformMaskParamsFactoryRegistry::instance()->addFactory("dumbparams", &KisDumbTransformMaskParams::fromXML);
}
}
