/*
 *  SPDX-FileCopyrightText: 2019 Boudewijn Rempt <boud@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisDialogStateSaverTest.h"
#include <QTest>
#include <KisDialogStateSaver.h>
#include <QWidget>
#include <ksharedconfig.h>
#include <kconfiggroup.h>


void KisDialogStateSaverTest::testSave()
{
    QWidget w;
    Ui::DialogSaverTestWidget page;
    page.setupUi(&w);

    page.lineEdit->setText("test");
    page.spinBox->setValue(5);
    page.doubleSpinBox->setValue(3.0);
    page.verticalSlider->setValue(10);
    page.checkBox->setChecked(true);
    KisDialogStateSaver::saveState(&w, "StateSaverTest");
    KConfigGroup group(KSharedConfig::openConfig(), "StateSaverTest");
    QCOMPARE(group.readEntry("lineEdit", QString()), QString("test"));
    QCOMPARE(group.readEntry("spinBox", 0), 5);
    QCOMPARE(group.readEntry("doubleSpinBox", 0.0), 3.0);
    QCOMPARE(group.readEntry("verticalSlider", 0), 10);
    QCOMPARE(group.readEntry("checkBox", false), true);
}

void KisDialogStateSaverTest::testRestore()
{
    QWidget w;
    Ui::DialogSaverTestWidget page;
    page.setupUi(&w);
    QMap<QString, QVariant> overrideMap;

    overrideMap["spinBox"] = QVariant::fromValue<int>(10);

    KisDialogStateSaver::restoreState(&w, "StateSaverTest", overrideMap);

    QCOMPARE(page.lineEdit->text(), QString("test"));
    QCOMPARE(page.spinBox->value(), 10);
    QCOMPARE(page.doubleSpinBox->value(), 3.0);
    QCOMPARE(page.verticalSlider->value(), 10);
    QCOMPARE(page.checkBox->isChecked(), true);

}


QTEST_MAIN(KisDialogStateSaverTest)
