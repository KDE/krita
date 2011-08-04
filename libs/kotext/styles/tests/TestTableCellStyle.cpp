#include "TestTableCellStyle.h"

#include "styles/KoTableStyle.h"
#include "styles/KoTableCellStyle.h"

#include <QTextTableCellFormat>
#include <QRectF>

void TestTableCellStyle::testPen()
{
    // Test basic functionality of the table cell style (roundtripping to format).
    QTextTableCellFormat format1;
    format1.setProperty(KoTableBorderStyle::TopBorderOuterPen, QPen(Qt::red, 5.0));
    format1.setProperty(KoTableBorderStyle::TopBorderInnerPen, QPen(Qt::red, 7.0));
    format1.setProperty(KoTableBorderStyle::LeftBorderOuterPen, QPen(Qt::red, 8.0));
    format1.setProperty(KoTableBorderStyle::LeftBorderInnerPen, QPen(Qt::red, 10.0));
    format1.setProperty(KoTableBorderStyle::BottomBorderOuterPen, QPen(Qt::red, 11.0));
    format1.setProperty(KoTableBorderStyle::BottomBorderInnerPen, QPen(Qt::red, 13.0));
    format1.setProperty(KoTableBorderStyle::RightBorderOuterPen, QPen(Qt::red, 14.0));
    format1.setProperty(KoTableBorderStyle::RightBorderInnerPen, QPen(Qt::red, 16.0));

    KoTableCellStyle *style = new KoTableCellStyle(format1);
    QVERIFY(style);
    QTextTableCellFormat format2;
    style->applyStyle(format2);

    QCOMPARE(format2.penProperty(KoTableBorderStyle::TopBorderOuterPen), QPen(Qt::red, 5.0));
    QCOMPARE(format2.penProperty(KoTableBorderStyle::TopBorderInnerPen), QPen(Qt::red, 7.0));
    QCOMPARE(format2.penProperty(KoTableBorderStyle::LeftBorderOuterPen), QPen(Qt::red, 8.0));
    QCOMPARE(format2.penProperty(KoTableBorderStyle::LeftBorderInnerPen), QPen(Qt::red, 10.0));
    QCOMPARE(format2.penProperty(KoTableBorderStyle::BottomBorderOuterPen), QPen(Qt::red, 11.0));
    QCOMPARE(format2.penProperty(KoTableBorderStyle::BottomBorderInnerPen), QPen(Qt::red, 13.0));
    QCOMPARE(format2.penProperty(KoTableBorderStyle::RightBorderOuterPen), QPen(Qt::red, 14.0));
    QCOMPARE(format2.penProperty(KoTableBorderStyle::RightBorderInnerPen), QPen(Qt::red, 16.0));
}

void TestTableCellStyle::testPadding()
{
    // Test basic functionality of the table cell style (roundtripping to format).
    QTextTableCellFormat format1;
    format1.setLeftPadding(1.0);
    format1.setRightPadding(2.0);
    format1.setTopPadding(3.0);
    format1.setBottomPadding(4.0);

    KoTableCellStyle *style = new KoTableCellStyle(format1);
    QVERIFY(style);
    QTextTableCellFormat format2;
    style->applyStyle(format2);

    QCOMPARE(format2.leftPadding(), 1.0);
    QCOMPARE(format2.rightPadding(), 2.0);
    QCOMPARE(format2.topPadding(), 3.0);
    QCOMPARE(format2.bottomPadding(), 4.0);

    QRectF rect(0.0, 0.0, 100.0, 100.0);
    QCOMPARE(style->contentRect(rect), QRectF(1.0, 3.0, 97.0, 93.0));
    QCOMPARE(style->boundingRect(rect), QRectF(-1.0, -3.0, 103.0, 107.0));
}

void TestTableCellStyle::testSpacing()
{
    // Test basic functionality of the table cell style (roundtripping to format).
    QTextTableCellFormat format1;
    format1.setProperty(KoTableBorderStyle::LeftBorderSpacing, 9.0);
    format1.setProperty(KoTableBorderStyle::RightBorderSpacing, 15.0);
    format1.setProperty(KoTableBorderStyle::TopBorderSpacing, 6.0);
    format1.setProperty(KoTableBorderStyle::BottomBorderSpacing, 12.0);

    KoTableCellStyle *style = new KoTableCellStyle(format1);
    QVERIFY(style);
    QTextTableCellFormat format2;
    style->applyStyle(format2);

    QCOMPARE(format2.doubleProperty(KoTableBorderStyle::LeftBorderSpacing), 9.0);
    QCOMPARE(format2.doubleProperty(KoTableBorderStyle::RightBorderSpacing), 15.0);
    QCOMPARE(format2.doubleProperty(KoTableBorderStyle::TopBorderSpacing), 6.0);
    QCOMPARE(format2.doubleProperty(KoTableBorderStyle::BottomBorderSpacing), 12.0);

    QRectF rect(0.0, 0.0, 100.0, 100.0);
    QCOMPARE(style->contentRect(rect), QRectF(9.0, 6.0, 76.0, 82.0));
    QCOMPARE(style->boundingRect(rect), QRectF(-9.0, -6.0, 124.0, 118.0));
}

void TestTableCellStyle::testMargin()
{
    QTextTableFormat format1;
    format1.setProperty(QTextFormat::FrameLeftMargin, 4.0);
    format1.setProperty(QTextFormat::FrameRightMargin, 8.0);
    format1.setProperty(QTextFormat::FrameTopMargin, 9.0);
    format1.setProperty(QTextFormat::FrameBottomMargin, 3.0);

    KoTableStyle *style = new KoTableStyle(format1);
    QVERIFY(style);

    QCOMPARE(style->leftMargin(), 4.0);
    QCOMPARE(style->rightMargin(), 8.0);
    QCOMPARE(style->topMargin(), 9.0);
    QCOMPARE(style->bottomMargin(), 3.0);

    style->setLeftMargin(QTextLength(QTextLength::FixedLength, 14.0));
    style->setRightMargin(QTextLength(QTextLength::FixedLength, 18.0));
    style->setTopMargin(QTextLength(QTextLength::FixedLength, 19.0));
    style->setBottomMargin(QTextLength(QTextLength::FixedLength, 13.0));

    QTextTableFormat format2;
    style->applyStyle(format2);

    QCOMPARE(format2.doubleProperty(QTextFormat::FrameLeftMargin), 14.0);
    QCOMPARE(format2.doubleProperty(QTextFormat::FrameRightMargin), 18.0);
    QCOMPARE(format2.doubleProperty(QTextFormat::FrameTopMargin), 19.0);
    QCOMPARE(format2.doubleProperty(QTextFormat::FrameBottomMargin), 13.0);
}

QTEST_MAIN(TestTableCellStyle)
#include <TestTableCellStyle.moc>
