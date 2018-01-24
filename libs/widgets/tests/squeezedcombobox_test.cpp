


#include "squeezedcombobox_test.h"
#include <QTest>

#include <squeezedcombobox.h>

void SqueezedComboBoxTest::testContains()
{
    SqueezedComboBox *comboBox = new SqueezedComboBox();

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

QTEST_MAIN(SqueezedComboBoxTest)

