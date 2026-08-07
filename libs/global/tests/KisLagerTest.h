/*
 *  SPDX-FileCopyrightText: 2026 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISLAGERTEST_H
#define KISLAGERTEST_H

#include <QTest>

class KisLagerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testOptionalHasValue();
};

#endif // KISLAGERTEST_H
