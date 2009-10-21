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
#include "kis_meta_data_parser.h"
#include "kis_meta_data_validator.h"

using namespace KisMetaData;

KisMetaData::Value KisMetaDataTest::createRationalValue()
{
    return KisMetaData::Value(Rational(12, -42));
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
    list << createRationalValue() << createIntegerValue() << createStringValue();
    return list;
}

void KisMetaDataTest::testRationals()
{
    {
        KisMetaData::Rational sr(-10, -14);
        QCOMPARE(sr.numerator, -10);
        QCOMPARE(sr.denominator, -14);
        KisMetaData::Rational sr2(14, 10);
        QVERIFY(sr == sr);
        QVERIFY(!(sr == sr2));
    }
    {
        KisMetaData::Rational sr(10, 14);
        QCOMPARE(sr.numerator, 10);
        QCOMPARE(sr.denominator, 14);
        KisMetaData::Rational sr2(14, 10);
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
        KisMetaData::Rational sr(42, -12);
        Value v(sr);
        QCOMPARE(v.type(), Value::Rational);
        QCOMPARE(v.asRational(), sr);
        QCOMPARE(createRationalValue().type(), Value::Rational);
        QCOMPARE(v.asInteger(), -42 / 12);
        QCOMPARE(v.asDouble(), -42.0 / 12.0);
    }
    {
        KisMetaData::Rational sr(42, 12);
        Value v(sr);
        QCOMPARE(v.type(), Value::Rational);
        QCOMPARE(v.asRational(), sr);
        QCOMPARE(createRationalValue().type(), Value::Rational);
        QCOMPARE(v.asInteger(), 42 / 12);
        QCOMPARE(v.asDouble(), 42.0 / 12.0);
    }
    {
        QList<Value> list;
        list << createRationalValue() << createIntegerValue() << createStringValue();
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
    QVERIFY(createRationalValue() == createRationalValue());
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
    TEST_VALUE_COPY(createRationalValue);
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
    QVERIFY(TypeInfo::Private::Boolean->propertyType() == TypeInfo::BooleanType);
    QVERIFY(TypeInfo::Private::Boolean->embeddedPropertyType() == 0);
    QVERIFY(TypeInfo::Private::Boolean->choices().size() == 0);
    QVERIFY(TypeInfo::Private::Boolean->hasCorrectType(Value(true)));
    QVERIFY(!TypeInfo::Private::Boolean->hasCorrectType(createIntegerValue()));
    QVERIFY(!TypeInfo::Private::Boolean->hasCorrectType(createStringValue()));
    QVERIFY(!TypeInfo::Private::Boolean->hasCorrectType(createListValue()));

    QVERIFY(TypeInfo::Private::Integer->propertyType() == TypeInfo::IntegerType);
    QVERIFY(TypeInfo::Private::Integer->embeddedPropertyType() == 0);
    QVERIFY(TypeInfo::Private::Integer->choices().size() == 0);
    QVERIFY(!TypeInfo::Private::Integer->hasCorrectType(Value(true)));
    QVERIFY(TypeInfo::Private::Integer->hasCorrectType(createIntegerValue()));
    QVERIFY(!TypeInfo::Private::Integer->hasCorrectType(createStringValue()));
    QVERIFY(!TypeInfo::Private::Integer->hasCorrectType(createListValue()));

    QVERIFY(TypeInfo::Private::Text->propertyType() == TypeInfo::TextType);
    QVERIFY(TypeInfo::Private::Text->embeddedPropertyType() == 0);
    QVERIFY(TypeInfo::Private::Text->choices().size() == 0);
    QVERIFY(!TypeInfo::Private::Text->hasCorrectType(Value(true)));
    QVERIFY(!TypeInfo::Private::Text->hasCorrectType(createIntegerValue()));
    QVERIFY(TypeInfo::Private::Text->hasCorrectType(createStringValue()));
    QVERIFY(!TypeInfo::Private::Text->hasCorrectType(createListValue()));

    QVERIFY(TypeInfo::Private::Date->propertyType() == TypeInfo::DateType);
    QVERIFY(TypeInfo::Private::Date->embeddedPropertyType() == 0);
    QVERIFY(TypeInfo::Private::Date->choices().size() == 0);
    QVERIFY(TypeInfo::Private::Date->hasCorrectType(Value(QDateTime())));
    QVERIFY(!TypeInfo::Private::Date->hasCorrectType(createIntegerValue()));
    QVERIFY(!TypeInfo::Private::Date->hasCorrectType(createStringValue()));
    QVERIFY(!TypeInfo::Private::Date->hasCorrectType(createListValue()));

    QVERIFY(TypeInfo::Private::Rational->propertyType() == TypeInfo::RationalType);
    QVERIFY(TypeInfo::Private::Rational->embeddedPropertyType() == 0);
    QVERIFY(TypeInfo::Private::Rational->choices().size() == 0);
    QVERIFY(TypeInfo::Private::Rational->hasCorrectType(createRationalValue()));
    QVERIFY(!TypeInfo::Private::Rational->hasCorrectType(createIntegerValue()));
    QVERIFY(!TypeInfo::Private::Rational->hasCorrectType(createStringValue()));
    QVERIFY(!TypeInfo::Private::Rational->hasCorrectType(createListValue()));

    QVERIFY(TypeInfo::Private::GPSCoordinate->propertyType() == TypeInfo::GPSCoordinateType);
    QVERIFY(TypeInfo::Private::GPSCoordinate->embeddedPropertyType() == 0);
    QVERIFY(TypeInfo::Private::GPSCoordinate->choices().size() == 0);

    QVERIFY(TypeInfo::Private::LangArray->propertyType() == TypeInfo::LangArrayType);
    QVERIFY(TypeInfo::Private::LangArray->embeddedPropertyType() == TypeInfo::Private::Text);
    QVERIFY(TypeInfo::Private::LangArray->choices().size() == 0);

    // Create List of integer Value
    QList< Value > goodIntegerList;
    goodIntegerList.push_back(createIntegerValue());
    goodIntegerList.push_back(createIntegerValue(12));
    QList< Value > badIntegerList;
    badIntegerList.push_back(createIntegerValue());
    badIntegerList.push_back(createStringValue());
    badIntegerList.push_back(createIntegerValue(12));

    // Test OrderedArray
    const TypeInfo* arrOA1 = TypeInfo::Private::orderedArray(TypeInfo::Private::Integer);
    QVERIFY(arrOA1->propertyType() == TypeInfo::OrderedArrayType);
    QVERIFY(arrOA1->embeddedPropertyType() == TypeInfo::Private::Integer);
    QVERIFY(arrOA1->choices().size() == 0);
    QVERIFY(arrOA1 == TypeInfo::Private::orderedArray(TypeInfo::Private::Integer));
    const TypeInfo* arrOA2 = TypeInfo::Private::orderedArray(TypeInfo::Private::Text);
    QVERIFY(arrOA1 != arrOA2);
    QVERIFY(arrOA2->embeddedPropertyType() == TypeInfo::Private::Text);
    QVERIFY(!arrOA1->hasCorrectType(Value(true)));
    QVERIFY(!arrOA1->hasCorrectType(createIntegerValue()));
    QVERIFY(!arrOA1->hasCorrectType(createStringValue()));
    QVERIFY(!arrOA1->hasCorrectType(createListValue()));
    QVERIFY(arrOA1->hasCorrectType(Value(goodIntegerList, Value::OrderedArray)));
    QVERIFY(!arrOA1->hasCorrectType(Value(badIntegerList, Value::OrderedArray)));

    // Test UnarderedArray
    const TypeInfo* arrUOA1 = TypeInfo::Private::unorderedArray(TypeInfo::Private::Integer);
    QVERIFY(arrUOA1->propertyType() == TypeInfo::UnorderedArrayType);
    QVERIFY(arrUOA1->embeddedPropertyType() == TypeInfo::Private::Integer);
    QVERIFY(arrUOA1->choices().size() == 0);
    QVERIFY(arrUOA1 == TypeInfo::Private::unorderedArray(TypeInfo::Private::Integer));
    const TypeInfo* arrUOA2 = TypeInfo::Private::unorderedArray(TypeInfo::Private::Text);
    QVERIFY(arrUOA1 != arrUOA2);
    QVERIFY(arrUOA2->embeddedPropertyType() == TypeInfo::Private::Text);
    QVERIFY(arrUOA1 != arrOA1);
    QVERIFY(arrUOA2 != arrOA2);
    QVERIFY(!arrUOA1->hasCorrectType(Value(true)));
    QVERIFY(!arrUOA1->hasCorrectType(createIntegerValue()));
    QVERIFY(!arrUOA1->hasCorrectType(createStringValue()));
    QVERIFY(!arrUOA1->hasCorrectType(createListValue()));
    QVERIFY(arrUOA1->hasCorrectType(Value(goodIntegerList, Value::UnorderedArray)));
    QVERIFY(!arrUOA1->hasCorrectType(Value(badIntegerList, Value::UnorderedArray)));

    // Test AlternativeArray
    const TypeInfo* arrAA1 = TypeInfo::Private::alternativeArray(TypeInfo::Private::Integer);
    QVERIFY(arrAA1->propertyType() == TypeInfo::AlternativeArrayType);
    QVERIFY(arrAA1->embeddedPropertyType() == TypeInfo::Private::Integer);
    QVERIFY(arrAA1->choices().size() == 0);
    QVERIFY(arrAA1 == TypeInfo::Private::alternativeArray(TypeInfo::Private::Integer));
    const TypeInfo* arrAA2 = TypeInfo::Private::alternativeArray(TypeInfo::Private::Text);
    QVERIFY(arrAA1 != arrAA2);
    QVERIFY(arrAA2->embeddedPropertyType() == TypeInfo::Private::Text);
    QVERIFY(arrAA1 != arrOA1);
    QVERIFY(arrAA1 != arrUOA1);
    QVERIFY(arrAA2 != arrOA2);
    QVERIFY(arrAA2 != arrUOA2);
    QVERIFY(!arrAA1->hasCorrectType(Value(true)));
    QVERIFY(!arrAA1->hasCorrectType(createIntegerValue()));
    QVERIFY(!arrAA1->hasCorrectType(createStringValue()));
    QVERIFY(!arrAA1->hasCorrectType(createListValue()));
    QVERIFY(arrAA1->hasCorrectType(Value(goodIntegerList, Value::AlternativeArray)));
    QVERIFY(!arrAA1->hasCorrectType(Value(badIntegerList, Value::AlternativeArray)));

    // Test Choice
    QList< TypeInfo::Choice > choices;
    choices.push_back(TypeInfo::Choice(Value(12), "Hello"));
    choices.push_back(TypeInfo::Choice(Value(42), "World"));
    const TypeInfo* oChoice = TypeInfo::Private::createChoice(TypeInfo::OpenedChoice, TypeInfo::Private::Integer, choices);
    QVERIFY(oChoice->propertyType() == TypeInfo::OpenedChoice);
    QVERIFY(oChoice->embeddedPropertyType() == TypeInfo::Private::Integer);
    QVERIFY(oChoice->choices().size() == 2);
    QVERIFY(oChoice->choices()[0].value() == Value(12));
    QVERIFY(oChoice->choices()[0].hint() == "Hello");
    QVERIFY(oChoice->choices()[1].value() == Value(42));
    QVERIFY(oChoice->choices()[1].hint() == "World");
    QVERIFY(!oChoice->hasCorrectType(Value(true)));
    QVERIFY(oChoice->hasCorrectType(createIntegerValue(12)));
    QVERIFY(oChoice->hasCorrectType(createIntegerValue(-12)));
    QVERIFY(oChoice->hasCorrectType(createIntegerValue(42)));
    QVERIFY(!oChoice->hasCorrectType(createStringValue()));
    QVERIFY(!oChoice->hasCorrectType(createListValue()));
    QVERIFY(!oChoice->hasCorrectType(Value(goodIntegerList, Value::AlternativeArray)));
    QVERIFY(!oChoice->hasCorrectType(Value(badIntegerList, Value::AlternativeArray)));

    const TypeInfo* cChoice = TypeInfo::Private::createChoice(TypeInfo::ClosedChoice, TypeInfo::Private::Integer, choices);
    QVERIFY(cChoice->propertyType() == TypeInfo::ClosedChoice);
    QVERIFY(!cChoice->hasCorrectType(Value(true)));
    QVERIFY(cChoice->hasCorrectType(createIntegerValue(12)));
    QVERIFY(cChoice->hasCorrectType(createIntegerValue(-12)));
    QVERIFY(cChoice->hasCorrectType(createIntegerValue(42)));
    QVERIFY(!cChoice->hasCorrectType(createStringValue()));
    QVERIFY(!cChoice->hasCorrectType(createListValue()));
    QVERIFY(!cChoice->hasCorrectType(Value(goodIntegerList, Value::AlternativeArray)));
    QVERIFY(!cChoice->hasCorrectType(Value(badIntegerList, Value::AlternativeArray)));
    QVERIFY(cChoice->hasCorrectValue(createIntegerValue(12)));
    QVERIFY(!cChoice->hasCorrectValue(createIntegerValue(-12)));
    QVERIFY(cChoice->hasCorrectValue(createIntegerValue(42)));

    // Test structure


}

void KisMetaDataTest::testSchemaParse()
{
    const Schema* exifSchema = SchemaRegistry::instance()->schemaFromUri(Schema::EXIFSchemaUri);
    QVERIFY(exifSchema);

    const TypeInfo* colorSpaceType = exifSchema->propertyType("ColorSpace");
    QVERIFY(colorSpaceType);
    QVERIFY(colorSpaceType->propertyType() == TypeInfo::ClosedChoice);
    QVERIFY(colorSpaceType->choices().size() == 2);
    QVERIFY(colorSpaceType->choices()[0].value() == Value(1));
    QVERIFY(colorSpaceType->choices()[0].hint() == "sRGB");
    QVERIFY(colorSpaceType->choices()[1].value() == Value(65635));
    QVERIFY(colorSpaceType->choices()[1].hint() == "uncalibrated");

    QVERIFY(exifSchema->propertyType("CompressedBitsPerPixel"));
    QVERIFY(exifSchema->propertyType("CompressedBitsPerPixel")->propertyType() == TypeInfo::RationalType);

    QVERIFY(exifSchema->propertyType("PixelXDimension"));
    QVERIFY(exifSchema->propertyType("PixelXDimension")->propertyType() == TypeInfo::IntegerType);

    QVERIFY(exifSchema->propertyType("UserComment"));
    QVERIFY(exifSchema->propertyType("UserComment")->propertyType() == TypeInfo::LangArrayType);

    QVERIFY(exifSchema->propertyType("RelatedSoundFile"));
    QVERIFY(exifSchema->propertyType("RelatedSoundFile")->propertyType() == TypeInfo::TextType);

    QVERIFY(exifSchema->propertyType("DateTimeOriginal"));
    QVERIFY(exifSchema->propertyType("DateTimeOriginal")->propertyType() == TypeInfo::DateType);

    QVERIFY(exifSchema->propertyType("ISOSpeedRatings"));
    QVERIFY(exifSchema->propertyType("ISOSpeedRatings")->propertyType() == TypeInfo::OrderedArrayType);
    QVERIFY(exifSchema->propertyType("ISOSpeedRatings")->embeddedPropertyType() == TypeInfo::Private::Integer);

    const TypeInfo* oecfType = exifSchema->propertyType("OECF");
    QVERIFY(oecfType);
    QVERIFY(oecfType->propertyType() == TypeInfo::StructureType);
    QVERIFY(oecfType == exifSchema->structure("OECFSFR"));
    QVERIFY(oecfType->structureName() == "OECFSFR");
    QVERIFY(oecfType->structureSchema());
    QVERIFY(oecfType->structureSchema()->propertyType("Columns")->propertyType() == TypeInfo::IntegerType);
    QVERIFY(oecfType->structureSchema()->propertyType("Rows")->propertyType() == TypeInfo::IntegerType);
    QVERIFY(oecfType->structureSchema()->propertyType("Names")->propertyType() == TypeInfo::OrderedArrayType);
    QVERIFY(oecfType->structureSchema()->propertyType("Names")->embeddedPropertyType()->propertyType() == TypeInfo::TextType);
    QVERIFY(oecfType->structureSchema()->propertyType("Values")->propertyType() == TypeInfo::OrderedArrayType);
    QVERIFY(oecfType->structureSchema()->propertyType("Values")->embeddedPropertyType()->propertyType() == TypeInfo::RationalType);

}

void KisMetaDataTest::testParser()
{
    Value intV = TypeInfo::Private::Integer->parser()->parse("1242");
    QVERIFY(intV.type() == Value::Variant);
    QVERIFY(intV.asVariant() == 1242);
    QVERIFY(intV.asVariant().type() == QVariant::Int);

    Value textV = TypeInfo::Private::Text->parser()->parse("Bouh");
    QVERIFY(textV.type() == Value::Variant);
    QVERIFY(textV.asVariant() == "Bouh");
    QVERIFY(textV.asVariant().type() == QVariant::String);

    Value dateV1 = TypeInfo::Private::Date->parser()->parse("2005-10-31");
    QVERIFY(dateV1.type() == Value::Variant);
    QDateTime d1 = dateV1.asVariant().toDateTime();
    QVERIFY(d1.date().year() == 2005);
    QVERIFY(d1.date().month() == 10);
    QVERIFY(d1.date().day() == 31);

    Value dateV2 = TypeInfo::Private::Date->parser()->parse("2005");
    QVERIFY(dateV2.type() == Value::Variant);
    QDateTime d2 = dateV2.asVariant().toDateTime();
    QVERIFY(d2.date().year() == 2005);

    Value dateV3 = TypeInfo::Private::Date->parser()->parse("2005-12");
    QVERIFY(dateV3.type() == Value::Variant);
    QDateTime d3 = dateV3.asVariant().toDateTime();
    QVERIFY(d3.date().year() == 2005);
    QVERIFY(d3.date().month() == 12);

    Value dateV4 = TypeInfo::Private::Date->parser()->parse("2005-10-31T12:20");
    QVERIFY(dateV4.type() == Value::Variant);
    QDateTime d4 = dateV4.asVariant().toDateTime();
    QVERIFY(d4.date().year() == 2005);
    QVERIFY(d4.date().month() == 10);
    QVERIFY(d4.date().day() == 31);
    QVERIFY(d4.time().hour() == 12);
    QVERIFY(d4.time().minute() == 20);

    Value dateV5 = TypeInfo::Private::Date->parser()->parse("2005-10-31T12:20:32");
    QVERIFY(dateV5.type() == Value::Variant);
    QDateTime d5 = dateV5.asVariant().toDateTime();
    QVERIFY(d5.date().year() == 2005);
    QVERIFY(d5.date().month() == 10);
    QVERIFY(d5.date().day() == 31);
    QVERIFY(d5.time().hour() == 12);
    QVERIFY(d5.time().minute() == 20);
    QVERIFY(d5.time().second() == 32);

    Value dateV6 = TypeInfo::Private::Date->parser()->parse("2005-10-31T12:20:32-06:00");
    QVERIFY(dateV6.type() == Value::Variant);
    QDateTime d6 = dateV6.asVariant().toDateTime();
    QVERIFY(d6.date().year() == 2005);
    QVERIFY(d6.date().month() == 10);
    QVERIFY(d6.date().day() == 31);
    QVERIFY(d6.time().hour() == 18);
    QVERIFY(d6.time().minute() == 20);
    QVERIFY(d6.time().second() == 32);

    Value rational1 = TypeInfo::Private::Rational->parser()->parse("-10/20");
    QVERIFY(rational1.type() == Value::Rational);
    QVERIFY(rational1.asRational().numerator == -10);
    QVERIFY(rational1.asRational().denominator == 20);

    Value rational2 = TypeInfo::Private::Rational->parser()->parse("10/20");
    QVERIFY(rational2.type() == Value::Rational);
    QVERIFY(rational2.asRational().numerator == 10);
    QVERIFY(rational2.asRational().denominator == 20);
}

void KisMetaDataTest::testValidator()
{
    Store store;
    const Schema* exif = SchemaRegistry::instance()->schemaFromUri(Schema::EXIFSchemaUri);
    QVERIFY(exif);
    store.addEntry(Entry(exif, "PixelXDimension", createIntegerValue()));
    store.addEntry(Entry(exif, "PixelYDimension", createIntegerValue()));
    store.addEntry(Entry(exif, "RelatedSoundFile", createStringValue()));
    store.addEntry(Entry(exif, "ColorSpace", createIntegerValue(1)));
    store.addEntry(Entry(exif, "ExposureTime", createRationalValue()));
    Validator validator(&store);
    QCOMPARE(validator.countInvalidEntries(), 0);
    QCOMPARE(validator.countValidEntries(), 5);
    QCOMPARE(validator.invalidEntries().size(), 0);

    // Unknown entry
    store.addEntry(Entry(exif, "azerty", createIntegerValue()));
    validator.revalidate();
    QCOMPARE(validator.countInvalidEntries(), 1);
    QCOMPARE(validator.countValidEntries(), 5);
    QCOMPARE(validator.invalidEntries()[ exif->generateQualifiedName("azerty")].type(), Validator::Reason::UNKNOWN_ENTRY);
    store.removeEntry(exif, "azerty");

    // Invalid type for rational
    store.addEntry(Entry(exif, "FNumber", createIntegerValue()));
    validator.revalidate();
    QCOMPARE(validator.countInvalidEntries(), 1);
    QCOMPARE(validator.countValidEntries(), 5);
    QCOMPARE(validator.invalidEntries()[ exif->generateQualifiedName("FNumber")].type(), Validator::Reason::INVALID_TYPE);
    store.removeEntry(exif, "FNumber");

    // Invalid type for integer
    store.addEntry(Entry(exif, "SubjectLocation", createStringValue()));
    validator.revalidate();
    QCOMPARE(validator.countInvalidEntries(), 1);
    QCOMPARE(validator.countValidEntries(), 5);
    QCOMPARE(validator.invalidEntries()[ exif->generateQualifiedName("SubjectLocation")].type(), Validator::Reason::INVALID_TYPE);
    store.removeEntry(exif, "SubjectLocation");

    // Invalid type for choice
    store.addEntry(Entry(exif, "SensingMethod", createStringValue()));
    validator.revalidate();
    QCOMPARE(validator.countInvalidEntries(), 1);
    QCOMPARE(validator.countValidEntries(), 5);
    QCOMPARE(validator.invalidEntries()[ exif->generateQualifiedName("SensingMethod")].type(), Validator::Reason::INVALID_TYPE);
    store.removeEntry(exif, "SensingMethod");

    // Invalid value for choice
    store.addEntry(Entry(exif, "SensingMethod", createIntegerValue(1242)));
    validator.revalidate();
    QCOMPARE(validator.countInvalidEntries(), 1);
    QCOMPARE(validator.countValidEntries(), 5);
    QCOMPARE(validator.invalidEntries()[ exif->generateQualifiedName("SensingMethod")].type(), Validator::Reason::INVALID_VALUE);
    store.removeEntry(exif, "SensingMethod");

}

QTEST_KDEMAIN(KisMetaDataTest, GUI)
#include "kis_meta_data_test.moc"
