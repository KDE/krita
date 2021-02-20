/*
 *  SPDX-FileCopyrightText: 2019 Boudewijn Rempt <boud@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISDIALOGSTATESAVERTEST_H
#define KISDIALOGSTATESAVERTEST_H

#include <QTest>
#include "ui_dialogsavertestwidget.h"

class KisDialogStateSaverTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSave();
    void testRestore();
};

#endif // KISDIALOGSTATESAVERTEST_H
