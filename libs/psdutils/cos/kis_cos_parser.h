/*
 *  SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISCOSPARSER_H
#define KISCOSPARSER_H

#include <QJsonDocument>
#include "kritapsdutils_export.h"

/**
 * @brief The KisCosParser class
 *
 * PSD text engine data is written in PDF's Carousel Object Structure,
 * a format not unsimilar to (might be a precursor) to JSON.
 *
 * This parser tries to parse the ByteArray as a JSON document, though
 * there's some minor differences between the two formats.
 *
 * For one, 'name' objects are interpreted as strings prepended with /
 * Hex strings are kept inside their < and >
 * The parser does not differentiate between integer and float values.
 *
 * Code was based off qjsonparser.cpp
 */

class KRITAPSDUTILS_EXPORT KisCosParser
{
public:
    QJsonDocument parseCosToJson(QByteArray *ba);
private:

    bool parseValue(QIODevice &dev, QJsonValue &val);
    bool parseObject(QIODevice &dev, QJsonObject &object);
    bool parseArray(QIODevice &dev, QJsonArray &array);
};

#endif // KISCOSPARSER_H
