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
#ifndef TEST_RESOURCE_SEARCH_BOX_FILTER_H
#define TEST_RESOURCE_SEARCH_BOX_FILTER_H

#include <QObject>
#include <QScopedPointer>

#include <KisResourceSearchBoxFilter.h>

class TestResourceSearchBoxFilter : public QObject
{
    Q_OBJECT

public:

    TestResourceSearchBoxFilter();

private Q_SLOTS:
    void testOnePartialName_data();
    void testOnePartialName();

    void testMultiplePartialNames_data();
    void testMultiplePartialNames();

    void testOneExactMatch_data();
    void testOneExactMatch();

    void testMultipleExactMatches_data();
    void testMultipleExactMatches();

    void testOneTag_data();
    void testOneTag();

    void testMultipleTags_data();
    void testMultipleTags();

private:
    bool filterMatches(QString resourceName, QString filter);
    void runNameTest();

};

#endif // TESTRESOURCESEARCHBOXFILTER_H
