/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_transform_mask_test.h"

#include <qtest_kde.h>
#include "kis_transform_mask.h"
#include "kis_transform_mask_params_interface.h"

#include "testutil.h"


void KisTransformMaskTest::test()
{
    QImage refImage(TestUtil::fetchDataFileLazy("test_transform_quality.png"));
    TestUtil::MaskParent p(refImage.rect());

    p.layer->paintDevice()->convertFromQImage(refImage, 0);

    KisTransformMaskSP mask = new KisTransformMask();

    p.image->addNode(mask, p.layer);

    QTransform transform =
        QTransform::fromTranslate(100.0, 0.0) *
        QTransform::fromScale(2.0, 1.0);

    mask->setTransformParams(KisTransformMaskParamsInterfaceSP(
                                 new KisDumbTransformMaskParams(transform)));

    p.layer->setDirty(QRect(160, 160, 150, 300));
    p.layer->setDirty(QRect(310, 160, 150, 300));
    p.layer->setDirty(QRect(460, 160, 150, 300));
    p.layer->setDirty(QRect(610, 160, 150, 300));
    p.layer->setDirty(QRect(760, 160, 150, 300));

    p.image->waitForDone();

    QImage result = p.layer->projection()->convertToQImage(0);
    TestUtil::checkQImage(result, "transform_mask_test", "partial", "single");

}

QTEST_KDEMAIN(KisTransformMaskTest, GUI)
