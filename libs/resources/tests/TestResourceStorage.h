/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef TESTRESOURCESTORAGE_H
#define TESTRESOURCESTORAGE_H

#include <QObject>

class TestResourceStorage : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testStorage();
private:
};

#endif
