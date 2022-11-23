/*
 *  SPDX-FileCopyrightText: 2022 Halla Rempt <halla@valdyas.org>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __TEST_KIS_PALETTE_MODEL_H
#define __TEST_KIS_PALETTE_MODEL_H

#include <QObject>
#include <KoColor.h>
#include <KoColorSet.h>

class TestKisPaletteModel : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testSetColorSet();

    void testAddSwatch();

    void testSetSwatch();

    void testRemoveSwatch();

    void testChangeGroupName();

    void testRemoveGroup();

    void testAddGroup();

    void testSetRowCountForGroup();

    void testClear();

    void testIndexRowForInfo();

    void testRowNumberInGroup();

    void testIndexForClosest();

    void testData();

private:

    KoColorSetSP createColorSet();
    KoColor blue();
    KoColor red();

};

#endif /* __KIS_PALETTE_MODEL */
