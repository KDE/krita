/*
 *  SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TXT2_UTLS_H
#define KIS_TXT2_UTLS_H

#include <QVariantHash>
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
    static QVariantHash uncompressKeys(QVariantHash doc);

    /**
     * @brief defaultTxt2
     * Generate a default txt2 varianthash. This includes stuff like Kinsoku sets
     * default fonts and other things that might be expected but Krita doesn't write itself.
     * @return a default txt2 hash.
     */
    static QVariantHash defaultTxt2();

    /**
     * @brief tyShFromTxt2
     * Txt2 is more or less a superset of tySh, so this function allows
     * generating the tySh data from a Txt2 hash.
     * @param Txt2 the global txt2 hash.
     * @param textIndex the text object index for which to generate the tySh data.
     * @return the tySh variantHash
     */
    static QVariantHash tyShFromTxt2(const QVariantHash Txt2, int textIndex);
};

#endif // KIS_TXT2_UTLS_H
