#include "TestTableCellStyle.h"

#include "styles/KoTableStyle.h"
#include "styles/KoTableCellStyle.h"

#include <QTextTableCellFormat>
#include <QRectF>
#include <QTest>

void TestTableCellStyle::testPen()
{
    // Test basic functionality of the table cell style (roundtripping to format).
    QTextTableCellFormat format1;
    KoBorder border;
    KoBorder::BorderData data;
    data.outerPen = QPen(Qt::red, 5.0);
    data.innerPen = QPen(Qt::red, 7.0);
    border.setBorderData(KoBorder::TopBorder, data);

    data.outerPen = QPen(Qt::red, 8.0);
    data.innerPen = QPen(Qt::red, 10.0);
    border.setBorderData(KoBorder::LeftBorder, data);

    data.outerPen = QPen(Qt::red, 11.0);
    data.innerPen = QPen(Qt::red, 13.0);
    border.setBorderData(KoBorder::BottomBorder, data);

    data.outerPen = QPen(Qt::red, 14.0);
    data.innerPen = QPen(Qt::red, 16.0);
    border.setBorderData(KoBorder::RightBorder, data);

    format1.setProperty(KoTableCellStyle::Borders, QVariant::fromValue<KoBorder>(border));

    KoTableCellStyle *style = new KoTableCellStyle(format1);
    QVERIFY(style);
    QTextTableCellFormat format2;
    style->applyStyle(format2);
    KoBorder border2 = format2.property(KoTableCellStyle::Borders).value<KoBorder>();
    QCOMPARE(border2.borderData(KoBorder::TopBorder).outerPen, QPen(Qt::red, 5.0));
    QCOMPARE(border2.borderData(KoBorder::TopBorder).innerPen, QPen(Qt::red, 7.0));
    QCOMPARE(border2.borderData(KoBorder::LeftBorder).outerPen, QPen(Qt::red, 8.0));
    QCOMPARE(border2.borderData(KoBorder::LeftBorder).innerPen, QPen(Qt::red, 10.0));
    QCOMPARE(border2.borderData(KoBorder::BottomBorder).outerPen, QPen(Qt::red, 11.0));
    QCOMPARE(border2.borderData(KoBorder::BottomBorder).innerPen, QPen(Qt::red, 13.0));
    QCOMPARE(border2.borderData(KoBorder::RightBorder).outerPen, QPen(Qt::red, 14.0));
    QCOMPARE(border2.borderData(KoBorder::RightBorder).innerPen, QPen(Qt::red, 16.0));
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
    KoBorder border1;
    border1.setBorderSpacing(KoBorder::LeftBorder, 9.0);
    border1.setBorderSpacing(KoBorder::RightBorder, 15.0);
    border1.setBorderSpacing(KoBorder::TopBorder, 6.0);
    border1.setBorderSpacing(KoBorder::BottomBorder, 12.0);
    format1.setProperty(KoTableCellStyle::Borders, QVariant::fromValue<KoBorder>(border1));

    KoTableCellStyle *style = new KoTableCellStyle(format1);
    QVERIFY(style);
    QTextTableCellFormat format2;
    style->applyStyle(format2);

    KoBorder border2 = format2.property(KoTableCellStyle::Borders).value<KoBorder>();
    QCOMPARE(border2.borderSpacing(KoBorder::LeftBorder), 9.0);
    QCOMPARE(border2.borderSpacing(KoBorder::RightBorder), 15.0);
    QCOMPARE(border2.borderSpacing(KoBorder::TopBorder), 6.0);
    QCOMPARE(border2.borderSpacing(KoBorder::BottomBorder), 12.0);

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
