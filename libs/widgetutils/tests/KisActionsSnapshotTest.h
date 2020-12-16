/*
 *  SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KO_PROPERTIES_TEST_H
#define KO_PROPERTIES_TEST_H

#include <QObject>

class KisActionsSnapshotTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testCreation();
};

#endif

