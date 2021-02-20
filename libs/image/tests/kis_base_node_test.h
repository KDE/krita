/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_BASE_NODE_TESTER_H
#define KIS_BASE_NODE_TESTER_H

#include <QtTest>

class KisBaseNodeTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testCreation();
    void testContract();
    void testProperties();
    void testOpacityKeyframing();
};

#endif

