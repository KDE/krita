/*
 *  SPDX-FileCopyrightText: 2019 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef KISMULTIFEEDRSSMODELTEST_H
#define KISMULTIFEEDRSSMODELTEST_H

#include <QObject>

class KisMultiFeedRssModelTest : public QObject
{
    Q_OBJECT
public:
    explicit KisMultiFeedRssModelTest(QObject *parent = nullptr);

private Q_SLOTS:
    void testAddFeed();
    void testAddFeed_data();

    void testRemoveFeed();
    void testRemoveFeed_data();
};

#endif // KISMULTIFEEDRSSMODELTEST_H
