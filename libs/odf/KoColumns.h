/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright 2002, 2003 David Faure <faure@kde.org>
   Copyright 2003 Nicolas GOUTTE <goutte@kde.org>
   Copyright 2007 Thomas Zander <zander@kde.org>
   Copyright 2012 Friedrich W. H. Kossebau <kossebau@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOCOLUMNS_H
#define KOCOLUMNS_H

#include "koodf_export.h"

#include <QtGlobal>
#include <QColor>

class KoGenStyle;
class KoXmlElement;


/** structure for columns */
struct KoColumns {
    enum SeparatorVerticalAlignment {
        AlignTop = Qt::AlignTop,
        AlignVCenter = Qt::AlignVCenter,
        AlignBottom = Qt::AlignBottom
    };

    enum SeparatorStyle {
        None = Qt::NoPen,
        Solid = Qt::SolidLine,
        Dashed = Qt::DashLine,
        Dotted = Qt::DotLine,
        DotDashed = Qt::DashDotLine
    };

    struct ColumnDatum
    {
        /** Left indent in points */
        qreal leftMargin;
        /** Right indent in points */
        qreal rightMargin;
        /** Top indent in points */
        qreal topMargin;
        /** Bottom indent in points */
        qreal bottomMargin;

        /** The relative width */
        int relativeWidth;

        ColumnDatum() {}
        ColumnDatum(qreal lm, qreal rm, qreal tm, qreal bm, int rw)
        : leftMargin(lm), rightMargin(rm), topMargin(tm), bottomMargin(bm), relativeWidth(rw) {}

        bool operator==(const KoColumns::ColumnDatum &rhs) const
        {
            return
                (leftMargin == rhs.leftMargin) &&
                (rightMargin == rhs.rightMargin) &&
                (topMargin == rhs.topMargin) &&
                (bottomMargin == rhs.bottomMargin) &&
                (relativeWidth == rhs.relativeWidth);
        }
    };

    /** Number of columns */
    int count;

    /** Spacing between columns in points */
    qreal gapWidth;

    SeparatorStyle separatorStyle;
    QColor separatorColor;
    SeparatorVerticalAlignment separatorVerticalAlignment;
    /** Width in pt */
    qreal separatorWidth;
    /** Height in percent. Default is 100% */
    unsigned int separatorHeight;

    /** data about the individual columns if there  */
    QList<ColumnDatum> columnData;

    /**
     * Construct a columns with the default column count 1,
     * default margins (2 cm), and portrait orientation.
     */
    KOODF_EXPORT KoColumns();

    KOODF_EXPORT void reset();
    KOODF_EXPORT bool operator==(const KoColumns &l) const;
    KOODF_EXPORT bool operator!=(const KoColumns &l) const;

    /**
     * Save this columns to ODF.
     */
    KOODF_EXPORT void saveOdf(KoGenStyle &style) const;

    /**
     * Load this columns from ODF
     */
    KOODF_EXPORT void loadOdf(const KoXmlElement &style);

    qreal totalRelativeWidth() const
    {
        qreal result = 0.0;
        foreach(const ColumnDatum &c, columnData) {
            result += c.relativeWidth;
        }
        return result;
    }

    KOODF_EXPORT static const char * separatorStyleString(KoColumns::SeparatorStyle separatorStyle);
    KOODF_EXPORT static const char * separatorVerticalAlignmentString(KoColumns::SeparatorVerticalAlignment separatorVerticalAlignment);
    KOODF_EXPORT static KoColumns::SeparatorVerticalAlignment parseSeparatorVerticalAlignment(const QString &value);
    KOODF_EXPORT static QColor parseSeparatorColor(const QString &value);
    KOODF_EXPORT static int parseSeparatorHeight(const QString &value);
    KOODF_EXPORT static KoColumns::SeparatorStyle parseSeparatorStyle(const QString &value);
    KOODF_EXPORT static int parseRelativeWidth(const QString &value);
};

#endif /* KOCOLUMNS_H */

