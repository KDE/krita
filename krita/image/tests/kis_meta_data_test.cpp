/*
 *  Copyright (c) 2008-2009 Cyrille Berger <cberger@cberger.net>
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
#include "kis_meta_data_schema_registry.h"
#include "kis_meta_data_store.h"
#include "kis_meta_data_type_info.h"
#include "kis_meta_data_type_info_p.h"

using namespace KisMetaData;

KisMetaData::Value KisMetaDataTest::createUnsignedRationalValue()
{
    return KisMetaData::Value(UnsignedRational(42, 12));
}

KisMetaData::Value KisMetaDataTest::createSignedRationalValue()
{
    return KisMetaData::Value(SignedRational(12, -42));
}

KisMetaData::Value KisMetaDataTest::createIntegerValue(int v)
{
    return KisMetaData::Value(v);
}

KisMetaData::Value KisMetaDataTest::createStringValue()
{
    return KisMetaData::Value("Hello World !");
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
        QCOMPARE(sr.numerator, -10);
        QCOMPARE(sr.denominator, -14);
        KisMetaData::SignedRational sr2(14, 10);
        QVERIFY(sr == sr);
        QVERIFY(!(sr == sr2));
    }
    {
        KisMetaData::UnsignedRational sr(10, 14);
        QCOMPARE(sr.numerator, (unsigned int)10);
        QCOMPARE(sr.denominator, (unsigned int)14);
        KisMetaData::UnsignedRational sr2(14, 10);
        QVERIFY(sr == sr);
        QVERIFY(!(sr == sr2));
    }
}

void KisMetaDataTest::testValueCreation()
{
    {
        Value v;
        QCOMPARE(v.type(), Value::Invalid);
    }
    {
        Value v(10);
        QCOMPARE(v.type(), Value::Variant);
        QCOMPARE(v.asVariant().toInt(), 10);
        QCOMPARE(v.asInteger(), 10);
        QCOMPARE(createIntegerValue().type(), Value::Variant);
    }
    {
        Value v("Hello World !");
        QCOMPARE(v.type(), Value::Variant);
        QCOMPARE(v.asVariant().toString(), QString("Hello World !"));
        QCOMPARE(createStringValue().type(), Value::Variant);
    }
    {
        KisMetaData::SignedRational sr(42, -12);
        Value v(sr);
        QCOMPARE(v.type(), Value::SignedRational);
        QCOMPARE(v.asSignedRational(), sr);
        QCOMPARE(createSignedRationalValue().type(), Value::SignedRational);
        QCOMPARE(v.asInteger(), -42 / 12);
        QCOMPARE(v.asDouble(), -42.0 / 12.0);
    }
    {
        KisMetaData::UnsignedRational sr(42, 12);
        Value v(sr);
        QCOMPARE(v.type(), Value::UnsignedRational);
        QCOMPARE(v.asUnsignedRational(), sr);
        QCOMPARE(createUnsignedRationalValue().type(), Value::UnsignedRational);
        QCOMPARE(v.asInteger(), 42 / 12);
        QCOMPARE(v.asDouble(), 42.0 / 12.0);
    }
    {
        QList<Value> list;
        list << createUnsignedRationalValue() << createSignedRationalValue() << createIntegerValue() << createStringValue();
        Value v(list);
        QCOMPARE(v.type(), Value::OrderedArray);
        QVERIFY(v.isArray());
        QCOMPARE(v.asArray(), list);
        QCOMPARE(createListValue().type(), Value::OrderedArray);
    }
    {
        Value v(QList<Value>(), Value::OrderedArray);
        QCOMPARE(v.type(), Value::OrderedArray);
        QVERIFY(v.isArray());
    }
    {
        Value v(QList<Value>(), Value::UnorderedArray);
        QCOMPARE(v.type(), Value::UnorderedArray);
        QVERIFY(v.isArray());
    }
    {
        Value v(QList<Value>(), Value::AlternativeArray);
        QCOMPARE(v.type(), Value::AlternativeArray);
        QVERIFY(v.isArray());
    }
}

void KisMetaDataTest::testValueEquality()
{
    QVERIFY(createUnsignedRationalValue() == createUnsignedRationalValue());
    QVERIFY(createSignedRationalValue() == createSignedRationalValue());
    QVERIFY(createIntegerValue() == createIntegerValue());
    QVERIFY(createStringValue() == createStringValue());
    QVERIFY(createListValue() == createListValue());
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


void KisMetaDataTest::testSchemaBasic()
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
    QVERIFY(!s.containsEntry(schema, "test"));
    s.addEntry(e);
    QVERIFY(s.containsEntry(schema, "test"));
    QVERIFY(s.containsEntry(e.qualifiedName()));
    QVERIFY(s.containsEntry(KisMetaData::Schema::TIFFSchemaUri, "test"));
    s.removeEntry(schema, "test");
    QVERIFY(!s.containsEntry(schema, "test"));
    Entry& e2 = s.getEntry(schema, "hello");
    QVERIFY(s.containsEntry(schema, "hello"));
    QVERIFY(e2.name() == "hello");
    QVERIFY(e2.schema() == schema);
}

void KisMetaDataTest::testFilters()
{
    // Test anonymizer
    {
        Store s;
        const KisMetaData::Filter* filter = FilterRegistry::instance()->get("Anonymizer");
        QVERIFY(filter);
        const KisMetaData::Schema* dcSchema = KisMetaData::SchemaRegistry::instance()->schemaFromUri(KisMetaData::Schema::DublinCoreSchemaUri);
        s.addEntry(Entry(dcSchema, "contributor", Value("somevalue")));
        s.addEntry(Entry(dcSchema, "creator", Value("somevalue")));
        s.addEntry(Entry(dcSchema, "publisher", Value("somevalue")));
        s.addEntry(Entry(dcSchema, "rights", Value("somevalue")));
        const KisMetaData::Schema* psSchema = KisMetaData::SchemaRegistry::instance()->schemaFromUri(KisMetaData::Schema::PhotoshopSchemaUri);
        s.addEntry(Entry(psSchema, "AuthorsPosition", Value("somevalue")));
        s.addEntry(Entry(psSchema, "CaptionWriter", Value("somevalue")));
        s.addEntry(Entry(psSchema, "Credit", Value("somevalue")));
        s.addEntry(Entry(psSchema, "City", Value("somevalue")));
        s.addEntry(Entry(psSchema, "Country", Value("somevalue")));
        QList<const KisMetaData::Filter*> filters;
        filters << filter;
        s.applyFilters(filters);
        QVERIFY(!s.containsEntry(dcSchema, "contributor"));
        QVERIFY(!s.containsEntry(dcSchema, "creator"));
        QVERIFY(!s.containsEntry(dcSchema, "publisher"));
        QVERIFY(!s.containsEntry(dcSchema, "rights"));
        QVERIFY(!s.containsEntry(psSchema, "AuthorsPosition"));
        QVERIFY(!s.containsEntry(psSchema, "CaptionWriter"));
        QVERIFY(!s.containsEntry(psSchema, "Credit"));
        QVERIFY(!s.containsEntry(psSchema, "City"));
        QVERIFY(!s.containsEntry(psSchema, "Country"));
    }
}

void KisMetaDataTest::testTypeInfo()
{
    QVERIFY( TypeInfo::Private::Integer->propertyType() == TypeInfo::IntegerType );
    QVERIFY( TypeInfo::Private::Integer->embeddedPropertyType() == 0 );
    QVERIFY( TypeInfo::Private::Integer->choices().size() == 0 );
    
    QVERIFY( TypeInfo::Private::Text->propertyType() == TypeInfo::TextType );
    QVERIFY( TypeInfo::Private::Text->embeddedPropertyType() == 0 );
    QVERIFY( TypeInfo::Private::Text->choices().size() == 0 );
    
    QVERIFY( TypeInfo::Private::Date->propertyType() == TypeInfo::DateType );
    QVERIFY( TypeInfo::Private::Date->embeddedPropertyType() == 0 );
    QVERIFY( TypeInfo::Private::Date->choices().size() == 0 );
    
    QVERIFY( TypeInfo::Private::SignedRational->propertyType() == TypeInfo::SignedRationalType );
    QVERIFY( TypeInfo::Private::SignedRational->embeddedPropertyType() == 0 );
    QVERIFY( TypeInfo::Private::SignedRational->choices().size() == 0 );
    
    QVERIFY( TypeInfo::Private::UnsignedRational->propertyType() == TypeInfo::UnsignedRationalType );
    QVERIFY( TypeInfo::Private::UnsignedRational->embeddedPropertyType() == 0 );
    QVERIFY( TypeInfo::Private::UnsignedRational->choices().size() == 0 );
    
    QVERIFY( TypeInfo::Private::GPSCoordinate->propertyType() == TypeInfo::GPSCoordinateType );
    QVERIFY( TypeInfo::Private::GPSCoordinate->embeddedPropertyType() == 0 );
    QVERIFY( TypeInfo::Private::GPSCoordinate->choices().size() == 0 );
    
    QVERIFY( TypeInfo::Private::LangArray->propertyType() == TypeInfo::LangArrayType );
    QVERIFY( TypeInfo::Private::LangArray->embeddedPropertyType() == TypeInfo::Private::Text );
    QVERIFY( TypeInfo::Private::LangArray->choices().size() == 0 );
    
    // Test OrderedArray
    const TypeInfo* arrOA1 = TypeInfo::Private::orderedArray( TypeInfo::Private::Integer );
    QVERIFY( arrOA1->propertyType() == TypeInfo::OrderedArrayType );
    QVERIFY( arrOA1->embeddedPropertyType() == TypeInfo::Private::Integer );
    QVERIFY( arrOA1->choices().size() == 0 );
    QVERIFY( arrOA1 == TypeInfo::Private::orderedArray( TypeInfo::Private::Integer ) );
    const TypeInfo* arrOA2 = TypeInfo::Private::orderedArray( TypeInfo::Private::Text );
    QVERIFY( arrOA1 != arrOA2 );
    QVERIFY( arrOA2->embeddedPropertyType() == TypeInfo::Private::Text );
    
    // Test UnarderedArray
    const TypeInfo* arrUOA1 = TypeInfo::Private::unorderedArray( TypeInfo::Private::Integer );
    QVERIFY( arrUOA1->propertyType() == TypeInfo::UnorderedArrayType );
    QVERIFY( arrUOA1->embeddedPropertyType() == TypeInfo::Private::Integer );
    QVERIFY( arrUOA1->choices().size() == 0 );
    QVERIFY( arrUOA1 == TypeInfo::Private::unorderedArray( TypeInfo::Private::Integer ) );
    const TypeInfo* arrUOA2 = TypeInfo::Private::unorderedArray( TypeInfo::Private::Text );
    QVERIFY( arrUOA1 != arrUOA2 );
    QVERIFY( arrUOA2->embeddedPropertyType() == TypeInfo::Private::Text );
    QVERIFY( arrUOA1 != arrOA1 );
    QVERIFY( arrUOA2 != arrOA2 );
    
    // Test AlternativeArray
    const TypeInfo* arrAA1 = TypeInfo::Private::alternativeArray( TypeInfo::Private::Integer );
    QVERIFY( arrAA1->propertyType() == TypeInfo::AlternativeArrayType );
    QVERIFY( arrAA1->embeddedPropertyType() == TypeInfo::Private::Integer );
    QVERIFY( arrAA1->choices().size() == 0 );
    QVERIFY( arrAA1 == TypeInfo::Private::alternativeArray( TypeInfo::Private::Integer ) );
    const TypeInfo* arrAA2 = TypeInfo::Private::alternativeArray( TypeInfo::Private::Text );
    QVERIFY( arrAA1 != arrAA2 );
    QVERIFY( arrAA2->embeddedPropertyType() == TypeInfo::Private::Text );
    QVERIFY( arrAA1 != arrOA1 );
    QVERIFY( arrAA1 != arrUOA1 );
    QVERIFY( arrAA2 != arrOA2 );
    QVERIFY( arrAA2 != arrUOA2 );
    
    // Test Choice
    QList< TypeInfo::Choice > choices;
    choices.push_back( TypeInfo::Choice(Value(12), "Hello" ) );
    choices.push_back( TypeInfo::Choice(Value(42), "World" ) );
    const TypeInfo* oChoice = TypeInfo::Private::createChoice( TypeInfo::OpenedChoice, TypeInfo::Private::Integer, choices );
    QVERIFY( oChoice->propertyType() == TypeInfo::OpenedChoice );
    QVERIFY( oChoice->embeddedPropertyType() == TypeInfo::Private::Integer );
    QVERIFY( oChoice->choices().size() == 2 );
    QVERIFY( oChoice->choices()[0].value() == Value(12) );
    QVERIFY( oChoice->choices()[0].hint() == "Hello" );
    QVERIFY( oChoice->choices()[1].value() == Value(42) );
    QVERIFY( oChoice->choices()[1].hint() == "World" );
    const TypeInfo* cChoice = TypeInfo::Private::createChoice( TypeInfo::ClosedChoice, TypeInfo::Private::Integer, choices );
    QVERIFY( cChoice->propertyType() == TypeInfo::ClosedChoice );
}

void KisMetaDataTest::testSchemaParse()
{
    const Schema* exifSchema = SchemaRegistry::instance()->schemaFromUri(Schema::EXIFSchemaUri);
    QVERIFY(exifSchema);
    
    const TypeInfo* colorSpaceType = exifSchema->propertyType("ColorSpace");
    QVERIFY(colorSpaceType);
    QVERIFY(colorSpaceType->propertyType() == TypeInfo::ClosedChoice);
    QVERIFY(colorSpaceType->choices().size() == 2 );
    QVERIFY(colorSpaceType->choices()[0].value() == Value(1) );
    QVERIFY(colorSpaceType->choices()[0].hint() == "sRGB" );
    QVERIFY(colorSpaceType->choices()[1].value() == Value(65635) );
    QVERIFY(colorSpaceType->choices()[1].hint() == "uncalibrated" );
    
    QVERIFY(exifSchema->propertyType("CompressedBitsPerPixel"));
    QVERIFY(exifSchema->propertyType("CompressedBitsPerPixel")->propertyType() == TypeInfo::SignedRationalType);
    
    QVERIFY(exifSchema->propertyType("PixelXDimension"));
    QVERIFY(exifSchema->propertyType("PixelXDimension")->propertyType() == TypeInfo::IntegerType);
    
    QVERIFY(exifSchema->propertyType("UserComment"));
    QVERIFY(exifSchema->propertyType("UserComment")->propertyType() == TypeInfo::LangArrayType);
    
    QVERIFY(exifSchema->propertyType("RelatedSoundFile"));
    QVERIFY(exifSchema->propertyType("RelatedSoundFile")->propertyType() == TypeInfo::TextType);
    
    QVERIFY(exifSchema->propertyType("DateTimeOriginal"));
    QVERIFY(exifSchema->propertyType("DateTimeOriginal")->propertyType() == TypeInfo::DateType);
    
    const TypeInfo* oecfType = exifSchema->propertyType("OECF");
    QVERIFY(oecfType);
    QVERIFY(oecfType == exifSchema->structure("OECFSFR"));
    // TODO
    
}

QTEST_KDEMAIN(KisMetaDataTest, GUI)
#include "kis_meta_data_test.moc"
