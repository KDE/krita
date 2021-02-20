/*  This file is part of the Krita project

    SPDX-FileCopyrightText: 2019 Agata Cacko <cacko.azh@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "TestResourceSearchBoxFilter.h"


#include <QTest>


TestResourceSearchBoxFilter::TestResourceSearchBoxFilter()
{
}


bool TestResourceSearchBoxFilter::filterMatches(QString resourceName, QString filterText)
{
    KisResourceSearchBoxFilter filter;
    filter.setFilter(filterText);
    return filter.matchesResource(resourceName, QStringList());
}

void addNameDataColumns()
{
    QTest::addColumn<QString>("resourceName");
    QTest::addColumn<QString>("filter");
    QTest::addColumn<bool>("matches");
}

void addNameData(QString name, QString resourceName, QString filter, bool matches)
{
    QTest::newRow(name.toStdString().c_str()) << resourceName << filter << matches;
}

void TestResourceSearchBoxFilter::runNameTest()
{
    QFETCH(QString, resourceName);
    QFETCH(QString, filter);
    QFETCH(bool, matches);
    QCOMPARE(filterMatches(resourceName, filter), matches);
}

QList<MockResource> TestResourceSearchBoxFilter::filterResourceList(QList<MockResource> &resources, KisResourceSearchBoxFilter &filter)
{
    QList<MockResource> matches;
    Q_FOREACH(const MockResource& resource, resources) {
        if (filter.matchesResource(resource.m_name, resource.m_tags)) {
            matches.append(resource);
        }
    }
    return matches;
}

void TestResourceSearchBoxFilter::testOnePartialName_data()
{
    addNameDataColumns();
    addNameData("1", "Nanana", "na", true);
    addNameData("2", "Nanana", "nam", false);
    addNameData("3", "Nanana", "Nanana", true);
    addNameData("4", "Nanana", "Nananam", false);
    addNameData("5", "f)_Bristle-4_Glaze", "Bristle", true);

    addNameData("6", "Nanana", "!na", false);
    addNameData("7", "Nanana", "!nam", true);
    addNameData("8", "Nanana", "!Nanana", false);
    addNameData("9", "Nanana", "!Nananam", true);
    addNameData("10", "f)_Bristle-4_Glaze", "!Bristle", false);

}

void TestResourceSearchBoxFilter::testOnePartialName()
{
    runNameTest();

}

void TestResourceSearchBoxFilter::testMultiplePartialNames_data()
{
    addNameDataColumns();
    addNameData("1", "Nanana", "na,nan", true);
    addNameData("2", "Bristle", "Bristl,Na", false);
    addNameData("3", "Bristle", "Bri,stle", true);
    addNameData("4", "Bristle", "Glaze,tle", false);
    addNameData("5", "f)_Bristle-4_Glaze", "Bristle,Glaze", true);

    addNameData("6", "Nanana", "!na,!nan", false);
    addNameData("7", "Bristle", "!Bristl,!Na", false);
    addNameData("8", "Bristle", "!Bri,!stle", false);
    addNameData("9", "Bristle", "!Glaze,!tle", false);
    addNameData("10", "f)_Bristle-4_Glaze", "!Bristle,!Glaze", false);
    addNameData("11", "Bristle", "!na,!nan", true);
}

void TestResourceSearchBoxFilter::testMultiplePartialNames()
{
    runNameTest();
}

void TestResourceSearchBoxFilter::testOneExactMatch_data()
{
    addNameDataColumns();
    addNameData("1", "Resource1", "\"Resource1\"", true);
    addNameData("2", "Brush2", "\"nam\"", false);
    addNameData("3", "Preset3", "\"Preset3\"", true);
    addNameData("4", "Nanana", "\"Nananam\"", false);
    addNameData("5", "f)_Bristle-4_Glaze", "\"f)_Bristle-4_Glaze\"", true);

    addNameData("6", "Resource1", "!\"Resource1\"", false);
    addNameData("7", "Brush2", "!\"nam\"", true);
    addNameData("8", "Preset3", "!\"Preset3\"", false);
    addNameData("9", "Nanana", "!\"Nananam\"", true);
    addNameData("10", "f)_Bristle-4_Glaze", "!\"f)_Bristle-4_Glaze\"", false);
}

void TestResourceSearchBoxFilter::testOneExactMatch()
{
    runNameTest();
}

void TestResourceSearchBoxFilter::testMultipleExactMatches_data()
{
    addNameDataColumns();
    addNameData("1", "Nanana", "\"Nanana\",\"Nanana\"", true);
    // needs to ask for the expected behaviour first
    /* addNameData("2", "Nanana", "\"Nanana\",\"Glaze\"", false); */

    addNameData("3", "Nanana", "!\"Nanana\",!\"Nanana\"", false);
    addNameData("4", "Nanana", "!\"Nanana\",\"Nanana\"", false);
    addNameData("5", "Nanana", "!\"Nanana\",!\"Glaze\"", false);
    addNameData("6", "Bristle", "!\"Nanana\",!\"Glaze\"", true);

}

void TestResourceSearchBoxFilter::testMultipleExactMatches()
{
    runNameTest();
}

void TestResourceSearchBoxFilter::testResourceSearch()
{
    // Define some tags to be reused later
    const QString tagPencil = "Pencil";
    const QString tagPen = "Pen";
    const QString tagEraser = "Eraser";
    const QString tagHard = "Hard";
    const QString tagSoft = "Soft";
    const QString tagTexture = "Texture";
    const QString tagPaint = "Paint";


    // Define a list of resources that make up our "database" that we wish to filter.
    QList<MockResource> resources;

    MockResource hbPencil("HB Pencil", QStringList() << tagPencil << tagTexture << tagHard);
    resources.append( hbPencil );

    MockResource dullLeadHolder("Dull Leadholder", QStringList() << tagPencil << tagTexture << tagSoft);
    resources.append(dullLeadHolder);

    MockResource watercolorFlat("Watercolor Flat Brush", QStringList() << tagHard << tagPaint);
    resources.append(watercolorFlat);

    MockResource watercolorDry("Watercolor Round Dry Brush", QStringList() << tagSoft << tagTexture);
    resources.append(watercolorDry);

    MockResource pixel("Single Pixel Brush", QStringList() << tagHard);
    resources.append(pixel);

    MockResource ink("David's Inking Brush", QStringList() << tagPen << tagHard);
    resources.append(ink);

    MockResource impasto("Ramon's Oil Impasto Round Brush", QStringList() << tagHard << tagTexture << tagPaint);
    resources.append(impasto);

    MockResource oilKnife("Ramon's Oil Palette Knife", QStringList() << tagPaint << tagHard);
    resources.append(oilKnife);

    MockResource rubber("Kneaded Rubber Eraser", QStringList() << tagEraser << tagSoft);
    resources.append(rubber);

    MockResource polymer("Polymer Eraser", QStringList() << tagHard << tagEraser);
    resources.append(polymer);


    // Create filter instance, check initialization and basic API test.
    KisResourceSearchBoxFilter filter;
    QVERIFY(filter.isEmpty());

    filter.setFilter("Pen");
    QVERIFY(!filter.isEmpty());
    QVERIFY(filter.matchesResource("Pen", QStringList()));
    QVERIFY(filter.matchesResource("pEn", QStringList())); //Test case insensitivity

    filter.setFilter(QString());
    QVERIFY(filter.isEmpty());


    {   // Find all pencils
        filter.setFilter("Pencil");
        QList<MockResource> results = filterResourceList(resources, filter);
        QVERIFY(results.size() == 2);
        QVERIFY(results.contains(hbPencil));
        QVERIFY(results.contains(dullLeadHolder));
        QVERIFY(!results.contains(impasto));
        QVERIFY(!results.contains(rubber));
    }

    {   // Find all Ramon's brushes using partial search
        filter.setFilter("Ramo");
        QList<MockResource> results = filterResourceList(resources, filter);
        QVERIFY(results.size() == 2);
        QVERIFY(results.contains(impasto));
        QVERIFY(results.contains(oilKnife));
        QVERIFY(!results.contains(polymer));
    }

    {   // Find only the brushes with a specific tag
        filter.setFilter("#\"" + tagTexture + "\"");
        QList<MockResource> results = filterResourceList(resources, filter);
        QVERIFY(results.size() == 4);
        QVERIFY(results.contains(hbPencil));
        QVERIFY(results.contains(dullLeadHolder));
        QVERIFY(results.contains(watercolorDry));
        QVERIFY(results.contains(impasto));
        QVERIFY(!results.contains(polymer));

        filter.setFilter("!#\"" + tagTexture + "\"");
        results = filterResourceList(resources, filter);
        QVERIFY(results.size() == resources.size() - 4);
        QVERIFY(!results.contains(hbPencil));
        QVERIFY(!results.contains(dullLeadHolder));
        QVERIFY(!results.contains(watercolorDry));
        QVERIFY(!results.contains(impasto));
        QVERIFY(results.contains(polymer));
    }

    {   // Find with a very short partial search...
        filter.setFilter("er");
        QList<MockResource> results = filterResourceList(resources, filter);
        QVERIFY(results.size() == 5);
        QVERIFY(results.contains(dullLeadHolder));
        QVERIFY(results.contains(watercolorFlat));
        QVERIFY(results.contains(watercolorDry));
        QVERIFY(results.contains(rubber));
        QVERIFY(results.contains(polymer));
    }

    {   // Set filter to be empty, should show everything.
        filter.setFilter(QString());
        QList<MockResource> results = filterResourceList(resources, filter);
        QVERIFY(results.size() == resources.size());
    }
}



QTEST_MAIN(TestResourceSearchBoxFilter)
