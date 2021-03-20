/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef ZOOMCONTROLLER_TEST_H
#define ZOOMCONTROLLER_TEST_H

#include <QObject>

class zoomcontroller_test : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    // tests
    void testApi();
};

#endif

