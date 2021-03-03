/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_META_DATA_TEST_H
#define KIS_META_DATA_TEST_H

#include <simpletest.h>

namespace KisMetaData
{
class Value;
}

class KisMetaDataTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testSchemaBasic();
    void testRationals();
    void testValueCreation();
    void testValueEquality();
    void testValueCopy();
    void testEntry();
    void testStore();
    void testFilters();
    void testTypeInfo();
    void testSchemaParse();
    void testParser();
    void testValidator();
private:
    KisMetaData::Value createRationalValue();
    KisMetaData::Value createIntegerValue(int v = 42);
    KisMetaData::Value createStringValue();
    KisMetaData::Value createListValue();
};

#endif
