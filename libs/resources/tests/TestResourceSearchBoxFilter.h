/*  This file is part of the Krita project

    SPDX-FileCopyrightText: 2019 Agata Cacko <cacko.azh@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
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

private:
    bool filterMatches(QString resourceName, QString filter);
    void runNameTest();

};

#endif // TESTRESOURCESEARCHBOXFILTER_H
