/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _KIS_META_DATA_PARSER_H_
#define _KIS_META_DATA_PARSER_H_

#include <kritametadata_export.h>

#include <QString>

namespace KisMetaData
{
class TypeInfo;
class Value;
/**
 * This class allow to parse from a string and return a value.
 */
class KRITAMETADATA_EXPORT Parser
{
    friend class TypeInfo;
public:
    virtual ~Parser();
    virtual Value parse(const QString&) const = 0;
};
}

#endif
