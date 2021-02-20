/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_CLIPBOARD_TEST_H
#define __KIS_CLIPBOARD_TEST_H

#include <QtTest>

class KisClipboardTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testRoundTrip();
};

#endif /* __KIS_CLIPBOARD_TEST_H */
