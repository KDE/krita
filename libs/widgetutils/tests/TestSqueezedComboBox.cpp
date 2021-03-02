/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "TestSqueezedComboBox.h"
#include <simpletest.h>

#include <KisSqueezedComboBox.h>

void TestSqueezedComboBox::testContains()
{
    KisSqueezedComboBox *comboBox = new KisSqueezedComboBox();

    const int comboBoxWidth = 10;
    const int comboBoxHeight = 50;

    comboBox->resize(comboBoxWidth, comboBoxHeight);

    const QString testItemText("A test item name");

    comboBox->addSqueezedItem(testItemText);

    const int testItemIndex = 0;
    const QString squeezedItemText = comboBox->itemText(testItemIndex);

    Q_ASSERT(squeezedItemText != testItemText);

    QVERIFY(comboBox->contains(testItemText));
}

SIMPLE_TEST_MAIN(TestSqueezedComboBox)

