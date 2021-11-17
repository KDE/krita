/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef TESTKOCOLORSET_H
#define TESTKOCOLORSET_H

#include <QObject>

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
    void testLoadSBZ();

private:
};


#endif /* TESTKOCOLORSET_H */
