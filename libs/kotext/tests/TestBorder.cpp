#include "TestBorder.h"

#include "styles/KoParagraphStyle.h"
#include "KoTextBlockBorderData.h"

void TestBorder::testBorder()
{
    KoParagraphStyle style;
    style.setLeftBorderWidth(4.0);
    style.setLeftBorderSpacing(2.0);
    style.setLeftInnerBorderWidth(1.0);
    style.setTopBorderWidth(6.0);
    style.setTopBorderSpacing(7.0);
    style.setTopInnerBorderWidth(8.0);
    QTextBlockFormat format;
    style.applyStyle(format);

    KoTextBlockBorderData data(QRectF(10, 10, 100, 100));
    QCOMPARE(QRectF(10, 10, 100, 100), data.rect());
    data.setParagraphBottom(90);
    QCOMPARE(QRectF(10, 10, 100, 80), data.rect());

    data.setEdge(KoTextBlockBorderData::Left, format, KoParagraphStyle::LeftBorderStyle,
                 KoParagraphStyle::LeftBorderWidth, KoParagraphStyle::LeftBorderColor,
                 KoParagraphStyle::LeftBorderSpacing, KoParagraphStyle::LeftInnerBorderWidth);
    data.setEdge(KoTextBlockBorderData::Top, format, KoParagraphStyle::TopBorderStyle,
                 KoParagraphStyle::TopBorderWidth, KoParagraphStyle::TopBorderColor,
                 KoParagraphStyle::TopBorderSpacing, KoParagraphStyle::TopInnerBorderWidth);

    QCOMPARE(QRectF(10, 10, 100, 80), data.rect());
    QCOMPARE(7., data.inset(KoTextBlockBorderData::Left));
    QCOMPARE(0., data.inset(KoTextBlockBorderData::Right));
    QCOMPARE(21., data.inset(KoTextBlockBorderData::Top));
    QCOMPARE(0., data.inset(KoTextBlockBorderData::Bottom));

    KoInsets insets;
    data.applyInsets(insets, 0, false);
    QCOMPARE(insets.left, 7.);
    QCOMPARE(insets.right, 0.);
    QCOMPARE(insets.top, 0.);
    QCOMPARE(insets.bottom, 0.);

    KoInsets insets2;
    data.applyInsets(insets2, 31, true);
    QCOMPARE(insets2.left, 7.);
    QCOMPARE(insets2.top, 21.);

    KoInsets insets3;
    data.applyInsets(insets3, 10, false);
    QCOMPARE(insets3.left, 7.);
    QCOMPARE(insets3.top, 21.);
}

QTEST_MAIN(TestBorder)
#include <TestBorder.moc>
