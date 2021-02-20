/*
 * SPDX-FileCopyrightText: 2019 boud <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TESTRESOURCE_H
#define TESTRESOURCE_H

#include <QObject>

class TestResource : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCopyResource();
};

#endif // TESTRESOURCE_H
