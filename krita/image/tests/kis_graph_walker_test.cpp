/*
 *  Copyright (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_graph_walker_test.h"

#include "kis_graph_walker.h"

#include "kis_merge_walkers.h"

#include <qtest_kde.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include "kis_image.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_selection.h"

#define DEBUG_VISITORS

QString nodeTypeString(KisGraphWalker::NodePosition position);
QString nodeTypePostfix(KisGraphWalker::NodePosition position);

/************** Test Implementation Of A Walker *********************/
class KisTestWalker : public KisGraphWalker
{
public:
    QStringList popResult() {
        QStringList order(m_order);
        m_order.clear();
        return order;
    }

protected:

    void registerChangeRect(KisNodeSP node) {
#ifdef DEBUG_VISITORS
        qDebug()<< "FW:"<< node->name();
#endif
        m_order.append(node->name());
    }

    void registerNeedRect(KisNodeSP node, NodePosition position) {
#ifdef DEBUG_VISITORS
        qDebug()<< "BW:"<< node->name() <<'\t'<< nodeTypeString(position);
#endif
        m_order.append(node->name() + nodeTypePostfix(position));
    }

protected:
    QStringList m_order;
};

/************** Debug And Verify Code *******************************/
void reportStartWith(QString nodeName)
{
    qDebug();
    qDebug() << "Start with:" << nodeName;
}

QString nodeTypeString(KisGraphWalker::NodePosition position)
{
    static QString pos("  normal,  lower,top,bottom");
    static QStringList positionName  = pos.split(",");

    return positionName[position];
}

QString nodeTypePostfix(KisGraphWalker::NodePosition position)
{
    static QString index("_N,_L,_T,_B");
    static QStringList indexName  = index.split(",");

    return indexName[position];
}

void KisGraphWalkerTest::verifyResult(KisMergeWalker walker, QStringList reference,
                 QRect accessRect, bool changeRectVaries,
                 bool needRectVaries)
{
    KisMergeWalker::NodeStack &list = walker.nodeStack();
    QStringList::const_iterator iter = reference.constBegin();

    foreach(KisMergeWalker::JobItem item, list) {
#ifdef DEBUG_VISITORS
        qDebug() << item.m_node->name() << '\t'
                 << item.m_applyRect << '\t'
                 << nodeTypeString(item.m_position);
#endif

        QVERIFY(item.m_node->name() == *iter);

        iter++;
    }

#ifdef DEBUG_VISITORS
    qDebug() << "Result AR:\t" << walker.accessRect();
#endif

    QVERIFY(walker.accessRect() == accessRect);
    QVERIFY(walker.changeRectVaries() == changeRectVaries);
    QVERIFY(walker.needRectVaries() == needRectVaries);
}

/************** Actual Testing **************************************/

    /*
      +----------+
      |root      |
      | layer 5  |
      | group    |
      |  paint 4 |
      |  paint 3 |
      |  adj     |
      |  paint 2 |
      | paint 1  |
      +----------+
     */

void KisGraphWalkerTest::testUsualVisiting()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageWSP image = new KisImage(0, 512, 512, colorSpace, "walker test");

    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE);
    KisLayerSP paintLayer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE);
    KisLayerSP paintLayer3 = new KisPaintLayer(image, "paint3", OPACITY_OPAQUE);
    KisLayerSP paintLayer4 = new KisPaintLayer(image, "paint4", OPACITY_OPAQUE);
    KisLayerSP paintLayer5 = new KisPaintLayer(image, "paint5", OPACITY_OPAQUE);

    KisLayerSP groupLayer = new KisGroupLayer(image, "group", OPACITY_OPAQUE);
    KisLayerSP adjustmentLayer = new KisAdjustmentLayer(image, "adj", 0, 0);


    image->addNode(paintLayer1, image->rootLayer());
    image->addNode(groupLayer, image->rootLayer());
    image->addNode(paintLayer5, image->rootLayer());

    image->addNode(paintLayer2, groupLayer);
    image->addNode(adjustmentLayer, groupLayer);
    image->addNode(paintLayer3, groupLayer);
    image->addNode(paintLayer4, groupLayer);

    KisTestWalker walker;

    {
        QString order("paint3,paint4,group,paint5,root,"
                      "root_T,paint5_T,group_N,paint1_B,"
                      "paint4_T,paint3_N,adj_L,paint2_B");
        QStringList orderList = order.split(",");

        reportStartWith("paint3");
        walker.startTrip(paintLayer3);
        QVERIFY(walker.popResult() == orderList);
    }

    {
        QString order("adj,paint3,paint4,group,paint5,root,"
                      "root_T,paint5_T,group_N,paint1_B,"
                      "paint4_T,paint3_N,adj_N,paint2_B");
        QStringList orderList = order.split(",");

        reportStartWith("adj");
        walker.startTrip(adjustmentLayer);
        QVERIFY(walker.popResult() == orderList);
    }

    {
        QString order("group,paint5,root,"
                      "root_T,paint5_T,group_N,paint1_B");
        QStringList orderList = order.split(",");

        reportStartWith("group");
        walker.startTrip(groupLayer);
        QVERIFY(walker.popResult() == orderList);
    }
}

    /*
      +----------+
      |root      |
      | layer 5  |
      | cplx  2  |
      | group    |
      |  paint 4 |
      |  paint 3 |
      |  cplx  1 |
      |  paint 2 |
      | paint 1  |
      +----------+
     */

void KisGraphWalkerTest::testMergeVisiting()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageWSP image = new KisImage(0, 512, 512, colorSpace, "walker test");

    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE);
    KisLayerSP paintLayer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE);
    KisLayerSP paintLayer3 = new KisPaintLayer(image, "paint3", OPACITY_OPAQUE);
    KisLayerSP paintLayer4 = new KisPaintLayer(image, "paint4", OPACITY_OPAQUE);
    KisLayerSP paintLayer5 = new KisPaintLayer(image, "paint5", OPACITY_OPAQUE);

    KisLayerSP groupLayer = new KisGroupLayer(image, "group", OPACITY_OPAQUE);
    KisLayerSP complexRectsLayer1 = new ComplexRectsLayer(image, "cplx1", OPACITY_OPAQUE);
    KisLayerSP complexRectsLayer2 = new ComplexRectsLayer(image, "cplx2", OPACITY_OPAQUE);

    image->addNode(paintLayer1, image->rootLayer());
    image->addNode(groupLayer, image->rootLayer());
    image->addNode(complexRectsLayer2, image->rootLayer());
    image->addNode(paintLayer5, image->rootLayer());

    image->addNode(paintLayer2, groupLayer);
    image->addNode(complexRectsLayer1, groupLayer);
    image->addNode(paintLayer3, groupLayer);
    image->addNode(paintLayer4, groupLayer);

    QRect testRect(10,10,10,10);

    KisMergeWalker walker;

    {
        QString order("root,paint5,cplx2,group,paint1,"
                      "paint4,paint3,cplx1,paint2");
        QStringList orderList = order.split(",");
        QRect accessRect(-7,-7,44,44);

        reportStartWith("paint3");
        walker.collectRects(paintLayer3, testRect);
        verifyResult(walker, orderList, accessRect, true, true);
    }

    {
        QString order("root,paint5,cplx2,group,paint1,"
                      "paint4,paint3,cplx1,paint2");
        QStringList orderList = order.split(",");
        QRect accessRect(-10,-10,50,50);

        reportStartWith("paint2");
        walker.collectRects(paintLayer2, testRect);
        verifyResult(walker, orderList, accessRect, true, true);
    }

    {
        QString order("root,paint5,cplx2,group,paint1");
        QStringList orderList = order.split(",");
        QRect accessRect(3,3,24,24);

        reportStartWith("paint5");
        walker.collectRects(paintLayer5, testRect);
        verifyResult(walker, orderList, accessRect, false, true);
    }

}


    /*
      +----------+
      |root      |
      | layer 5  |
      | cache1   |
      | group    |
      |  paint 4 |
      |  paint 3 |
      |  paint 2 |
      | paint 1  |
      +----------+
     */

void KisGraphWalkerTest::testCachedVisiting()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageWSP image = new KisImage(0, 512, 512, colorSpace, "walker test");

    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE);
    KisLayerSP paintLayer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE);
    KisLayerSP paintLayer3 = new KisPaintLayer(image, "paint3", OPACITY_OPAQUE);
    KisLayerSP paintLayer4 = new KisPaintLayer(image, "paint4", OPACITY_OPAQUE);
    KisLayerSP paintLayer5 = new KisPaintLayer(image, "paint5", OPACITY_OPAQUE);

    KisLayerSP groupLayer = new KisGroupLayer(image, "group", OPACITY_OPAQUE);
    KisLayerSP cacheLayer1 = new CacheLayer(image, "cache1", OPACITY_OPAQUE);

    image->addNode(paintLayer1, image->rootLayer());
    image->addNode(groupLayer, image->rootLayer());
    image->addNode(cacheLayer1, image->rootLayer());
    image->addNode(paintLayer5, image->rootLayer());

    image->addNode(paintLayer2, groupLayer);
    image->addNode(paintLayer3, groupLayer);
    image->addNode(paintLayer4, groupLayer);

    QRect testRect(10,10,10,10);

    KisMergeWalker walker;

    {
        QString order("root,paint5,cache1,group,paint1,"
                      "paint4,paint3,paint2");
        QStringList orderList = order.split(",");
        QRect accessRect(0,0,30,30);

        reportStartWith("paint3");
        walker.collectRects(paintLayer3, testRect);
        verifyResult(walker, orderList, accessRect, true, true);
    }

    {
        QString order("root,paint5,cache1");
        QStringList orderList = order.split(",");
        QRect accessRect(10,10,10,10);

        reportStartWith("paint5");
        walker.collectRects(paintLayer5, testRect);
        verifyResult(walker, orderList, accessRect, false, true);
    }

}

QTEST_KDEMAIN(KisGraphWalkerTest, NoGUI)
#include "kis_graph_walker_test.moc"

