/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _KIS_EXIV2_H_
#define _KIS_EXIV2_H_


#include <kis_meta_data_value.h>
#include <exiv2/exiv2.hpp>
#include "kritaui_export.h"

/// Convert an exiv value to a KisMetaData value
KisMetaData::Value exivValueToKMDValue(const Exiv2::Value::AutoPtr value, bool forceSeq, KisMetaData::Value::ValueType arrayType = KisMetaData::Value::UnorderedArray);

/// Convert a KisMetaData to an Exiv value
Exiv2::Value* kmdValueToExivValue(const KisMetaData::Value& value, Exiv2::TypeId type);

/**
 * Convert a KisMetaData to an Exiv value, without knowing the targeted Exiv2::TypeId
 * This function should be used for saving to XMP.
 */
Exiv2::Value* kmdValueToExivXmpValue(const KisMetaData::Value& value);

struct KRITAUI_EXPORT KisExiv2 {

    static void initialize();

};
#endif
