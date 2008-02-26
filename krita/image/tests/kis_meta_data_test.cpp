/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_meta_data_test.h"

#include <qtest_kde.h>
#include "kis_meta_data_entry.h"
#include "kis_meta_data_filter_registry.h"
#include "kis_meta_data_value.h"
#include "kis_meta_data_schema.h"
#include "kis_meta_data_store.h"

using namespace KisMetaData;

KisMetaData::Value KisMetaDataTest::createUnsignedRationalValue()
{
    return KisMetaData::Value( UnsignedRational( 42, 12 ) );
}

KisMetaData::Value KisMetaDataTest::createSignedRationalValue()
{
    return KisMetaData::Value( SignedRational( 12, -42 ) );
}

KisMetaData::Value KisMetaDataTest::createIntegerValue(int v)
{
    return KisMetaData::Value( v );
}

KisMetaData::Value KisMetaDataTest::createStringValue()
{
    return KisMetaData::Value( "Hello World !" );
}

KisMetaData::Value KisMetaDataTest::createListValue()
{
    QList<Value> list;
    list << createUnsignedRationalValue() << createSignedRationalValue() << createIntegerValue() << createStringValue();
    return list;
}

void KisMetaDataTest::testRationals()
{
    {
        KisMetaData::SignedRational sr(-10, -14);
        QCOMPARE( sr.numerator, -10);
        QCOMPARE( sr.denominator, -14);
        KisMetaData::SignedRational sr2(14, 10);
        QVERIFY( sr == sr);
        QVERIFY( !(sr == sr2));
    }
    {
        KisMetaData::UnsignedRational sr(10, 14);
        QCOMPARE( sr.numerator, (unsigned int)10);
        QCOMPARE( sr.denominator, (unsigned int)14);
        KisMetaData::UnsignedRational sr2(14, 10);
        QVERIFY( sr == sr);
        QVERIFY( !(sr == sr2));
    }
}

void KisMetaDataTest::testValueCreation()
{
    {
        Value v;
        QCOMPARE( v.type(), Value::Invalid );
    }
    {
        Value v(10);
        QCOMPARE( v.type(), Value::Variant);
        QCOMPARE( v.asVariant().toInt(), 10 );
        QCOMPARE( v.asInteger(), 10 );
        QCOMPARE( createIntegerValue().type(), Value::Variant);
    }
    {
        Value v("Hello World !");
        QCOMPARE( v.type(), Value::Variant);
        QCOMPARE( v.asVariant().toString(), QString("Hello World !" ) );
        QCOMPARE( createStringValue().type(), Value::Variant);
    }
    {
        KisMetaData::SignedRational sr(42,-12);
        Value v(sr);
        QCOMPARE( v.type(), Value::SignedRational);
        QCOMPARE( v.asSignedRational(), sr);
        QCOMPARE( createSignedRationalValue().type(), Value::SignedRational );
        QCOMPARE( v.asInteger(), -42/12 );
        QCOMPARE( v.asDouble(), -42.0/12.0 );
    }
    {
        KisMetaData::UnsignedRational sr( 42, 12);
        Value v( sr );
        QCOMPARE( v.type(), Value::UnsignedRational);
        QCOMPARE( v.asUnsignedRational(), sr);
        QCOMPARE( createUnsignedRationalValue().type(), Value::UnsignedRational );
        QCOMPARE( v.asInteger(), 42/12 );
        QCOMPARE( v.asDouble(), 42.0/12.0 );
    }
    {
        QList<Value> list;
        list << createUnsignedRationalValue() << createSignedRationalValue() << createIntegerValue() << createStringValue();
        Value v( list);
        QCOMPARE( v.type(), Value::OrderedArray );
        QVERIFY( v.isArray());
        QCOMPARE( v.asArray(), list );
        QCOMPARE( createListValue().type(), Value::OrderedArray );
    }
    {
        Value v( QList<Value>(), Value::OrderedArray);
        QCOMPARE( v.type(), Value::OrderedArray );
        QVERIFY( v.isArray());
    }
    {
        Value v( QList<Value>(), Value::UnorderedArray);
        QCOMPARE( v.type(), Value::UnorderedArray );
        QVERIFY( v.isArray());
    }
    {
        Value v( QList<Value>(), Value::AlternativeArray);
        QCOMPARE( v.type(), Value::AlternativeArray );
        QVERIFY( v.isArray());
    }
}

void KisMetaDataTest::testValueEquality()
{
    QVERIFY( createUnsignedRationalValue() == createUnsignedRationalValue());
    QVERIFY( createSignedRationalValue() == createSignedRationalValue());
    QVERIFY( createIntegerValue() == createIntegerValue());
    QVERIFY( createStringValue() == createStringValue());
    QVERIFY( createListValue() == createListValue());
}


#define TEST_VALUE_COPY(func) \
    { \
        Value v1 = func(); \
        Value v2(v1); \
        Value v3 = v1; \
        QCOMPARE(v1, v2); \
        QCOMPARE(v1, v3); \
    }

void KisMetaDataTest::testValueCopy()
{
    TEST_VALUE_COPY(createUnsignedRationalValue);
    TEST_VALUE_COPY(createSignedRationalValue);
    TEST_VALUE_COPY(createIntegerValue);
    TEST_VALUE_COPY(createStringValue);
    TEST_VALUE_COPY(createListValue);
}

#define TEST_SCHEMA(uriStr) \
{ \
    const Schema* schema = SchemaRegistry::instance()->schemaFromUri(KisMetaData::Schema::uriStr); \
    QVERIFY(schema); \
    QCOMPARE(schema->uri(), KisMetaData::Schema::uriStr); \
    QCOMPARE(schema, SchemaRegistry::instance()->schemaFromPrefix(schema->prefix()) ); \
    QVERIFY( !SchemaRegistry::instance()->create("http://tartampion.com", schema->prefix())); \
    QCOMPARE(schema, SchemaRegistry::instance()->create(KisMetaData::Schema::uriStr, "tartampion")); \
    QCOMPARE(schema->prefix() + ":hello", schema->generateQualifiedName("hello")); \
}
    

void KisMetaDataTest::testSchema()
{
    TEST_SCHEMA(TIFFSchemaUri);
    TEST_SCHEMA(EXIFSchemaUri);
    TEST_SCHEMA(DublinCoreSchemaUri);
    TEST_SCHEMA(XMPSchemaUri);
    TEST_SCHEMA(XMPRightsSchemaUri);
    TEST_SCHEMA(MakerNoteSchemaUri);
    TEST_SCHEMA(IPTCSchemaUri);
    TEST_SCHEMA(PhotoshopSchemaUri);
}

void KisMetaDataTest::testEntry()
{
    const Schema* schema = SchemaRegistry::instance()->schemaFromUri(KisMetaData::Schema::TIFFSchemaUri);
    Value v1 = createIntegerValue(42);
    Value v2 = createIntegerValue(12);
    Entry e(schema, "test", v1);
    QCOMPARE(e.name(), QString("test"));
    QCOMPARE(e.schema(), schema);
    QCOMPARE(e.qualifiedName(), schema->generateQualifiedName("test"));
    QCOMPARE(e.value(), v1);
    e.value() = v2;
    QCOMPARE(e.value(), v2);
}

void KisMetaDataTest::testStore()
{
    const Schema* schema = SchemaRegistry::instance()->schemaFromUri(KisMetaData::Schema::TIFFSchemaUri);
    Store s;
    Entry e(schema, "test", createIntegerValue());
    QVERIFY(!s.containsEntry(schema, "test" ) );
    s.addEntry( e );
    QVERIFY(s.containsEntry(schema, "test" ) );
    QVERIFY(s.containsEntry(e.qualifiedName() ) );
    QVERIFY(s.containsEntry(KisMetaData::Schema::TIFFSchemaUri, "test" ) );
    s.removeEntry(schema, "test");
    QVERIFY(!s.containsEntry(schema, "test" ) );
}

void KisMetaDataTest::testFilters()
{
    // Test anonymizer
    {
        Store s;
        const KisMetaData::Filter* filter = FilterRegistry::instance()->get("Anonymizer");
        QVERIFY(filter);
        const KisMetaData::Schema* dcSchema = KisMetaData::SchemaRegistry::instance()->schemaFromUri(KisMetaData::Schema::DublinCoreSchemaUri);
        s.addEntry( Entry(dcSchema, "contributor", Value("somevalue") ) );
        s.addEntry( Entry(dcSchema, "creator", Value("somevalue") ) );
        s.addEntry( Entry(dcSchema, "publisher", Value("somevalue") ) );
        s.addEntry( Entry(dcSchema, "rights", Value("somevalue") ) );
        const KisMetaData::Schema* psSchema = KisMetaData::SchemaRegistry::instance()->schemaFromUri(KisMetaData::Schema::PhotoshopSchemaUri);
        s.addEntry( Entry(psSchema, "AuthorsPosition", Value("somevalue") ) );
        s.addEntry( Entry(psSchema, "CaptionWriter", Value("somevalue") ) );
        s.addEntry( Entry(psSchema, "Credit", Value("somevalue") ) );
        s.addEntry( Entry(psSchema, "City", Value("somevalue") ) );
        s.addEntry( Entry(psSchema, "Country", Value("somevalue") ) );
        QList<const KisMetaData::Filter*> filters;
        filters << filter;
        s.applyFilters( filters );
        QVERIFY(!s.containsEntry( dcSchema, "contributor") );
        QVERIFY(!s.containsEntry( dcSchema, "creator") );
        QVERIFY(!s.containsEntry( dcSchema, "publisher") );
        QVERIFY(!s.containsEntry( dcSchema, "rights") );
        QVERIFY(!s.containsEntry( psSchema, "AuthorsPosition") );
        QVERIFY(!s.containsEntry( psSchema, "CaptionWriter") );
        QVERIFY(!s.containsEntry( psSchema, "Credit") );
        QVERIFY(!s.containsEntry( psSchema, "City") );
        QVERIFY(!s.containsEntry( psSchema, "Country") );
    }
}

QTEST_KDEMAIN(KisMetaDataTest, GUI)
#include "kis_meta_data_test.moc"
