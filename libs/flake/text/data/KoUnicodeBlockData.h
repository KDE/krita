/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOUNICODEBLOCKDATA_H
#define KOUNICODEBLOCKDATA_H

#include <QString>
#include <QChar>
#include <QScopedPointer>
#include "kritaflake_export.h"

#include <boost/operators.hpp>
#include <klocalizedstring.h>

struct KRITAFLAKE_EXPORT KoUnicodeBlockData : public boost::equality_comparable<KoUnicodeBlockData> {
    KoUnicodeBlockData(QString name, uint start, uint end)
        : name(name)
        , start(start)
        , end(end) {}
    QString name; ///< Name of the block.
    uint start; ///< Start char
    uint end; ///< End char

    bool operator==(const KoUnicodeBlockData &rhs) const {
        return (start == rhs.start && end == rhs.end);
    }

    bool match (const uint &codepoint) const {
        return codepoint >= start && codepoint <= end;
    }
};

// This is a helper class to generate unicode block data.

class KRITAFLAKE_EXPORT KoUnicodeBlockDataFactory {
public:
    KoUnicodeBlockDataFactory();
    ~KoUnicodeBlockDataFactory();

    // Returns the unicode block for the given code point, if not available, returns noBlock().
    KoUnicodeBlockData blockForUCS(const uint &codepoint);

    // Default block when there's no other blocks.
    static KoUnicodeBlockData noBlock() {
        return KoUnicodeBlockData(i18nc("@title", "No Block"), 0x10FFFF, 0x10FFFF);
    }
private:
    struct Private;

    QScopedPointer<Private> d;
};

#endif // KOUNICODEBLOCKDATA_H
