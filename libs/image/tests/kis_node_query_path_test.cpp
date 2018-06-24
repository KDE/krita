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

#include "kis_node_query_path_test.h"

#include <QTest>

#include "kis_node_query_path.h"
#include <kis_node.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <KoColorSpaceRegistry.h>

KisNodeQueryPathTest::KisNodeQueryPathTest()
{
    image = new KisImage(0, 100, 100, KoColorSpaceRegistry::instance()->rgb8(), "");
    current = new KisGroupLayer(image, "", OPACITY_OPAQUE_U8);
    child1 = new KisPaintLayer(image, "", OPACITY_OPAQUE_U8);
    child2 = new KisPaintLayer(image, "", OPACITY_OPAQUE_U8);
    image->addNode(child1, current);
    image->addNode(child2, current);
    parent = new KisGroupLayer(image, "", OPACITY_OPAQUE_U8);
    brother1 = new KisPaintLayer(image, "", OPACITY_OPAQUE_U8);
    brother2 = new KisPaintLayer(image, "", OPACITY_OPAQUE_U8);
    image->addNode(brother1, parent);
    image->addNode(current, parent);
    image->addNode(brother2, parent);
    image->addNode(parent, image->rootLayer());
}

#define TESTS(name, relativeStr, absoluteStr1, absoluteStr2, node)              \
    void KisNodeQueryPathTest::test##name##LayerFromRelativeString()            \
    {                                                                           \
        KisNodeQueryPath path = KisNodeQueryPath::fromString(relativeStr);      \
        QCOMPARE(path.toString(), QString(relativeStr));                        \
        QList<KisNodeSP> nodes = path.queryNodes(image, current);               \
        QCOMPARE(nodes.size(), 1);                                              \
        QCOMPARE(nodes[0], node);                                               \
    }                                                                           \
                                                                                \
    void KisNodeQueryPathTest::test##name##LayerFromAbsoluteString()            \
    {                                                                           \
        {                                                                       \
            KisNodeQueryPath path = KisNodeQueryPath::fromString(absoluteStr1); \
            QCOMPARE(path.toString(), QString(absoluteStr1));                   \
            QList<KisNodeSP> nodes = path.queryNodes(image, current);           \
            QCOMPARE(nodes.size(), 1);                                          \
            QCOMPARE(nodes[0], node);                                           \
        }                                                                       \
        {                                                                       \
            KisNodeQueryPath path = KisNodeQueryPath::fromString(absoluteStr2); \
            QCOMPARE(path.toString(), QString(absoluteStr2));                   \
            QList<KisNodeSP> nodes = path.queryNodes(image, current);           \
            QCOMPARE(nodes.size(), 1);                                          \
            QCOMPARE(nodes[0], node);                                           \
        }                                                                       \
    }                                                                           \
                                                                                \
    void KisNodeQueryPathTest::test##name##LayerFromAbsolutePath()              \
    {                                                                           \
        KisNodeQueryPath path = KisNodeQueryPath::absolutePath(node);           \
        QCOMPARE(path.toString(), QString(absoluteStr1));                       \
        QList<KisNodeSP> nodes = path.queryNodes(image, current);               \
        QCOMPARE(nodes.size(), 1);                                              \
        QCOMPARE(nodes[0], node);                                               \
    }

TESTS(Current, ".", "/0/1", "/*/1", current)
TESTS(Child1, "0", "/0/1/0", "/*/1/0", child1)
TESTS(Child2, "1", "/0/1/1", "/*/1/1", child2)
TESTS(Brother1, "../0", "/0/0", "/*/0", brother1)
TESTS(Brother2, "../2", "/0/2", "/*/2", brother2)
TESTS(Parent, "..", "/0", "/*", parent)
TESTS(Root, "../..", "/", "/", KisNodeSP(image->rootLayer()))

void KisNodeQueryPathTest::testPathCompression()
{
    KisNodeQueryPath path = KisNodeQueryPath::fromString("1/../3/../5");
    QCOMPARE(path.toString(), QString("5"));
    KisNodeQueryPath path2 = KisNodeQueryPath::fromString("/*/..");
    QCOMPARE(path2.toString(), QString("/"));
}

QTEST_MAIN(KisNodeQueryPathTest)
