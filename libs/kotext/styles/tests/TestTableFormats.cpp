#include "TestTableFormats.h"

#include "styles/KoTableColumnFormat.h"
#include "styles/KoTableRowFormat.h"

#include <QBrush>

void TestTableFormats::testTableColumnFormat()
{
    // Basic functionality testing.
    KoTableColumnFormat columnFormat;

    columnFormat.setBreakAfter(true);
    QCOMPARE(columnFormat.breakAfter(), true);
    columnFormat.setBreakAfter(false);
    QCOMPARE(columnFormat.breakAfter(), false);

    columnFormat.setBreakBefore(true);
    QCOMPARE(columnFormat.breakBefore(), true);
    columnFormat.setBreakBefore(false);
    QCOMPARE(columnFormat.breakBefore(), false);

    columnFormat.setColumnWidth(5.0);
    QCOMPARE(columnFormat.columnWidth(), 5.0);
    columnFormat.setColumnWidth(10.0);
    QCOMPARE(columnFormat.columnWidth(), 10.0);

    columnFormat.setRelativeColumnWidth(5.0);
    QCOMPARE(columnFormat.relativeColumnWidth(), 5.0);
    columnFormat.setRelativeColumnWidth(10.0);
    QCOMPARE(columnFormat.relativeColumnWidth(), 10.0);

    QCOMPARE(columnFormat.properties().count(), 4);
}

void TestTableFormats::testTableRowFormat()
{
    // Basic functionality testing.
    KoTableRowFormat rowFormat;

    rowFormat.setBreakAfter(true);
    QCOMPARE(rowFormat.breakAfter(), true);
    rowFormat.setBreakAfter(false);
    QCOMPARE(rowFormat.breakAfter(), false);

    rowFormat.setBreakBefore(true);
    QCOMPARE(rowFormat.breakBefore(), true);
    rowFormat.setBreakBefore(false);
    QCOMPARE(rowFormat.breakBefore(), false);

    rowFormat.setBackground(QBrush(Qt::red));
    QCOMPARE(rowFormat.background(), QBrush(Qt::red));
    rowFormat.setBackground(QBrush(Qt::green));
    QCOMPARE(rowFormat.background(), QBrush(Qt::green));

    rowFormat.setMinimumHeight(5.0);
    QCOMPARE(rowFormat.minimumHeight(), 5.0);
    rowFormat.setMinimumHeight(10.0);
    QCOMPARE(rowFormat.minimumHeight(), 10.0);

    rowFormat.setKeepTogether(true);
    QCOMPARE(rowFormat.keepTogether(), true);
    rowFormat.setKeepTogether(false);
    QCOMPARE(rowFormat.keepTogether(), false);

    QCOMPARE(rowFormat.properties().count(), 5);
}

QTEST_MAIN(TestTableFormats)
#include <TestTableFormats.moc>
