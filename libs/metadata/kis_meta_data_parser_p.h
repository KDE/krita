/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "kis_meta_data_parser.h"

namespace KisMetaData
{

class IntegerParser : public Parser
{
public:
    Value parse(const QString&) const override;
};
class TextParser : public Parser
{
public:
    Value parse(const QString&) const override;
};
class DateParser : public Parser
{
public:
    Value parse(const QString&) const override;
};
class RationalParser : public Parser
{
public:
    Value parse(const QString&) const override;
};

}

