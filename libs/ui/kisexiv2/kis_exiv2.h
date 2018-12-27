/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_EXIV2_H_
#define _KIS_EXIV2_H_


#include <metadata/kis_meta_data_value.h>
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
