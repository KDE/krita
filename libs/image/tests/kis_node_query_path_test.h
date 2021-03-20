/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_NODE_QUERY_PATH_TEST_H_
#define _KIS_NODE_QUERY_PATH_TEST_H_

#include <simpletest.h>

#include "kis_types.h"
#include "kis_image.h"

class KisNodeQueryPathTest : public QObject
{
    Q_OBJECT
public:
    KisNodeQueryPathTest();
private Q_SLOTS:

    void testCurrentLayerFromRelativeString();
    void testCurrentLayerFromAbsoluteString();
    void testCurrentLayerFromAbsolutePath();
    void testChild1LayerFromRelativeString();
    void testChild1LayerFromAbsoluteString();
    void testChild1LayerFromAbsolutePath();
    void testChild2LayerFromRelativeString();
    void testChild2LayerFromAbsoluteString();
    void testChild2LayerFromAbsolutePath();
    void testBrother1LayerFromRelativeString();
    void testBrother1LayerFromAbsoluteString();
    void testBrother1LayerFromAbsolutePath();
    void testBrother2LayerFromRelativeString();
    void testBrother2LayerFromAbsoluteString();
    void testBrother2LayerFromAbsolutePath();
    void testParentLayerFromRelativeString();
    void testParentLayerFromAbsoluteString();
    void testParentLayerFromAbsolutePath();
    void testRootLayerFromRelativeString();
    void testRootLayerFromAbsoluteString();
    void testRootLayerFromAbsolutePath();
    void testPathCompression();
private:
    KisImageSP image;
    KisNodeSP current;
    KisNodeSP parent;
    KisNodeSP child1, child2;
    KisNodeSP brother1, brother2;
};


#endif
