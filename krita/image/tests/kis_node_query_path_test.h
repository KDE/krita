/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_CUBIC_CURVE_TEST_H_
#define _KIS_CUBIC_CURVE_TEST_H_

#include <QtTest/QtTest>

#include "kis_types.h"

class KisNodeQueryPathTest : public QObject
{
    Q_OBJECT
public:
    KisNodeQueryPathTest();
private slots:

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
