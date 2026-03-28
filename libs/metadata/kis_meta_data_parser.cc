/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "kis_meta_data_parser.h"

#include "kis_meta_data_value.h"

using namespace KisMetaData;

Parser::~Parser()
{
}

#include "kis_meta_data_parser_p.h"

#include <QDateTime>
#include <QRegularExpression>
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
    QRegularExpression regexp("(\\-?\\d+)/(\\d+)");
    QRegularExpressionMatch match = regexp.match(_v);

    if (match.capturedTexts().size() > 2)
        return Value(Rational(match.captured(1).toInt(), match.captured(2).toInt()));
    return Value();
}
