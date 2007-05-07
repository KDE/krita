/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <KoGenStyles.h>
#include <KoXmlWriter.h>
#include "../../store/tests/xmlwritertest.h"
#include <kdebug.h>
#include <assert.h>
//Added by qt3to4:
#include <Q3ValueList>

int testLookup()
{
    kDebug() << k_funcinfo << endl;
    KoGenStyles coll;

    QMap<QString, QString> map1;
    map1.insert( "map1key", "map1value" );
    QMap<QString, QString> map2;
    map2.insert( "map2key1", "map2value1" );
    map2.insert( "map2key2", "map2value2" );

    KoGenStyle first( KoGenStyle::STYLE_AUTO, "paragraph" );
    first.addAttribute( "style:master-page-name", "Standard" );
    first.addProperty( "style:page-number", "0" );
    first.addProperty( "style:foobar", "2", KoGenStyle::TextType );
    first.addStyleMap( map1 );
    first.addStyleMap( map2 );

    QString firstName = coll.lookup( first );
    kDebug() << "The first style got assigned the name " << firstName << endl;
    assert( firstName == "A1" ); // it's fine if it's something else, but the koxmlwriter tests require a known name
    assert( first.type() == KoGenStyle::STYLE_AUTO );

    KoGenStyle second( KoGenStyle::STYLE_AUTO, "paragraph" );
    second.addAttribute( "style:master-page-name", "Standard" );
    second.addProperty( "style:page-number", "0" );
    second.addProperty( "style:foobar", "2", KoGenStyle::TextType );
    second.addStyleMap( map1 );
    second.addStyleMap( map2 );

    QString secondName = coll.lookup( second );
    kDebug() << "The second style got assigned the name " << secondName << endl;

    assert( firstName == secondName ); // check that sharing works
    assert( first == second ); // check that operator== works :)

    const KoGenStyle* s = coll.style( firstName ); // check lookup of existing style
    assert( s );
    assert( *s == first );
    s = coll.style( "foobarblah" ); // check lookup of non-existing style
    assert( !s );

    KoGenStyle third( KoGenStyle::STYLE_AUTO, "paragraph", secondName ); // inherited style
    third.addProperty( "style:margin-left", "1.249cm" );
    third.addProperty( "style:page-number", "0" ); // same as parent
    third.addProperty( "style:foobar", "3", KoGenStyle::TextType ); // different from parent
    assert( third.parentName() == secondName );

    QString thirdName = coll.lookup( third, "P" );
    kDebug() << "The third style got assigned the name " << thirdName << endl;
    assert( thirdName == "P1" );

    KoGenStyle user( KoGenStyle::STYLE_USER, "paragraph" ); // differs from third since it doesn't inherit second, and has a different type
    user.addProperty( "style:margin-left", "1.249cm" );

    QString userStyleName = coll.lookup( user, "User", KoGenStyles::DontForceNumbering );
    kDebug() << "The user style got assigned the name " << userStyleName << endl;
    assert( userStyleName == "User" );

    KoGenStyle sameAsParent( KoGenStyle::STYLE_AUTO, "paragraph", secondName ); // inherited style
    sameAsParent.addAttribute( "style:master-page-name", "Standard" );
    sameAsParent.addProperty( "style:page-number", "0" );
    sameAsParent.addProperty( "style:foobar", "2", KoGenStyle::TextType );
    sameAsParent.addStyleMap( map1 );
    sameAsParent.addStyleMap( map2 );
    QString sapName = coll.lookup( sameAsParent, "foobar" );
    kDebug() << "The 'same as parent' style got assigned the name " << sapName << endl;

    assert( sapName == secondName );
    assert( coll.styles().count() == 3 );

    // OK, now add a style marked as for styles.xml; it looks like the above style, but
    // since it's marked for styles.xml it shouldn't be shared with it.
    KoGenStyle headerStyle( KoGenStyle::STYLE_AUTO, "paragraph" );
    headerStyle.addAttribute( "style:master-page-name", "Standard" );
    headerStyle.addProperty( "style:page-number", "0" );
    headerStyle.addProperty( "style:foobar", "2", KoGenStyle::TextType );
    headerStyle.addStyleMap( map1 );
    headerStyle.addStyleMap( map2 );
    headerStyle.setAutoStyleInStylesDotXml( true );
    QString headerStyleName = coll.lookup( headerStyle, "foobar" );

    assert( coll.styles().count() == 4 );
    assert( coll.styles( KoGenStyle::STYLE_AUTO ).count() == 2 );
    assert( coll.styles( KoGenStyle::STYLE_USER ).count() == 1 );

    Q3ValueList<KoGenStyles::NamedStyle> stylesXmlStyles = coll.styles( KoGenStyle::STYLE_AUTO, true );
    assert( stylesXmlStyles.count() == 1 );
    KoGenStyles::NamedStyle firstStyle = stylesXmlStyles.first();
    assert( firstStyle.name == headerStyleName );

    TEST_BEGIN( 0, 0 );
    first.writeStyle( &writer, coll, "style:style", firstName, "style:paragraph-properties" );
    TEST_END( "XML for first/second style", "<r>\n <style:style style:name=\"A1\" style:family=\"paragraph\" style:master-page-name=\"Standard\">\n  <style:paragraph-properties style:page-number=\"0\"/>\n  <style:text-properties style:foobar=\"2\"/>\n  <style:map map1key=\"map1value\"/>\n  <style:map map2key1=\"map2value1\" map2key2=\"map2value2\"/>\n </style:style>\n</r>\n" );

    TEST_BEGIN( 0, 0 );
    third.writeStyle( &writer, coll, "style:style", thirdName, "style:paragraph-properties" );
    TEST_END( "XML for third style", "<r>\n <style:style style:name=\"P1\" style:parent-style-name=\"A1\" style:family=\"paragraph\">\n  <style:paragraph-properties style:margin-left=\"1.249cm\"/>\n  <style:text-properties style:foobar=\"3\"/>\n </style:style>\n</r>\n" );

    coll.markStyleForStylesXml( firstName );
    {
        Q3ValueList<KoGenStyles::NamedStyle> stylesXmlStyles = coll.styles( KoGenStyle::STYLE_AUTO, true );
        assert( stylesXmlStyles.count() == 2 );
        Q3ValueList<KoGenStyles::NamedStyle> contentXmlStyles = coll.styles( KoGenStyle::STYLE_AUTO, false );
        assert( contentXmlStyles.count() == 1 );
    }

    return 0;
}

int testDefaultStyle()
{
    kDebug() << k_funcinfo << endl;
    /* Create a default style,
     * and then an auto style with exactly the same attributes
     * -> the lookup gives the default style.
     *
     * Also checks how the default style gets written out to XML.
     */
    KoGenStyles coll;

    KoGenStyle defaultStyle( KoGenStyle::STYLE_AUTO, "paragraph" );
    defaultStyle.addAttribute( "style:master-page-name", "Standard" );
    defaultStyle.addProperty( "myfont", "isBold" );
    defaultStyle.setDefaultStyle( true );
    QString defaultStyleName = coll.lookup( defaultStyle );
    assert( !defaultStyleName.isEmpty() ); // whatever, but not empty
    assert( defaultStyle.type() == KoGenStyle::STYLE_AUTO );
    assert( defaultStyle.isDefaultStyle() );

    KoGenStyle anotherStyle( KoGenStyle::STYLE_AUTO, "paragraph" );
    anotherStyle.addAttribute( "style:master-page-name", "Standard" );
    anotherStyle.addProperty( "myfont", "isBold" );
    QString anotherStyleName = coll.lookup( anotherStyle );
    assert( anotherStyleName == defaultStyleName );

    assert( coll.styles().count() == 1 );

    TEST_BEGIN( 0, 0 );
    defaultStyle.writeStyle( &writer, coll, "style:default-style", defaultStyleName, "style:paragraph-properties" );
    TEST_END( "XML for default style", "<r>\n <style:default-style style:family=\"paragraph\" style:master-page-name=\"Standard\">\n  <style:paragraph-properties myfont=\"isBold\"/>\n </style:default-style>\n</r>\n" );

    // The kspread case: not writing out all properties, only if they differ
    // from the default style.
    // KoGenStyles doesn't fetch info from the parent style when testing
    // for equality, so KSpread uses isEmpty() to check for equality-to-parent.
    KoGenStyle dataStyle( KoGenStyle::STYLE_AUTO, "paragraph", defaultStyleName );
    assert( dataStyle.isEmpty() );
    // and then it doesn't look up the auto style, but rather uses the parent style directly.

    return 0;
}

int testUserStyles()
{
    kDebug() << k_funcinfo << endl;
    /* Two user styles with exactly the same attributes+properties will not get merged, since
     * they don't have exactly the same attributes after all: the display-name obviously differs :)
     */
    KoGenStyles coll;

    KoGenStyle user1( KoGenStyle::STYLE_USER, "paragraph" );
    user1.addAttribute( "style:display-name", "User 1" );
    user1.addProperty( "myfont", "isBold" );

    QString user1StyleName = coll.lookup( user1, "User1", KoGenStyles::DontForceNumbering );
    kDebug() << "The user style got assigned the name " << user1StyleName << endl;
    assert( user1StyleName == "User1" );

    KoGenStyle user2( KoGenStyle::STYLE_USER, "paragraph" );
    user2.addAttribute( "style:display-name", "User 2" );
    user2.addProperty( "myfont", "isBold" );

    QString user2StyleName = coll.lookup( user2, "User2", KoGenStyles::DontForceNumbering );
    kDebug() << "The user style got assigned the name " << user2StyleName << endl;
    assert( user2StyleName == "User2" );

    // And now, what if the data uses that style?
    // This is like sameAsParent in the other test, but this time the
    // parent is a STYLE_USER...
    KoGenStyle dataStyle( KoGenStyle::STYLE_AUTO, "paragraph", user2StyleName );
    dataStyle.addProperty( "myfont", "isBold" );

    QString dataStyleName = coll.lookup( dataStyle, "DataStyle" );
    kDebug() << "The auto style got assigned the name " << dataStyleName << endl;
    assert( dataStyleName == "User2" ); // it found the parent as equal

    // Let's do the opposite test, just to make sure
    KoGenStyle dataStyle2( KoGenStyle::STYLE_AUTO, "paragraph", user2StyleName );
    dataStyle2.addProperty( "myfont", "isNotBold" );

    QString dataStyle2Name = coll.lookup( dataStyle2, "DataStyle" );
    kDebug() << "The different auto style got assigned the name " << dataStyle2Name << endl;
    assert( dataStyle2Name == "DataStyle1" );

    assert( coll.styles().count() == 3 );

    TEST_BEGIN( 0, 0 );
    user1.writeStyle( &writer, coll, "style:style", user1StyleName, "style:paragraph-properties" );
    TEST_END( "XML for user style 1", "<r>\n <style:style style:name=\"User1\" style:display-name=\"User 1\" style:family=\"paragraph\">\n  <style:paragraph-properties myfont=\"isBold\"/>\n </style:style>\n</r>\n" );

    TEST_BEGIN( 0, 0 );
    user2.writeStyle( &writer, coll, "style:style", user2StyleName, "style:paragraph-properties" );
    TEST_END( "XML for user style 2", "<r>\n <style:style style:name=\"User2\" style:display-name=\"User 2\" style:family=\"paragraph\">\n  <style:paragraph-properties myfont=\"isBold\"/>\n </style:style>\n</r>\n" );

    return 0;
}

int testStylesDotXml()
{
    kDebug() << k_funcinfo << endl;
    KoGenStyles coll;

    // Check that an autostyle-in-style.xml and an autostyle-in-content.xml
    // don't get the same name. It confuses KoGenStyle's named-based maps.
    KoGenStyle headerStyle( KoGenStyle::STYLE_AUTO, "paragraph" );
    headerStyle.addAttribute( "style:master-page-name", "Standard" );
    headerStyle.addProperty( "style:page-number", "0" );
    headerStyle.setAutoStyleInStylesDotXml( true );
    QString headerStyleName = coll.lookup( headerStyle, "P" );
    assert( headerStyleName == "P1" );

    //coll.dump();

    KoGenStyle first( KoGenStyle::STYLE_AUTO, "paragraph" );
    first.addAttribute( "style:master-page-name", "Standard" );
    QString firstName = coll.lookup( first, "P" );
    kDebug() << "The auto style got assigned the name " << firstName << endl;
    assert( firstName == "P2" ); // anything but not P1.
    return 0;
}

int main( int, char** ) {

    if ( testLookup() )
        return 1;
    if ( testDefaultStyle() )
        return 1;
    if ( testUserStyles() )
        return 1;
    if ( testStylesDotXml() )
        return 1;

    return 0;
}
