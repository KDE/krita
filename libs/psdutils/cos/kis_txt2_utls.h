/*
 *  SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TXT2_UTLS_H
#define KIS_TXT2_UTLS_H

#include  <QJsonDocument>
#include "kritapsdutils_export.h"

/**
 * @brief The KisTxt2Utils class
 *
 * Utils for handling text engine data inside the Txt2 additional info block.
 * This is a global section info block, which contains more extensive data to
 * the TySh additional info block.
 */
class KRITAPSDUTILS_EXPORT KisTxt2Utils
{
public:

    /**
     * @brief uncompressKeys
     * One of the problems with the Txt2 block is that older versions of photoshop (cs2-cs3 era)
     * stored the dictionary keys as names, while new versions stores it as numbers. This function
     * translates the numbers to the names.
     *
     * Thankfully the inkscape devs had already figured out the numbers and their keys, so
     * special thanks to them for documenting that:
     *
     * https://gitlab.com/inkscape/extras/extension-ai/-/blob/main/
     * docs/specification_amendments/text_documents.md?ref_type=heads
     *
     * @param doc input txt2 doc.
     * @return documented with keys translated to names.
     */
    static QJsonDocument uncompressKeys(QJsonDocument doc);
};

#endif // KIS_TXT2_UTLS_H
