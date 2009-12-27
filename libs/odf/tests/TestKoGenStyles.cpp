/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
   Copyright (C) 2009 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "TestKoGenStyles.h"

#include <KoGenStyles.h>
#include <KoXmlWriter.h>
#include <KDebug>
#include <QList>
#include <QBuffer>
#include <QRegExp>


#define TEST_BEGIN(publicId,systemId) \
    { \
        QByteArray cstr; \
        QBuffer buffer( &cstr ); \
        buffer.open( QIODevice::WriteOnly ); \
        { \
            KoXmlWriter writer( &buffer ); \
            writer.startDocument( "r", publicId, systemId ); \
            writer.startElement( "r" )

#define TEST_END_QTTEST(expected) \
            writer.endElement(); \
            writer.endDocument(); \
        } \
        buffer.putChar( '\0' ); /*null-terminate*/ \
        QString expectedFull = QString::fromLatin1( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" ); \
        expectedFull += expected; \
        QString s1 = QString::fromLatin1( cstr ); \
        QCOMPARE( expectedFull, s1 ); \
    }


void TestKoGenStyles::testLookup()
{
    kDebug() ;
    KoGenStyles coll;

    QMap<QString, QString> map1;
    map1.insert("map1key", "map1value");
    QMap<QString, QString> map2;
    map2.insert("map2key1", "map2value1");
    map2.insert("map2key2", "map2value2");

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    KoXmlWriter childWriter(&buffer);
    childWriter.startElement("child");
    childWriter.addAttribute("test:foo", "bar");
    childWriter.endElement();
    QString childContents = QString::fromUtf8(buffer.buffer(), buffer.buffer().size());

    KoGenStyle first(KoGenStyle::StyleAuto, "paragraph");
    first.addAttribute("style:master-page-name", "Standard");
    first.addProperty("style:page-number", "0");
    first.addProperty("style:foobar", "2", KoGenStyle::TextType);
    first.addStyleMap(map1);
    first.addStyleMap(map2);
    first.addChildElement("test", childContents);

    QString firstName = coll.lookup(first);
    kDebug() << "The first style got assigned the name" << firstName;
    QVERIFY(!firstName.isEmpty());
    QCOMPARE(first.type(), KoGenStyle::StyleAuto);

    KoGenStyle second(KoGenStyle::StyleAuto, "paragraph");
    second.addAttribute("style:master-page-name", "Standard");
    second.addProperty("style:page-number", "0");
    second.addProperty("style:foobar", "2", KoGenStyle::TextType);
    second.addStyleMap(map1);
    second.addStyleMap(map2);
    second.addChildElement("test", childContents);

    QString secondName = coll.lookup(second);
    kDebug() << "The second style got assigned the name" << secondName;

    QCOMPARE(firstName, secondName);   // check that sharing works
    QCOMPARE(first, second);   // check that operator== works :)

    const KoGenStyle* s = coll.style(firstName);   // check lookup of existing style
    QVERIFY(s != 0);
    QCOMPARE(*s, first);
    s = coll.style("foobarblah");   // check lookup of non-existing style
    QVERIFY(s == 0);

    KoGenStyle third(KoGenStyle::StyleAuto, "paragraph", secondName);   // inherited style
    third.addProperty("style:margin-left", "1.249cm");
    third.addProperty("style:page-number", "0");   // same as parent
    third.addProperty("style:foobar", "3", KoGenStyle::TextType);   // different from parent
    QCOMPARE(third.parentName(), secondName);

    QString thirdName = coll.lookup(third, "P");
    kDebug() << "The third style got assigned the name" << thirdName;
    QVERIFY(thirdName != firstName);
    QVERIFY(!thirdName.isEmpty());

    KoGenStyle user(KoGenStyle::StyleUser, "paragraph");   // differs from third since it doesn't inherit second, and has a different type
    user.addProperty("style:margin-left", "1.249cm");

    QString userStyleName = coll.lookup(user, "User", KoGenStyles::DontForceNumbering);
    kDebug() << "The user style got assigned the name" << userStyleName;
    QCOMPARE(userStyleName, QString("User"));

    KoGenStyle sameAsParent(KoGenStyle::StyleAuto, "paragraph", secondName);   // inherited style
    sameAsParent.addAttribute("style:master-page-name", "Standard");
    sameAsParent.addProperty("style:page-number", "0");
    sameAsParent.addProperty("style:foobar", "2", KoGenStyle::TextType);
    sameAsParent.addStyleMap(map1);
    sameAsParent.addStyleMap(map2);
    sameAsParent.addChildElement("test", childContents);
    QString sapName = coll.lookup(sameAsParent, "foobar");
    kDebug() << "The 'same as parent' style got assigned the name" << sapName;

    QCOMPARE(sapName, secondName);
    QCOMPARE(coll.styles().count(), 3);

    // OK, now add a style marked as for styles.xml; it looks like the above style, but
    // since it's marked for styles.xml it shouldn't be shared with it.
    KoGenStyle headerStyle(KoGenStyle::StyleAuto, "paragraph");
    headerStyle.addAttribute("style:master-page-name", "Standard");
    headerStyle.addProperty("style:page-number", "0");
    headerStyle.addProperty("style:foobar", "2", KoGenStyle::TextType);
    headerStyle.addStyleMap(map1);
    headerStyle.addStyleMap(map2);
    headerStyle.setAutoStyleInStylesDotXml(true);
    QString headerStyleName = coll.lookup(headerStyle, "foobar");

    QCOMPARE(coll.styles().count(), 4);
    QCOMPARE(coll.styles(KoGenStyle::StyleAuto).count(), 2);
    QCOMPARE(coll.styles(KoGenStyle::StyleUser).count(), 1);

    QList<KoGenStyles::NamedStyle> stylesXmlStyles = coll.styles(KoGenStyle::StyleAuto, true);
    QCOMPARE(stylesXmlStyles.count(), 1);
    KoGenStyles::NamedStyle firstStyle = stylesXmlStyles.first();
    QCOMPARE(firstStyle.name, headerStyleName);

    // XML for first/second style
    TEST_BEGIN(0, 0);
    first.writeStyle(&writer, coll, "style:style", firstName, "style:paragraph-properties");


    TEST_END_QTTEST("<r>\n <style:style style:name=\"" + firstName + "\" style:family=\"paragraph\" "
        "style:master-page-name=\"Standard\">\n  <style:paragraph-properties style:page-number=\"0\">\n"
        "   <child test:foo=\"bar\"/>\n  </style:paragraph-properties>\n  <style:text-properties style:foobar=\"2\"/>\n"
        "  <style:map map1key=\"map1value\"/>\n  <style:map map2key1=\"map2value1\" map2key2=\"map2value2\"/>\n"
        " </style:style>\n</r>\n");

    // XML for third style
    TEST_BEGIN(0, 0);
    third.writeStyle(&writer, coll, "style:style", thirdName, "style:paragraph-properties");
    TEST_END_QTTEST("<r>\n <style:style style:name=\"" + thirdName + "\""
        " style:parent-style-name=\"" + firstName + "\" style:family=\"paragraph\">\n"
        "  <style:paragraph-properties style:margin-left=\"1.249cm\"/>\n"
        "  <style:text-properties style:foobar=\"3\"/>\n </style:style>\n</r>\n");

    coll.markStyleForStylesXml(firstName);
    {
        QList<KoGenStyles::NamedStyle> stylesXmlStyles = coll.styles(KoGenStyle::StyleAuto, true);
        QCOMPARE(stylesXmlStyles.count(), 2);
        QList<KoGenStyles::NamedStyle> contentXmlStyles = coll.styles(KoGenStyle::StyleAuto, false);
        QCOMPARE(contentXmlStyles.count(), 1);
    }
}

void TestKoGenStyles::testLookupFlags()
{
    KoGenStyles coll;

    KoGenStyle first(KoGenStyle::StyleAuto, "paragraph");
    first.addAttribute("style:master-page-name", "Standard");
    first.addProperty("style:page-number", "0");

    QString styleName = coll.lookup(first, "P", KoGenStyles::DontForceNumbering);
    QCOMPARE(styleName, QString("P"));

    styleName = coll.lookup(first, "P", KoGenStyles::ForceNumbering);
    QCOMPARE(styleName, QString("P"));

    KoGenStyle second(KoGenStyle::StyleAuto, "paragraph");
    second.addProperty("fo:text-align", "left");

    styleName = coll.lookup(second, "P", KoGenStyles::ForceNumbering);
    QCOMPARE(styleName, QString("P1"));

    styleName = coll.lookup(second, "P", KoGenStyles::AllowDuplicates);
    QCOMPARE(styleName, QString("P2"));

    styleName = coll.lookup(second, "P", KoGenStyles::AllowDuplicates | KoGenStyles::ForceNumbering);
    QCOMPARE(styleName, QString("P3"));

    styleName = coll.lookup(second, "P", KoGenStyles::AllowDuplicates | KoGenStyles::DontForceNumbering);
    QCOMPARE(styleName, QString("P4"));
}

void TestKoGenStyles::testWriteStyle()
{
    kDebug();
    KoGenStyles coll;

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    KoXmlWriter styleChildWriter(&buffer);
    styleChildWriter.startElement("styleChild");
    styleChildWriter.addAttribute("foo", "bar");
    styleChildWriter.endElement();
    QString styleChildContents = QString::fromUtf8(buffer.buffer(), buffer.buffer().size());

    KoGenStyle style(KoGenStyle::StyleUser, "paragraph");
    style.addProperty("style:foo", "bar");
    style.addProperty("style:paragraph", "property", KoGenStyle::ParagraphType);
    style.addProperty("style:graphic", "property", KoGenStyle::GraphicType);
    style.addProperty("styleChild", styleChildContents, KoGenStyle::StyleChildElement);
    QString styleName = coll.lookup(style, "P");

    // XML for style
    TEST_BEGIN(0, 0);
    style.writeStyle(&writer, coll, "style:style", styleName, "style:paragraph-properties");
    TEST_END_QTTEST("<r>\n <style:style style:name=\"P1\" style:family=\"paragraph\">\n  <style:paragraph-properties style:foo=\"bar\" style:paragraph=\"property\"/>\n  <style:graphic-properties style:graphic=\"property\"/>\n  <styleChild foo=\"bar\"/>\n </style:style>\n</r>\n");

    KoGenStyle pageLayoutStyle(KoGenStyle::StylePageLayout);
    pageLayoutStyle.addProperty("style:print-orientation", "portrait");
    QString pageLayoutStyleName = coll.lookup(pageLayoutStyle, "pm");

    // XML for page layout style
    TEST_BEGIN(0, 0);
    pageLayoutStyle.writeStyle(&writer, coll, "style:page-layout", pageLayoutStyleName, "style:page-layout-properties");
    TEST_END_QTTEST("<r>\n <style:page-layout style:name=\"pm1\">\n  <style:page-layout-properties style:print-orientation=\"portrait\"/>\n </style:page-layout>\n</r>\n");

    KoGenStyle listStyle(KoGenStyle::StyleList);
    QString listStyleName = coll.lookup(listStyle, "L");
    // XML for list layout style
    TEST_BEGIN(0, 0);
    listStyle.writeStyle(&writer, coll, "text:list-style", listStyleName, 0);
    TEST_END_QTTEST("<r>\n <text:list-style style:name=\"L1\"/>\n</r>\n");
}

void TestKoGenStyles::testDefaultStyle()
{
    kDebug() ;
    /* Create a default style,
     * and then an auto style with exactly the same attributes
     * -> the lookup gives the default style.
     *
     * Also checks how the default style gets written out to XML.
     */
    KoGenStyles coll;

    KoGenStyle defaultStyle(KoGenStyle::StyleUser, "paragraph");
    defaultStyle.addAttribute("style:master-page-name", "Standard");
    defaultStyle.addProperty("myfont", "isBold");
    defaultStyle.setDefaultStyle(true);
    QString defaultStyleName = coll.lookup(defaultStyle);
    // default styles don't get a name
    QVERIFY(defaultStyleName.isEmpty());
    QCOMPARE(defaultStyle.type(), KoGenStyle::StyleUser);
    QVERIFY(defaultStyle.isDefaultStyle());

    KoGenStyle anotherStyle(KoGenStyle::StyleUser, "paragraph");
    anotherStyle.addAttribute("style:master-page-name", "Standard");
    anotherStyle.addProperty("myfont", "isBold");
    QString anotherStyleName = coll.lookup(anotherStyle);
    QVERIFY(anotherStyleName != defaultStyleName);

    QCOMPARE(coll.styles().count(), 1);

    // XML for default style
    TEST_BEGIN(0, 0);
    defaultStyle.writeStyle(&writer, coll, "style:default-style", defaultStyleName, "style:paragraph-properties");
    TEST_END_QTTEST("<r>\n <style:default-style style:family=\"paragraph\" style:master-page-name=\"Standard\">\n  <style:paragraph-properties myfont=\"isBold\"/>\n </style:default-style>\n</r>\n");

    // The kspread case: not writing out all properties, only if they differ
    // from the default style.
    // KoGenStyles doesn't fetch info from the parent style when testing
    // for equality, so KSpread uses isEmpty() to check for equality-to-parent.
    KoGenStyle dataStyle(KoGenStyle::StyleUser, "paragraph", defaultStyleName);
    QVERIFY(dataStyle.isEmpty());
    // and then it doesn't look up the auto style, but rather uses the parent style directly.
}

void TestKoGenStyles:: testUserStyles()
{
    kDebug() ;
    /* Two user styles with exactly the same attributes+properties will not get merged, since
     * they don't have exactly the same attributes after all: the display-name obviously differs :)
     */
    KoGenStyles coll;

    KoGenStyle user1(KoGenStyle::StyleUser, "paragraph");
    user1.addAttribute("style:display-name", "User 1");
    user1.addProperty("myfont", "isBold");

    QString user1StyleName = coll.lookup(user1, "User1", KoGenStyles::DontForceNumbering);
    kDebug() << "The user style got assigned the name" << user1StyleName;
    QCOMPARE(user1StyleName, QString("User1"));

    KoGenStyle user2(KoGenStyle::StyleUser, "paragraph");
    user2.addAttribute("style:display-name", "User 2");
    user2.addProperty("myfont", "isBold");

    QString user2StyleName = coll.lookup(user2, "User2", KoGenStyles::DontForceNumbering);
    kDebug() << "The user style got assigned the name" << user2StyleName;
    QCOMPARE(user2StyleName, QString("User2"));

    // And now, what if the data uses that style?
    // This is like sameAsParent in the other test, but this time the
    // parent is a STYLE_USER...
    KoGenStyle dataStyle(KoGenStyle::StyleAuto, "paragraph", user2StyleName);
    dataStyle.addProperty("myfont", "isBold");

    QString dataStyleName = coll.lookup(dataStyle, "DataStyle");
    kDebug() << "The auto style got assigned the name" << dataStyleName;
    QCOMPARE(dataStyleName, QString("User2"));     // it found the parent as equal

    // Let's do the opposite test, just to make sure
    KoGenStyle dataStyle2(KoGenStyle::StyleAuto, "paragraph", user2StyleName);
    dataStyle2.addProperty("myfont", "isNotBold");

    QString dataStyle2Name = coll.lookup(dataStyle2, "DataStyle");
    kDebug() << "The different auto style got assigned the name" << dataStyle2Name;
    QCOMPARE(dataStyle2Name, QString("DataStyle1"));

    QCOMPARE(coll.styles().count(), 3);

    // XML for user style 1
    TEST_BEGIN(0, 0);
    user1.writeStyle(&writer, coll, "style:style", user1StyleName, "style:paragraph-properties");
    TEST_END_QTTEST("<r>\n <style:style style:name=\"User1\" style:display-name=\"User 1\" style:family=\"paragraph\">\n  <style:paragraph-properties myfont=\"isBold\"/>\n </style:style>\n</r>\n");

    // XML for user style 2
    TEST_BEGIN(0, 0);
    user2.writeStyle(&writer, coll, "style:style", user2StyleName, "style:paragraph-properties");
    TEST_END_QTTEST("<r>\n <style:style style:name=\"User2\" style:display-name=\"User 2\" style:family=\"paragraph\">\n  <style:paragraph-properties myfont=\"isBold\"/>\n </style:style>\n</r>\n");
}

void TestKoGenStyles::testStylesDotXml()
{
    kDebug() ;
    KoGenStyles coll;

    // Check that an autostyle-in-style.xml and an autostyle-in-content.xml
    // don't get the same name. It confuses KoGenStyle's named-based maps.
    KoGenStyle headerStyle(KoGenStyle::StyleAuto, "paragraph");
    headerStyle.addAttribute("style:master-page-name", "Standard");
    headerStyle.addProperty("style:page-number", "0");
    headerStyle.setAutoStyleInStylesDotXml(true);
    QString headerStyleName = coll.lookup(headerStyle, "P");
    QCOMPARE(headerStyleName, QString("P1"));

    //coll.dump();

    KoGenStyle first(KoGenStyle::StyleAuto, "paragraph");
    first.addAttribute("style:master-page-name", "Standard");
    QString firstName = coll.lookup(first, "P");
    kDebug() << "The auto style got assigned the name" << firstName;
    QCOMPARE(firstName, QString("P2"));     // anything but not P1.
}

QTEST_MAIN(TestKoGenStyles)
#include <TestKoGenStyles.moc>
