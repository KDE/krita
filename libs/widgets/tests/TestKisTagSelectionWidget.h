/*
 *  Author 2021 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __TEST_KIS_TAG_SELECTION_WIDGET_H
#define __TEST_KIS_TAG_SELECTION_WIDGET_H

#include <QtTest>


#include "KisTagSelectionWidget.h"

class TestKisTagSelectionWidget : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test();


private:
    QList<KoID> createAvailableTags();
    QList<KoID> createSelectedTags();


    WdgCloseableLabel* firstCloseableLabel(QLayout* layout);

};

#endif /* __KO_ANCHOR_SELECTION_WIDGET_TEST_H */
