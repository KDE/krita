/*
 *  SPDX-FileCopyrightText: 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef KIS_ACTION_MANAGER_TEST_H
#define KIS_ACTION_MANAGER_TEST_H

#include <simpletest.h>

class KisActionManagerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testUpdateGUI();
    void testCondition();
    void testTakeAction();
};

#endif // KIS_ACTION_MANAGER_TEST_H
