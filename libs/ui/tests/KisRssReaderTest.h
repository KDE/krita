/*
 *  SPDX-FileCopyrightText: 2019 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISRSSREADERTEST_H
#define KISRSSREADERTEST_H

#include <QObject>
#include <simpletest.h>

class KisRssReaderTest : public QObject
{
    Q_OBJECT
public:
    explicit KisRssReaderTest(QObject *parent = nullptr);

private Q_SLOTS:
    void testParseData();
};

#endif // KISRSSREADERTEST_H
