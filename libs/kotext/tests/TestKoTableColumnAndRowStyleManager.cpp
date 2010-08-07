#include "TestKoTableColumnAndRowStyleManager.h"

#include "styles/KoTableColumnStyle.h"
#include "../KoTableColumnAndRowStyleManager.h"

void TestKoTableColumnAndRowStyleManager::testManager()
{
    KoTableColumnAndRowStyleManager manager;

    KoTableColumnStyle *style1 = new KoTableColumnStyle();
    KoTableColumnStyle *style2 = new KoTableColumnStyle();

    manager.setColumnStyle(0, *style1);
    manager.setColumnStyle(2, *style2);

    QVERIFY(manager.columnStyle(0) == *style1);
    QVERIFY(manager.columnStyle(2) == *style2);
}

QTEST_MAIN(TestKoTableColumnAndRowStyleManager)
#include <TestKoTableColumnAndRowStyleManager.moc>
