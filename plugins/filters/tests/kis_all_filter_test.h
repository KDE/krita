/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_ALL_FILTER_TEST_H
#define KIS_ALL_FILTER_TEST_H

#include <simpletest.h>

class KisAllFilterTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testAllFilters();
    void testAllFiltersSrcNotIsDev();
    void testAllFiltersWithSelections();
};

#endif
