#include "kis_watercolorop_test.h"

#include <QTest>
#include <qimage_based_test.h>

#include "plugins/paintops/watercolor/kis_wetmap.h"

#include <QString>
#include <QVector>
#include <kis_filter_configuration.h>

class TestWetMap : public TestUtil::QImageBasedTest
{
public:
    TestWetMap() : QImageBasedTest("WetMap") {}

    void test() {
        QVector<QPoint> pos;
        QVector<qreal> radius;

        pos.push_back(QPoint(10, 10));
        radius.push_back(5.5);
        pos.push_back(QPoint(50, 50));
        radius.push_back(25);
        pos.push_back(QPoint(63, 60));
        radius.push_back(19.36);
        test(pos, radius);

        test(256, QString("Full_Drying"));
        test(128, QString("Half_Drying"));
        test(64, QString("Quarter_Drying"));

        pos.clear();
        radius.clear();
        for (int i = 0; i < 3; i++)
            radius.push_back(50);
        pos.push_back(QPoint(50, 50));
        pos.push_back(QPoint(100, 50));
        pos.push_back(QPoint(150, 50));
        QVector<int> addingTime;
        addingTime.push_back(0);
        addingTime.push_back(64);
        addingTime.push_back(128);
        test(256, pos, radius, addingTime, "One_Line");
        test(300, pos, radius, addingTime, "One_Line_Overtime");

        pos.clear();
        pos.push_back(QPoint(50, 50));
        pos.push_back(QPoint(125, 50));
        pos.push_back(QPoint(100, 125));
        test(256, pos, radius, addingTime, "Triange");
        test(300, pos, radius, addingTime, "Triange_Overtime");

        test(0);
        test(64);
        test(128);
        test(256);

        test(300);
    }

    // Testing adding watter on wetmap
    void test(QVector<QPoint> pos, QVector<qreal> radius) {

        KisWetMap *wetMap = new KisWetMap();
        for (int i = 0; i < pos.size(); i++) {
            wetMap->addWater(pos.at(i), radius.at(i));
        }
        QVERIFY(TestUtil::checkQImage(wetMap->getPaintDevice()->convertToQImage(0), QString("WaterColor"),
                              QString("WetMap"), QString("Adding_Water")));
        delete wetMap;
    }

    // Testing drying
    void test(int time, QString prefix) {
        KisWetMap *wetMap = new KisWetMap();
        QPoint pos(50, 50);
        qreal radius = 25;

        wetMap->addWater(pos, radius);

        for (int i = 0; i < time; i++) {
            wetMap->update();
        }

        QVERIFY(TestUtil::checkQImage(wetMap->getPaintDevice()->convertToQImage(0), QString("WaterColor"),
                              QString("WetMap"), prefix));
        delete wetMap;
    }

    // Tesitng multiple actions with wetmap
    void test(int fullWorkTime, QVector<QPoint> pos, QVector<qreal> radius,
              QVector<int> addingTime, QString prefix) {
        KisWetMap *wetMap = new KisWetMap();
        int posI;
        for (int i = 0; i <= fullWorkTime; i++) {
            if ((posI = addingTime.indexOf(i)) != -1) {
                wetMap->addWater(pos.at(posI), radius.at(posI));
            }
            wetMap->update();
        }

        QVERIFY(TestUtil::checkQImage(wetMap->getPaintDevice()->convertToQImage(0), QString("WaterColor"),
                              QString("WetMap_Multiple_Actions"), prefix));
        delete wetMap;
    }

    // Testing reading watercount
    void test(int time) {
        KisWetMap *wetMap = new KisWetMap();
        wetMap->addWater(QPoint(50, 50), 50);
        for (int i = 0; i < time; i++)
            wetMap->update();
        int realVal = wetMap->getWater(50, 50);
        int expectedVal = 65535 - 256 * time;
        if (expectedVal < 0)
            expectedVal = 0;
        QCOMPARE(realVal, expectedVal);
    }
};

void WetMapTest::testWetMap()
{
    TestWetMap t;
    t.test();
}

QTEST_MAIN(WetMapTest)
