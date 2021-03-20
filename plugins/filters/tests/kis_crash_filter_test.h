/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_CRASH_FILTER_TEST_H
#define KIS_CRASH_FILTER_TEST_H

#include <simpletest.h>

class KoColorSpace;
#include <kis_types.h>

/**
 * The crash filter test just loops through all filters
 * and tests every filter with all colorspaces we have.
 * We don't check the output; just whether we've got a
 * crasher somewhere.
 */
class KisCrashFilterTest : public QObject
{
    Q_OBJECT
private:

    bool applyFilter(const KoColorSpace * cs,  KisFilterSP f);
    bool testFilter(KisFilterSP f);

private Q_SLOTS:

    void testCrashFilters();
};

#endif
