/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_MASK_TEST_H
#define KIS_MASK_TEST_H

#include <QtTest>

class KisMaskTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testCreation();
    void testSelection();
    void testCropUpdateBySelection();
    void testSelectionParent();

    void testDeferredOffsetInitialization();
};

#endif
