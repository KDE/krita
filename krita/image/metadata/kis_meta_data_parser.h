/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_META_DATA_PARSER_H_
#define _KIS_META_DATA_PARSER_H_

#include <krita_export.h>

#include <QString>

namespace KisMetaData
{
class TypeInfo;
class Value;
/**
 * This class allow to parse from a string and return a value.
 */
class KRITAIMAGE_EXPORT Parser
{
    friend class TypeInfo;
public:
    virtual ~Parser();
    virtual Value parse(const QString&) const = 0;
};
}

#endif
