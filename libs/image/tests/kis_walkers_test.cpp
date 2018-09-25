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

#include "kis_walkers_test.h"

#include "kis_base_rects_walker.h"
#include "kis_refresh_subtree_walker.h"
#include "kis_full_refresh_walker.h"

#include <QTest>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_clone_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_selection.h"

#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "kis_filter_mask.h"
#include "kis_transparency_mask.h"

//#define DEBUG_VISITORS

QString nodeTypeString(KisMergeWalker::NodePosition position);
QString nodeTypePostfix(KisMergeWalker::NodePosition position);

/************** Test Implementation Of A Walker *********************/
class KisTestWalker : public KisMergeWalker
{
public:
    KisTestWalker()
        :KisMergeWalker(QRect())
    {
    }

    QStringList popResult() {
        QStringList order(m_order);
        m_order.clear();
        return order;
    }

    using KisMergeWalker::startTrip;

protected:

    void registerChangeRect(KisProjectionLeafSP node, NodePosition position) override {
        Q_UNUSED(position);

        QString postfix;

        if(!node->isLayer()) {
            postfix = "[skipped as not-a-layer]";
        }

#ifdef DEBUG_VISITORS
        qDebug()<< "FW:"<< node->node()->name() <<'\t'<< nodeTypeString(position) << postfix;
#endif

        if(postfix.isEmpty()) {
            m_order.append(node->node()->name());
        }
    }

    void registerNeedRect(KisProjectionLeafSP node, NodePosition position) override {
        QString postfix;

        if(!node->isLayer()) {
            postfix = "[skipped as not-a-layer]";
        }

#ifdef DEBUG_VISITORS
        qDebug()<< "BW:"<< node->node()->name() <<'\t'<< nodeTypeString(position) << postfix;
#endif
        if(postfix.isEmpty()) {
            m_order.append(node->node()->name() + nodeTypePostfix(position));
        }
    }

protected:
    QStringList m_order;
};

/************** Debug And Verify Code *******************************/

struct UpdateTestJob {
    QString updateAreaName;
    KisNodeSP startNode;
    QRect updateRect;

    QString referenceString;
    QRect accessRect;
    bool changeRectVaries;
    bool needRectVaries;
};

void reportStartWith(QString nodeName, QRect rect = QRect())
{
    qDebug();
    if(!rect.isEmpty())
        qDebug() << "Start with:" << nodeName << rect;
    else
        qDebug() << "Start with:" << nodeName;
}

QString nodeTypeString(KisMergeWalker::NodePosition position)
{
    QString string;

    if(position & KisMergeWalker::N_TOPMOST)
        string="TOP";
    else if(position & KisMergeWalker::N_BOTTOMMOST)
        string="BOT";
    else
        string="NOR";

    if(position & KisMergeWalker::N_ABOVE_FILTHY)
        string+="_ABOVE ";
    else if(position & KisMergeWalker::N_FILTHY)
        string+="_FILTH*";
    else if(position & KisMergeWalker::N_FILTHY_PROJECTION)
        string+="_PROJE*";
    else if(position & KisMergeWalker::N_FILTHY_ORIGINAL)
        string+="_ORIGI*_WARNINIG!!!: NOT USED";
    else if(position & KisMergeWalker::N_BELOW_FILTHY)
        string+="_BELOW ";
    else if(position & KisMergeWalker::N_EXTRA)
        string+="_EXTRA*";
    else
        qFatal("Impossible happened");

    return string;
}

QString nodeTypePostfix(KisMergeWalker::NodePosition position)
{
    QString string('_');

    if(position & KisMergeWalker::N_TOPMOST)
        string += 'T';
    else if(position & KisMergeWalker::N_BOTTOMMOST)
        string += 'B';
    else
        string += 'N';

    if(position & KisMergeWalker::N_ABOVE_FILTHY)
        string += 'A';
    else if(position & KisMergeWalker::N_FILTHY)
        string += 'F';
    else if(position & KisMergeWalker::N_FILTHY_PROJECTION)
        string += 'P';
    else if(position & KisMergeWalker::N_FILTHY_ORIGINAL)
        string += 'O';
    else if(position & KisMergeWalker::N_BELOW_FILTHY)
        string += 'B';
    else if(position & KisMergeWalker::N_EXTRA)
        string += 'E';
    else
        qFatal("Impossible happened");

    return string;
}

void KisWalkersTest::verifyResult(KisBaseRectsWalker &walker, struct UpdateTestJob &job)
{
    QStringList list;
    if(!job.referenceString.isEmpty()) {
        list = job.referenceString.split(',');
    }

    verifyResult(walker, list, job.accessRect,
                 job.changeRectVaries, job.needRectVaries);
}

void KisWalkersTest::verifyResult(KisBaseRectsWalker &walker, QStringList reference,
                                  QRect accessRect, bool changeRectVaries,
                                  bool needRectVaries)
{
    KisMergeWalker::LeafStack &list = walker.leafStack();
    QStringList::const_iterator iter = reference.constBegin();

    if(reference.size() != list.size()) {
        qDebug() << "*** Seems like the walker returned stack of wrong size"
                 << "( ref:" << reference.size() << "act:" << list.size() << ")";
        qDebug() << "*** We are going to crash soon... just wait...";
    }

    Q_FOREACH (const KisMergeWalker::JobItem &item, list) {
#ifdef DEBUG_VISITORS
        qDebug() << item.m_leaf->node()->name() << '\t'
                 << item.m_applyRect << '\t'
                 << nodeTypeString(item.m_position);
#endif

        QCOMPARE(item.m_leaf->node()->name(), *iter);

        iter++;
    }

#ifdef DEBUG_VISITORS
    qDebug() << "Result AR:\t" << walker.accessRect();
#endif

    QCOMPARE(walker.accessRect(), accessRect);
    QCOMPARE(walker.changeRectVaries(), changeRectVaries);
    QCOMPARE(walker.needRectVaries(), needRectVaries);
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

void KisWalkersTest::testUsualVisiting()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "walker test");

    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer3 = new KisPaintLayer(image, "paint3", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer4 = new KisPaintLayer(image, "paint4", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer5 = new KisPaintLayer(image, "paint5", OPACITY_OPAQUE_U8);

    KisLayerSP groupLayer = new KisGroupLayer(image, "group", OPACITY_OPAQUE_U8);
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
                      "root_TF,paint5_TA,group_NF,paint1_BB,"
                      "paint4_TA,paint3_NF,adj_NB,paint2_BB");
        QStringList orderList = order.split(',');

        reportStartWith("paint3");
        walker.startTrip(paintLayer3->projectionLeaf());
        QVERIFY(walker.popResult() == orderList);
    }

    {
        QString order("adj,paint3,paint4,group,paint5,root,"
                      "root_TF,paint5_TA,group_NF,paint1_BB,"
                      "paint4_TA,paint3_NA,adj_NF,paint2_BB");
        QStringList orderList = order.split(',');

        reportStartWith("adj");
        walker.startTrip(adjustmentLayer->projectionLeaf());
        QVERIFY(walker.popResult() == orderList);
    }

    {
        QString order("group,paint5,root,"
                      "root_TF,paint5_TA,group_NF,paint1_BB");
        QStringList orderList = order.split(',');

        reportStartWith("group");
        walker.startTrip(groupLayer->projectionLeaf());
        QVERIFY(walker.popResult() == orderList);
    }
}

    /*
      +----------+
      |root      |
      | layer 5  |
      | group    |
      |  mask  1 |
      |  paint 4 |
      |  paint 3 |
      |  adj     |
      |  paint 2 |
      | paint 1  |
      +----------+
     */

void KisWalkersTest::testVisitingWithTopmostMask()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "walker test");

    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer3 = new KisPaintLayer(image, "paint3", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer4 = new KisPaintLayer(image, "paint4", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer5 = new KisPaintLayer(image, "paint5", OPACITY_OPAQUE_U8);

    KisLayerSP groupLayer = new KisGroupLayer(image, "group", OPACITY_OPAQUE_U8);
    KisLayerSP adjustmentLayer = new KisAdjustmentLayer(image, "adj", 0, 0);


    KisFilterMaskSP filterMask1 = new KisFilterMask();
    filterMask1->initSelection(groupLayer);
    KisFilterSP filter = KisFilterRegistry::instance()->value("blur");
    Q_ASSERT(filter);
    KisFilterConfigurationSP configuration1 = filter->defaultConfiguration();
    filterMask1->setFilter(configuration1);

    image->addNode(paintLayer1, image->rootLayer());
    image->addNode(groupLayer, image->rootLayer());
    image->addNode(paintLayer5, image->rootLayer());

    image->addNode(paintLayer2, groupLayer);
    image->addNode(adjustmentLayer, groupLayer);
    image->addNode(paintLayer3, groupLayer);
    image->addNode(paintLayer4, groupLayer);

    // nasty mask!
    image->addNode(filterMask1, groupLayer);

    /**
     * The results must be the same as for testUsualVisiting
     */

    KisTestWalker walker;

    {
        QString order("paint3,paint4,group,paint5,root,"
                      "root_TF,paint5_TA,group_NF,paint1_BB,"
                      "paint4_TA,paint3_NF,adj_NB,paint2_BB");
        QStringList orderList = order.split(',');

        reportStartWith("paint3");
        walker.startTrip(paintLayer3->projectionLeaf());
        QVERIFY(walker.popResult() == orderList);
    }

    {
        QString order("adj,paint3,paint4,group,paint5,root,"
                      "root_TF,paint5_TA,group_NF,paint1_BB,"
                      "paint4_TA,paint3_NA,adj_NF,paint2_BB");
        QStringList orderList = order.split(',');

        reportStartWith("adj");
        walker.startTrip(adjustmentLayer->projectionLeaf());
        QVERIFY(walker.popResult() == orderList);
    }

    {
        QString order("group,paint5,root,"
                      "root_TF,paint5_TA,group_NF,paint1_BB");
        QStringList orderList = order.split(',');

        reportStartWith("group");
        walker.startTrip(groupLayer->projectionLeaf());
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

void KisWalkersTest::testMergeVisiting()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "walker test");

    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer3 = new KisPaintLayer(image, "paint3", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer4 = new KisPaintLayer(image, "paint4", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer5 = new KisPaintLayer(image, "paint5", OPACITY_OPAQUE_U8);

    KisLayerSP groupLayer = new KisGroupLayer(image, "group", OPACITY_OPAQUE_U8);
    KisLayerSP complexRectsLayer1 = new ComplexRectsLayer(image, "cplx1", OPACITY_OPAQUE_U8);
    KisLayerSP complexRectsLayer2 = new ComplexRectsLayer(image, "cplx2", OPACITY_OPAQUE_U8);

    image->addNode(paintLayer1, image->rootLayer());
    image->addNode(groupLayer, image->rootLayer());
    image->addNode(complexRectsLayer2, image->rootLayer());
    image->addNode(paintLayer5, image->rootLayer());

    image->addNode(paintLayer2, groupLayer);
    image->addNode(complexRectsLayer1, groupLayer);
    image->addNode(paintLayer3, groupLayer);
    image->addNode(paintLayer4, groupLayer);

    QRect testRect(10,10,10,10);
    // Empty rect to show we don't need any cropping
    QRect cropRect;

    KisMergeWalker walker(cropRect);

    {
        QString order("root,paint5,cplx2,group,paint1,"
                      "paint4,paint3,cplx1,paint2");
        QStringList orderList = order.split(',');
        QRect accessRect(-7,-7,44,44);

        reportStartWith("paint3");
        walker.collectRects(paintLayer3, testRect);
        verifyResult(walker, orderList, accessRect, true, true);
    }

    {
        QString order("root,paint5,cplx2,group,paint1,"
                      "paint4,paint3,cplx1,paint2");
        QStringList orderList = order.split(',');
        QRect accessRect(-10,-10,50,50);

        reportStartWith("paint2");
        walker.collectRects(paintLayer2, testRect);
        verifyResult(walker, orderList, accessRect, true, true);
    }

    {
        QString order("root,paint5,cplx2,group,paint1");
        QStringList orderList = order.split(',');
        QRect accessRect(3,3,24,24);

        reportStartWith("paint5");
        walker.collectRects(paintLayer5, testRect);
        verifyResult(walker, orderList, accessRect, false, true);
    }

    {
        /**
         * Test cropping
         */
        QString order("root,paint5,cplx2,group,paint1,"
                      "paint4,paint3,cplx1,paint2");
        QStringList orderList = order.split(',');
        QRect accessRect(0,0,40,40);

        reportStartWith("paint2 (with cropping)");
        walker.setCropRect(image->bounds());
        walker.collectRects(paintLayer2, testRect);
        walker.setCropRect(cropRect);
        verifyResult(walker, orderList, accessRect, true, true);
    }

    {
        /**
         * Test uncropped values
         */
        QString order("root,paint5,cplx2,group,paint1,"
                      "paint4,paint3,cplx1,paint2");
        QStringList orderList = order.split(',');
        QRect cropRect(9,9,12,12);
        QRect accessRect(cropRect);

        reportStartWith("paint2 (testing uncropped)");
        walker.setCropRect(cropRect);
        walker.collectRects(paintLayer2, testRect);
        walker.setCropRect(cropRect);
        verifyResult(walker, orderList, accessRect, true, false);

        QCOMPARE(walker.uncroppedChangeRect(), QRect(4,4,22,22));
    }

}

#include "kis_psd_layer_style.h"

/*
  +----------+
  |root      |
  | group ls |
  |  paint 3 |
  |  paint 2 |
  | paint 1  |
  +----------+
 */

void KisWalkersTest::testComplexGroupVisiting()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "walker test");


    KisPSDLayerStyleSP style(new KisPSDLayerStyle());

    {
        style->context()->keep_original = true;

        style->dropShadow()->setEffectEnabled(true);
        style->dropShadow()->setDistance(3);
        style->dropShadow()->setSpread(1);
        style->dropShadow()->setSize(7);
        style->dropShadow()->setNoise(0);
        style->dropShadow()->setKnocksOut(false);
        style->dropShadow()->setOpacity(50);
        style->dropShadow()->setAngle(0);
    }

    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer3 = new KisPaintLayer(image, "paint3", OPACITY_OPAQUE_U8);

    KisLayerSP groupLayer = new KisGroupLayer(image, "groupls", OPACITY_OPAQUE_U8);

    image->addNode(paintLayer1, image->rootLayer());
    image->addNode(groupLayer, image->rootLayer());
    image->addNode(paintLayer2, groupLayer);
    image->addNode(paintLayer3, groupLayer);

    groupLayer->setLayerStyle(style);

    QRect testRect(10,10,10,10);
    // Empty rect to show we don't need any cropping
    QRect cropRect;

    KisMergeWalker walker(cropRect);

    {
        QString order("root,groupls,paint1,"
                      "paint3,paint2");
        QStringList orderList = order.split(',');
        QRect accessRect(-8,-8,46,46);

        reportStartWith("paint3");
        walker.collectRects(paintLayer3, testRect);
        verifyResult(walker, orderList, accessRect, true, true);
    }
}


    /*
      +------------+
      |root        |
      | layer 5    |
      | cplx  2    |
      | group      |
      |  paint 4   |
      |  cplxacc 1 |
      |  paint 3   |
      |  cplx  1   |
      |  paint 2   |
      | paint 1    |
      +------------+
     */

void KisWalkersTest::testComplexAccessVisiting()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "walker test");

    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer3 = new KisPaintLayer(image, "paint3", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer4 = new KisPaintLayer(image, "paint4", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer5 = new KisPaintLayer(image, "paint5", OPACITY_OPAQUE_U8);

    KisLayerSP groupLayer = new KisGroupLayer(image, "group", OPACITY_OPAQUE_U8);
    KisLayerSP complexRectsLayer1 = new ComplexRectsLayer(image, "cplx1", OPACITY_OPAQUE_U8);
    KisLayerSP complexRectsLayer2 = new ComplexRectsLayer(image, "cplx2", OPACITY_OPAQUE_U8);
    KisLayerSP complexAccess = new ComplexAccessLayer(image, "cplxacc1", OPACITY_OPAQUE_U8);

    image->addNode(paintLayer1, image->rootLayer());
    image->addNode(groupLayer, image->rootLayer());
    image->addNode(complexRectsLayer2, image->rootLayer());
    image->addNode(paintLayer5, image->rootLayer());

    image->addNode(paintLayer2, groupLayer);
    image->addNode(complexRectsLayer1, groupLayer);
    image->addNode(paintLayer3, groupLayer);
    image->addNode(complexAccess, groupLayer);
    image->addNode(paintLayer4, groupLayer);

    QRect testRect(10,10,10,10);
    // Empty rect to show we don't need any cropping
    QRect cropRect;

    KisMergeWalker walker(cropRect);

    {
        QString order("root,paint5,cplx2,group,paint1,"
                      "paint4,cplxacc1,paint3,cplx1,paint2");
        QStringList orderList = order.split(',');
        QRect accessRect = QRect(-7,-7,44,44) | QRect(0,0,30,30).translated(70,0);

        reportStartWith("paint3");
        walker.collectRects(paintLayer3, testRect);
        verifyResult(walker, orderList, accessRect, true, true);
    }
}


void KisWalkersTest::checkNotification(const KisMergeWalker::CloneNotification &notification,
                                       const QString &name,
                                       const QRect &rect)
{
    QCOMPARE(notification.m_layer->name(), name);
    QCOMPARE(notification.m_dirtyRect, rect);
}

    /*
      +--------------+
      |root          |
      | paint 3 <--+ |
      | cplx  2    | |
      | group <--+ | |
      |  cplx  1 | | |
      |  paint 2 | | |
      | clone 2 -+ | |
      | clone 1 ---+ |
      | paint 1      |
      +--------------+
     */

void KisWalkersTest::testCloneNotificationsVisiting()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "walker test");

    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer3 = new KisPaintLayer(image, "paint3", OPACITY_OPAQUE_U8);

    KisLayerSP groupLayer = new KisGroupLayer(image, "group", OPACITY_OPAQUE_U8);
    KisLayerSP complexRectsLayer1 = new ComplexRectsLayer(image, "cplx1", OPACITY_OPAQUE_U8);
    KisLayerSP complexRectsLayer2 = new ComplexRectsLayer(image, "cplx2", OPACITY_OPAQUE_U8);

    KisLayerSP cloneLayer1 = new KisCloneLayer(paintLayer3, image, "clone1", OPACITY_OPAQUE_U8);
    KisLayerSP cloneLayer2 = new KisCloneLayer(groupLayer, image, "clone2", OPACITY_OPAQUE_U8);

    image->addNode(paintLayer1, image->rootLayer());
    image->addNode(cloneLayer1, image->rootLayer());
    image->addNode(cloneLayer2, image->rootLayer());
    image->addNode(groupLayer, image->rootLayer());
    image->addNode(complexRectsLayer2, image->rootLayer());
    image->addNode(paintLayer3, image->rootLayer());

    image->addNode(paintLayer2, groupLayer);
    image->addNode(complexRectsLayer1, groupLayer);

    QRect testRect(10,10,10,10);
    QRect cropRect(5,5,507,507);

    KisMergeWalker walker(cropRect);

    {
        QString order("root,paint3,cplx2,group,clone2,clone1,paint1,"
                      "cplx1,paint2");
        QStringList orderList = order.split(',');
        QRect accessRect = QRect(5,5,35,35);

        reportStartWith("paint2");
        walker.collectRects(paintLayer2, testRect);
        verifyResult(walker, orderList, accessRect, true, true);

        const KisMergeWalker::CloneNotificationsVector vector = walker.cloneNotifications();
        QCOMPARE(vector.size(), 1);
        checkNotification(vector[0], "group", QRect(7,7,16,16));
    }
}


class TestingRefreshSubtreeWalker : public KisRefreshSubtreeWalker
{
public:
    TestingRefreshSubtreeWalker(QRect cropRect) : KisRefreshSubtreeWalker(cropRect) {}
    UpdateType type() const override { return FULL_REFRESH; }
};


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

void KisWalkersTest::testRefreshSubtreeVisiting()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "walker test");

    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer3 = new KisPaintLayer(image, "paint3", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer4 = new KisPaintLayer(image, "paint4", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer5 = new KisPaintLayer(image, "paint5", OPACITY_OPAQUE_U8);

    KisLayerSP groupLayer = new KisGroupLayer(image, "group", OPACITY_OPAQUE_U8);
    KisLayerSP complexRectsLayer1 = new ComplexRectsLayer(image, "cplx1", OPACITY_OPAQUE_U8);
    KisLayerSP complexRectsLayer2 = new ComplexRectsLayer(image, "cplx2", OPACITY_OPAQUE_U8);

    image->addNode(paintLayer1, image->rootLayer());
    image->addNode(groupLayer, image->rootLayer());
    image->addNode(complexRectsLayer2, image->rootLayer());
    image->addNode(paintLayer5, image->rootLayer());

    image->addNode(paintLayer2, groupLayer);
    image->addNode(complexRectsLayer1, groupLayer);
    image->addNode(paintLayer3, groupLayer);
    image->addNode(paintLayer4, groupLayer);

    QRect testRect(10,10,10,10);
    // Empty rect to show we don't need any cropping
    QRect cropRect;

    TestingRefreshSubtreeWalker walker(cropRect);

    {
        QString order("root,paint5,cplx2,group,paint1,"
                      "paint4,paint3,cplx1,paint2");
        QStringList orderList = order.split(',');
        QRect accessRect(-4,-4,38,38);

        reportStartWith("root");
        walker.collectRects(image->rootLayer(), testRect);
        verifyResult(walker, orderList, accessRect, false, true);
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

void KisWalkersTest::testFullRefreshVisiting()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "walker test");

    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer3 = new KisPaintLayer(image, "paint3", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer4 = new KisPaintLayer(image, "paint4", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer5 = new KisPaintLayer(image, "paint5", OPACITY_OPAQUE_U8);

    KisLayerSP groupLayer = new KisGroupLayer(image, "group", OPACITY_OPAQUE_U8);
    KisLayerSP complexRectsLayer1 = new ComplexRectsLayer(image, "cplx1", OPACITY_OPAQUE_U8);
    KisLayerSP complexRectsLayer2 = new ComplexRectsLayer(image, "cplx2", OPACITY_OPAQUE_U8);

    image->addNode(paintLayer1, image->rootLayer());
    image->addNode(groupLayer, image->rootLayer());
    image->addNode(complexRectsLayer2, image->rootLayer());
    image->addNode(paintLayer5, image->rootLayer());

    image->addNode(paintLayer2, groupLayer);
    image->addNode(complexRectsLayer1, groupLayer);
    image->addNode(paintLayer3, groupLayer);
    image->addNode(paintLayer4, groupLayer);

    QRect testRect(10,10,10,10);
    // Empty rect to show we don't need any cropping
    QRect cropRect;

    KisFullRefreshWalker walker(cropRect);

    {
        QString order("root,paint5,cplx2,group,paint1,"
                      "group,paint4,paint3,cplx1,paint2");
        QStringList orderList = order.split(',');
        QRect accessRect(-10,-10,50,50);

        reportStartWith("root");
        walker.collectRects(groupLayer, testRect);
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

void KisWalkersTest::testCachedVisiting()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "walker test");

    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer3 = new KisPaintLayer(image, "paint3", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer4 = new KisPaintLayer(image, "paint4", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer5 = new KisPaintLayer(image, "paint5", OPACITY_OPAQUE_U8);

    KisLayerSP groupLayer = new KisGroupLayer(image, "group", OPACITY_OPAQUE_U8);
    KisLayerSP cacheLayer1 = new CacheLayer(image, "cache1", OPACITY_OPAQUE_U8);

    image->addNode(paintLayer1, image->rootLayer());
    image->addNode(groupLayer, image->rootLayer());
    image->addNode(cacheLayer1, image->rootLayer());
    image->addNode(paintLayer5, image->rootLayer());

    image->addNode(paintLayer2, groupLayer);
    image->addNode(paintLayer3, groupLayer);
    image->addNode(paintLayer4, groupLayer);

    QRect testRect(10,10,10,10);
    // Empty rect to show we don't need any cropping
    QRect cropRect;

    KisMergeWalker walker(cropRect);

    {
        QString order("root,paint5,cache1,group,paint1,"
                      "paint4,paint3,paint2");
        QStringList orderList = order.split(',');
        QRect accessRect(0,0,30,30);

        reportStartWith("paint3");
        walker.collectRects(paintLayer3, testRect);
        verifyResult(walker, orderList, accessRect, true, true);
    }

    {
        QString order("root,paint5,cache1");
        QStringList orderList = order.split(',');
        QRect accessRect(10,10,10,10);

        reportStartWith("paint5");
        walker.collectRects(paintLayer5, testRect);
        verifyResult(walker, orderList, accessRect, false, true);
    }

}

    /*
      +----------+
      |root      |
      | paint 2  |
      | paint 1  |
      |  fmask2  |
      |  tmask   |
      |  fmask1  |
      +----------+
     */

void KisWalkersTest::testMasksVisiting()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "walker test");

    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE_U8);

    image->addNode(paintLayer1, image->rootLayer());
    image->addNode(paintLayer2, image->rootLayer());

    KisFilterMaskSP filterMask1 = new KisFilterMask();
    KisFilterMaskSP filterMask2 = new KisFilterMask();
    KisTransparencyMaskSP transparencyMask = new KisTransparencyMask();

    KisFilterSP filter = KisFilterRegistry::instance()->value("blur");
    Q_ASSERT(filter);
    KisFilterConfigurationSP configuration1 = filter->defaultConfiguration();
    KisFilterConfigurationSP configuration2 = filter->defaultConfiguration();

    filterMask1->setFilter(configuration1);
    filterMask2->setFilter(configuration2);

    QRect selection1(10, 10, 20, 10);
    QRect selection2(30, 15, 10, 10);
    QRect selection3(20, 10, 20, 10);

    filterMask1->testingInitSelection(selection1, paintLayer1);
    transparencyMask->testingInitSelection(selection2, paintLayer1);
    filterMask2->testingInitSelection(selection3, paintLayer1);

    image->addNode(filterMask1, paintLayer1);
    image->addNode(transparencyMask, paintLayer1);
    image->addNode(filterMask2, paintLayer1);

    QRect testRect(5,5,30,30);
    // Empty rect to show we don't need any cropping
    QRect cropRect;

    KisMergeWalker walker(cropRect);
    {
        QString order("root,paint2,paint1");
        QStringList orderList = order.split(',');
        QRect accessRect(0,0,40,40);

        reportStartWith("tmask");
        walker.collectRects(transparencyMask, testRect);
        verifyResult(walker, orderList, accessRect, true, false);
    }

    KisTestWalker twalker;
    {
        QString order("paint2,root,"
                      "root_TF,paint2_TA,paint1_BP");
        QStringList orderList = order.split(',');

        reportStartWith("tmask");
        twalker.startTrip(transparencyMask->projectionLeaf());
        QCOMPARE(twalker.popResult(), orderList);
    }
}

    /*
      +----------+
      |root      |
      | paint 2  |
      | paint 1  |
      |  fmask2  |
      |  tmask   |
      |  fmask1  |
      +----------+
     */

void KisWalkersTest::testMasksVisitingNoFilthy()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "walker test");

    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE_U8);

    image->addNode(paintLayer1, image->rootLayer());
    image->addNode(paintLayer2, image->rootLayer());

    KisFilterMaskSP filterMask1 = new KisFilterMask();
    KisFilterMaskSP filterMask2 = new KisFilterMask();
    KisTransparencyMaskSP transparencyMask = new KisTransparencyMask();

    KisFilterSP filter = KisFilterRegistry::instance()->value("blur");
    Q_ASSERT(filter);
    KisFilterConfigurationSP configuration1 = filter->defaultConfiguration();
    KisFilterConfigurationSP configuration2 = filter->defaultConfiguration();

    filterMask1->setFilter(configuration1);
    filterMask2->setFilter(configuration2);

    QRect selection1(10, 10, 20, 10);
    QRect selection2(30, 15, 10, 10);
    QRect selection3(20, 10, 20, 10);

    filterMask1->testingInitSelection(selection1, paintLayer1);
    transparencyMask->testingInitSelection(selection2, paintLayer1);
    filterMask2->testingInitSelection(selection3, paintLayer1);

    image->addNode(filterMask1, paintLayer1);
    image->addNode(transparencyMask, paintLayer1);
    image->addNode(filterMask2, paintLayer1);

    QRect testRect(5,5,30,30);
    // Empty rect to show we don't need any cropping
    QRect cropRect;

    {
        KisMergeWalker walker(cropRect, KisMergeWalker::NO_FILTHY);

        QString order("root,paint2,paint1");
        QStringList orderList = order.split(',');
        QRect accessRect(0,0,40,40);

        reportStartWith("tmask");
        walker.collectRects(transparencyMask, testRect);
        verifyResult(walker, orderList, accessRect, true, false);
    }

    {
        KisMergeWalker walker(cropRect, KisMergeWalker::NO_FILTHY);

        QString order("root,paint2,paint1");
        QStringList orderList = order.split(',');
        QRect accessRect(5,5,30,30);

        reportStartWith("paint1");
        walker.collectRects(paintLayer1, testRect);
        verifyResult(walker, orderList, accessRect, false, false);
    }
}

    /*
      +----------+
      |root      |
      | paint 2  |
      | paint 1  |
      |  fmask2  |
      |  tmask   |
      |  fmask1  |
      +----------+
     */

void KisWalkersTest::testMasksOverlapping()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "walker test");

    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE_U8);

    image->addNode(paintLayer1, image->rootLayer());
    image->addNode(paintLayer2, image->rootLayer());

    KisFilterMaskSP filterMask1 = new KisFilterMask();
    KisFilterMaskSP filterMask2 = new KisFilterMask();
    KisTransparencyMaskSP transparencyMask = new KisTransparencyMask();

    KisFilterSP blurFilter = KisFilterRegistry::instance()->value("blur");
    KisFilterSP invertFilter = KisFilterRegistry::instance()->value("invert");
    Q_ASSERT(blurFilter);
    Q_ASSERT(invertFilter);
    KisFilterConfigurationSP blurConfiguration = blurFilter->defaultConfiguration();
    KisFilterConfigurationSP invertConfiguration = invertFilter->defaultConfiguration();

    filterMask1->setFilter(invertConfiguration);
    filterMask2->setFilter(blurConfiguration);

    QRect selection1(0, 0, 128, 128);
    QRect selection2(128, 0, 128, 128);
    QRect selection3(0, 64, 256, 128);

    filterMask1->testingInitSelection(selection1, paintLayer1);
    transparencyMask->testingInitSelection(selection2, paintLayer1);
    filterMask2->testingInitSelection(selection3, paintLayer1);

    image->addNode(filterMask1, paintLayer1);
    image->addNode(transparencyMask, paintLayer1);
    image->addNode(filterMask2, paintLayer1);

    // Empty rect to show we don't need any cropping
    QRect cropRect;

    QRect IMRect(10,10,50,50);
    QRect TMRect(135,10,40,40);
    QRect IMTMRect(10,10,256,40);

    QList<UpdateTestJob> updateList;

    {
        // FIXME: now we do not crop change rect if COMPOSITE_OVER is used!

        UpdateTestJob job = {"IM", paintLayer1, IMRect,
                               "",
                               QRect(0,0,0,0), true, false};
        updateList.append(job);
    }

    {
        UpdateTestJob job = {"IM", filterMask1, IMRect,
                             "",
                             QRect(0,0,0,0), true, false};
        updateList.append(job);
    }

    {
        UpdateTestJob job = {"IM", transparencyMask, IMRect,
                             "root,paint2,paint1",
                             QRect(5,10,60,55), true, false};
        updateList.append(job);
    }

    {
        UpdateTestJob job = {"IM", filterMask2, IMRect,
                             "root,paint2,paint1",
                             IMRect, false, false};
        updateList.append(job);
    }

    /******* Dirty rect: transparency mask *********/

    {
        UpdateTestJob job = {"TM", paintLayer1, TMRect,
                             "root,paint2,paint1",
                             TMRect, false, false};
        updateList.append(job);
    }

    {
        UpdateTestJob job = {"TM", filterMask1, TMRect,
                             "root,paint2,paint1",
                             TMRect, false, false};
        updateList.append(job);
    }

    {
        UpdateTestJob job = {"TM", transparencyMask, TMRect,
                             "root,paint2,paint1",
                             TMRect, false, false};
        updateList.append(job);
    }

    {
        UpdateTestJob job = {"TM", filterMask2, TMRect,
                             "root,paint2,paint1",
                             TMRect, false, false};
        updateList.append(job);
    }

    /******* Dirty rect: invert + transparency mask *********/

    {
        UpdateTestJob job = {"IMTM", paintLayer1, IMTMRect,
                             "root,paint2,paint1",
                             IMTMRect & selection2, true, false};
        updateList.append(job);
    }

    {
        UpdateTestJob job = {"IMTM", filterMask1, IMTMRect,
                             "root,paint2,paint1",
                             IMTMRect & selection2, true, false};
        updateList.append(job);
    }

    {
        UpdateTestJob job = {"IMTM", transparencyMask, IMTMRect,
                             "root,paint2,paint1",
                             IMTMRect, false, false};
        updateList.append(job);
    }

    {
        UpdateTestJob job = {"IMTM", filterMask2, IMTMRect,
                             "root,paint2,paint1",
                             IMTMRect, false, false};
        updateList.append(job);
    }

    Q_FOREACH (UpdateTestJob job, updateList) {
        KisMergeWalker walker(cropRect);
        reportStartWith(job.startNode->name(), job.updateRect);
        qDebug() << "Area:" << job.updateAreaName;
        walker.collectRects(job.startNode, job.updateRect);
        verifyResult(walker, job);
    }
}

    /*
      +----------+
      |root      |
      | adj      |
      | paint 1  |
      +----------+
     */

void KisWalkersTest::testRectsChecksum()
{
    QRect imageRect(0,0,512,512);
    QRect dirtyRect(100,100,100,100);

    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, imageRect.width(), imageRect.height(), colorSpace, "walker test");

    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
    KisAdjustmentLayerSP adjustmentLayer = new KisAdjustmentLayer(image, "adj", 0, 0);

    image->lock();
    image->addNode(paintLayer1, image->rootLayer());
    image->addNode(adjustmentLayer, image->rootLayer());
    image->unlock();

    KisFilterSP filter = KisFilterRegistry::instance()->value("blur");
    Q_ASSERT(filter);
    KisFilterConfigurationSP configuration;

    KisMergeWalker walker(imageRect);
    walker.collectRects(adjustmentLayer, dirtyRect);
    QCOMPARE(walker.checksumValid(), true);

    configuration = filter->defaultConfiguration();
    adjustmentLayer->setFilter(configuration);
    QCOMPARE(walker.checksumValid(), false);

    walker.recalculate(dirtyRect);
    QCOMPARE(walker.checksumValid(), true);

    configuration = filter->defaultConfiguration();
    configuration->setProperty("halfWidth", 20);
    configuration->setProperty("halfHeight", 20);
    adjustmentLayer->setFilter(configuration);
    QCOMPARE(walker.checksumValid(), false);

    walker.recalculate(dirtyRect);
    QCOMPARE(walker.checksumValid(), true);

    configuration = filter->defaultConfiguration();
    configuration->setProperty("halfWidth", 21);
    configuration->setProperty("halfHeight", 21);
    adjustmentLayer->setFilter(configuration);
    QCOMPARE(walker.checksumValid(), false);

    walker.recalculate(dirtyRect);
    QCOMPARE(walker.checksumValid(), true);

}

void KisWalkersTest::testGraphStructureChecksum()
{
    QRect imageRect(0,0,512,512);
    QRect dirtyRect(100,100,100,100);

    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, imageRect.width(), imageRect.height(), colorSpace, "walker test");

    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
    KisLayerSP paintLayer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE_U8);

    image->lock();
    image->addNode(paintLayer1, image->rootLayer());
    image->unlock();

    KisMergeWalker walker(imageRect);
    walker.collectRects(paintLayer1, dirtyRect);
    QCOMPARE(walker.checksumValid(), true);

    image->lock();
    image->addNode(paintLayer2, image->rootLayer());
    image->unlock();
    QCOMPARE(walker.checksumValid(), false);

    walker.recalculate(dirtyRect);
    QCOMPARE(walker.checksumValid(), true);

    image->lock();
    image->moveNode(paintLayer1, image->rootLayer(), paintLayer2);
    image->unlock();
    QCOMPARE(walker.checksumValid(), false);

    walker.recalculate(dirtyRect);
    QCOMPARE(walker.checksumValid(), true);

    image->lock();
    image->removeNode(paintLayer1);
    image->unlock();
    QCOMPARE(walker.checksumValid(), false);

    walker.recalculate(dirtyRect);
    QCOMPARE(walker.checksumValid(), true);
}

QTEST_MAIN(KisWalkersTest)

