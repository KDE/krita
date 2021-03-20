/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_TRANSFORM_MASK_TEST_H
#define __KIS_TRANSFORM_MASK_TEST_H

#include <simpletest.h>

class KisTransformMaskTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSafeTransform();
    void testMaskOnPaintLayer();
    void testMaskOnCloneLayer();
    void testMaskOnCloneLayerWithOffset();
    void testSafeTransformUnity();
    void testSafeTransformSingleVanishingPoint();

    void testMultipleMasks();
    void testMaskWithOffset();

    void testWeirdFullUpdates();
    void testTransformHiddenPartsOfTheGroup();
};

#endif /* __KIS_TRANSFORM_MASK_TEST_H */
