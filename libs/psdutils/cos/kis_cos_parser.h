/*
 *  SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISCOSPARSER_H
#define KISCOSPARSER_H

#include <QVariant>
#include <QIODevice>
#include "kritapsdutils_export.h"

/**
 * @brief The KisCosParser class
 *
 * PSD text engine data is written in PDF's Carousel Object Structure,
 * a format not unsimilar to (might be a precursor) to JSON.
 * JSON however doesn't differentiate between ints and doubles,
 * so we use QVariantHash instead.
 *
 * This parser tries to parse the ByteArray as a QVariantHash, though
 * not every data type is interpreted as such:
 *
 * For one, 'name' objects are interpreted as strings prepended with /
 * Hex strings are kept inside their < and >
 *
 * Code was based off qjsonparser.cpp
 */

class KRITAPSDUTILS_EXPORT KisCosParser
{
public:
    QVariantHash parseCosToJson(QByteArray *ba);
private:

    bool parseValue(QIODevice &dev, QVariant &val);
    bool parseObject(QIODevice &dev, QVariantHash &object, bool checkEnd = true);
    bool parseArray(QIODevice &dev, QVariantList &array);
};

#endif // KISCOSPARSER_H
