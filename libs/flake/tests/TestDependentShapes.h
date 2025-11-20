/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TESTDEPENDENTSHAPES_H
#define TESTDEPENDENTSHAPES_H

#include <QObject>

class TestDependentShapes : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testDeletionNotifications_data();
    void testDeletionNotifications();

    void testBulkActionInterface_data();
    void testBulkActionInterface();

    void testBulkActionLock_data();
    void testBulkActionLock();
};

#endif /* TESTDEPENDENTSHAPES_H */
