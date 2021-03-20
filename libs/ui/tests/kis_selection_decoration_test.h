/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SELECTION_DECORATION_TEST_H
#define __KIS_SELECTION_DECORATION_TEST_H

#include <simpletest.h>

class KisSelectionDecorationTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testConcurrentSelectionFetches();
};

#endif /* __KIS_SELECTION_DECORATION_TEST_H */
