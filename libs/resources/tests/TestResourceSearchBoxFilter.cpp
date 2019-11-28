/*  This file is part of the Krita project

    Copyright (c) 2019 Agata Cacko <cacko.azh@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
    return filter.matchesResource(resourceName);
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

void TestResourceSearchBoxFilter::testOneTag_data()
{
    // tag filtering not implemented yet
}

void TestResourceSearchBoxFilter::testOneTag()
{
    // tag filtering not implemented yet
}

void TestResourceSearchBoxFilter::testMultipleTags_data()
{
    // tag filtering not implemented yet
}

void TestResourceSearchBoxFilter::testMultipleTags()
{
    // tag filtering not implemented yet
}




QTEST_MAIN(TestResourceSearchBoxFilter)
