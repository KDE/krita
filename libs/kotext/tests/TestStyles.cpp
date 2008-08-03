#include "TestStyles.h"

#include "styles/KoParagraphStyle.h"
//   #include "styles/KoListStyle.h"
//   #include "styles/KoStyleManager.h"
//   #include "KoTextBlockData.h"
//   #include "KoTextBlockBorderData.h"

//#include <QtGui>

//#include <kdebug.h>
//#include <kcomponentdata.h>

void TestStyles::testApplyParagraphStyle()
{
    KoParagraphStyle style;
    style.setStyleId(1001);

    QTextBlockFormat format;
    QCOMPARE(format.properties().count(), 0);
    style.applyStyle(format);
    QCOMPARE(format.properties().count(), 1); // the styleId

    style.setName("name");
    style.setAlignment(Qt::AlignRight);
    style.applyStyle(format);
    QCOMPARE(format.properties().count(), 2);
    QCOMPARE(format.alignment(), Qt::AlignRight);
}

void TestStyles::testApplyParagraphStyleWithParent()
{
    KoParagraphStyle style1;
    style1.setStyleId(1002);
    KoParagraphStyle style2;
    style2.setStyleId(1003);
    KoParagraphStyle style3;
    style3.setStyleId(1004);

    style3.setParent(&style2);
    style2.setParent(&style1);

    style1.setAlignment(Qt::AlignRight);
    QCOMPARE(style1.alignment(), Qt::AlignRight);
    QCOMPARE(style2.alignment(), Qt::AlignRight);
    QCOMPARE(style3.alignment(), Qt::AlignRight);

    style2.setAlignment(Qt::AlignCenter);
    QCOMPARE(style1.alignment(), Qt::AlignRight);
    QCOMPARE(style2.alignment(), Qt::AlignCenter);
    QCOMPARE(style3.alignment(), Qt::AlignCenter);

    style3.setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    QCOMPARE(style1.alignment(), Qt::AlignRight);
    QCOMPARE(style2.alignment(), Qt::AlignCenter);
    QCOMPARE(style3.alignment(), Qt::AlignLeft | Qt::AlignAbsolute);

    style1.setLeftMargin(10.);
    QCOMPARE(style1.leftMargin(), 10.);
    QCOMPARE(style2.leftMargin(), 10.);
    QCOMPARE(style3.leftMargin(), 10.);
    style2.setRightMargin(20.);
    QCOMPARE(style1.rightMargin(), 0.);
    QCOMPARE(style2.rightMargin(), 20.);
    QCOMPARE(style3.rightMargin(), 20.);

    // now actually apply it.
    QTextBlockFormat format;
    style1.applyStyle(format);
    QCOMPARE(format.properties().count(), 3);
    QCOMPARE(format.alignment(), Qt::AlignRight);
    QCOMPARE(format.property(KoParagraphStyle::StyleId).toInt(), 1002);
    QCOMPARE(format.leftMargin(), 10.);
    QCOMPARE(format.rightMargin(), 0.);

    style2.applyStyle(format);
    QCOMPARE(format.properties().count(), 4);
    QCOMPARE(format.alignment(), Qt::AlignCenter);
    QCOMPARE(format.property(KoParagraphStyle::StyleId).toInt(), 1003);
    QCOMPARE(format.leftMargin(), 10.);
    QCOMPARE(format.rightMargin(), 20.);

    style3.applyStyle(format);
    QCOMPARE(format.properties().count(), 4);
    QCOMPARE(format.alignment(), Qt::AlignLeft | Qt::AlignAbsolute);
    QCOMPARE(format.property(KoParagraphStyle::StyleId).toInt(), 1004);
    QCOMPARE(format.leftMargin(), 10.);
    QCOMPARE(format.rightMargin(), 20.);
}

QTEST_KDEMAIN(TestStyles,GUI)

#include "TestStyles.moc"
