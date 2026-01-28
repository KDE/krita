/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISGLOBALTEST_H
#define KISGLOBALTEST_H

#include <QObject>

class KisGlobalTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testDeduplicateFileName_data();
    void testDeduplicateFileName();
};

#endif // KISGLOBALTEST_H
