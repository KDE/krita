/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef TESTKOCOLORSET_H
#define TESTKOCOLORSET_H

#include <QObject>
#include <KoColor.h>
#include <KoColorSet.h>

class TestKoColorSet : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testLoadGPL();
    void testLoadRIFF();
    void testLoadACT();
    void testLoadPSP_PAL();
    void testLoadACO();
    void testLoadXML();
    void testLoadKPL();
    void testLoadSBZ_data();
    void testLoadSBZ();
    void testLoadASE();
    void testLoadACB();
    void testLock();
    void testColumnCount();
    void testComment();
    void testPaletteType();
    void testAddSwatch();
    void testRemoveSwatch();
    void testAddGroup();
    void testChangeGroupName();
    void testMoveGroup();
    void testRemoveGroup();
    void testClear();
    void testGetSwatchFromGroup();
    void testIsGroupNameRow();
    void testStartRowForNamedGroup();
    void testGetClosestSwatchInfo();
    void testGetGroup();
    void testAllRows();
    void testRowNumberInGroup();
    void testGetColorGlobal();

private:

    KoColorSetSP createColorSet();
    KoColor blue();
    KoColor red();

};


#endif /* TESTKOCOLORSET_H */
