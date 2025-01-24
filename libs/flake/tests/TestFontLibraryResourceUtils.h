/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TESTLIBRARYRESOURCEUTILS_H
#define TESTLIBRARYRESOURCEUTILS_H

#include <QObject>
#include <simpletest.h>

class TestFontLibraryResourceUtils : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCreation();
    void testCopy();
    void testCopyAssignment();
    void testMove();
    void testReset();
    void testAssignNull();
    void testFailedDestruction();
};

#endif // TESTLIBRARYRESOURCEUTILS_H
