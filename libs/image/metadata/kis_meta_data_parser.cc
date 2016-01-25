/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "kis_meta_data_parser.h"

#include "kis_meta_data_value.h"

using namespace KisMetaData;

Parser::~Parser()
{
}

#include "kis_meta_data_parser_p.h"

#include <QDateTime>
#include <QRegExp>
#include <QStringList>
#include <QVariant>


Value IntegerParser::parse(const QString& _v) const
{
    return Value(_v.toInt());
}

Value TextParser::parse(const QString& _v) const
{
    return Value(_v);
}

Value DateParser::parse(const QString& _v) const
{
    if (_v.length() <= 4) {
        return Value(QDateTime::fromString(_v, "yyyy"));
    } else if (_v.length() <= 7) {
        return Value(QDateTime::fromString(_v, "yyyy-MM"));
    } else if (_v.length() <= 10) {
        return Value(QDateTime::fromString(_v, "yyyy-MM-dd"));
    } else if (_v.length() <= 16) {
        return Value(QDateTime::fromString(_v, "yyyy-MM-ddThh:mm"));
    } else if (_v.length() <= 19) {
        return Value(QDateTime::fromString(_v, "yyyy-MM-ddThh:mm:ss"));
    } else {
        return Value(QDateTime::fromString(_v));
    }
}

Value RationalParser::parse(const QString& _v) const
{
    QRegExp regexp("(\\-?\\d+)/(\\d+)");
    regexp.indexIn(_v);
    if (regexp.capturedTexts().size() > 2)
        return Value(Rational(regexp.capturedTexts()[1].toInt(), regexp.capturedTexts()[2].toInt()));
    return Value();
}
