/*
 *  SPDX-FileCopyrightText: 2019 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOGAMUTMASKTEST_H
#define KOGAMUTMASKTEST_H

#include <QObject>

class KoGamutMaskTest : public QObject
{
    Q_OBJECT
public:
    explicit KoGamutMaskTest(QObject *parent = nullptr);

private Q_SLOTS:
    void testCoordIsClear();
    void testCoordIsClear_data();

    void testLoad();
    void testLoad_data();
// TODO: add preview vs. non-preview testing
};

#endif // KOGAMUTMASKTEST_H
